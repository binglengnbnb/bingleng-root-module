/*
 * 冰冷 (BingLeng) - 内核模块主文件
 *
 * 功能：LKM入口/出口，模块参数处理，字符设备注册，ioctl处理
 *
 * 初始化流程：检测Android版本→检测GKI→初始化kallsyms→处理KCFI→安装hook→注册设备→初始化令牌→自隐藏
 *
 * 作者：冰冷开发组
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include "bingleng_main.h"
#include "utils.h"
#include "kallsyms.h"
#include "token_verify.h"
#include "gki_compat.h"
#include "arm64_hook.h"
#include "syscall_hook.h"
#include "priv_escalation.h"
#include "hide.h"

/* ============================================
 * 模块参数
 * ============================================
 */

/* 隐藏模块：1=隐藏，0=显示 */
int hide = 1;
module_param(hide, int, 0644);
MODULE_PARM_DESC(hide, "Hide module (1=hide, 0=show)");

/* 调试日志：1=启用，0=禁用 */
int debug = 0;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "Enable debug logs (1=enable, 0=disable)");

/* ============================================
 * 全局变量
 * ============================================
 */

/* 字符设备相关 */
static dev_t bl_devt;
static struct cdev bl_cdev;
struct class *bl_class = NULL;
struct device *bl_device = NULL;

/* 授权链表和自旋锁 */
struct list_head bl_auth_list;
spinlock_t bl_auth_lock;

/* 模块初始化状态 */
static int bl_initialized = 0;

/* ============================================
 * 字符设备操作函数
 * ============================================
 */

/**
 * bl_open - 设备打开函数
 */
static int bl_open(struct inode *inode, struct file *file)
{
    bl_printk(BL_LOG_DEBUG, "设备被打开\n");
    return 0;
}

/**
 * bl_release - 设备释放函数
 */
static int bl_release(struct inode *inode, struct file *file)
{
    bl_printk(BL_LOG_DEBUG, "设备被关闭\n");
    return 0;
}

/**
 * bl_unlocked_ioctl - ioctl处理函数
 */
static long bl_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int ret = 0;
    void __user *argp = (void __user *)arg;

    bl_printk(BL_LOG_DEBUG, "ioctl命令: cmd=0x%x\n", cmd);

    switch (cmd) {
        case BL_IOCTL_GET_TOKEN: {
            uint64_t token = bl_generate_token();
            if (copy_to_user(argp, &token, sizeof(token))) {
                ret = -EFAULT;
                bl_printk(BL_LOG_ERR, "复制令牌到用户空间失败\n");
            }
            break;
        }

        case BL_IOCTL_VERIFY: {
            struct bl_auth_request req;
            if (copy_from_user(&req, argp, sizeof(req))) {
                ret = -EFAULT;
                bl_printk(BL_LOG_ERR, "复制授权请求失败\n");
                break;
            }
            /* 验证令牌并添加授权 */
            if (bl_verify_token(req.uid, req.token)) {
                ret = bl_add_auth(req.uid, req.token, req.pid, req.pkg_name);
            } else {
                ret = -EPERM;
            }
            break;
        }

        case BL_IOCTL_REVOKE: {
            uid_t uid;
            if (copy_from_user(&uid, argp, sizeof(uid))) {
                ret = -EFAULT;
                bl_printk(BL_LOG_ERR, "复制UID失败\n");
                break;
            }
            bl_revoke_token(uid);
            break;
        }

        case BL_IOCTL_STATUS: {
            struct bl_status status;
            status.initialized = bl_initialized;
            status.hide_enabled = hide;
            status.debug_enabled = debug;
            status.auth_count = bl_get_auth_count();
            if (copy_to_user(argp, &status, sizeof(status))) {
                ret = -EFAULT;
                bl_printk(BL_LOG_ERR, "复制状态到用户空间失败\n");
            }
            break;
        }

        default:
            ret = -ENOTTY;
            bl_printk(BL_LOG_WARNING, "未知ioctl命令: 0x%x\n", cmd);
            break;
    }

    return ret;
}

#ifdef CONFIG_COMPAT
/**
 * bl_compat_ioctl - 32位兼容ioctl处理
 */
static long bl_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    return bl_unlocked_ioctl(file, cmd, (unsigned long)compat_ptr(arg));
}
#endif

/* 字符设备操作结构 */
static const struct file_operations bl_fops = {
    .owner = THIS_MODULE,
    .open = bl_open,
    .release = bl_release,
    .unlocked_ioctl = bl_unlocked_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = bl_compat_ioctl,
#endif
};

/* ============================================
 * 模块自隐藏函数
 * ============================================
 */

/**
 * bl_hide_module - 隐藏内核模块
 */
int bl_hide_module(void)
{
    if (!hide) {
        bl_printk(BL_LOG_INFO, "隐藏功能未启用\n");
        return 0;
    }

    bl_printk(BL_LOG_INFO, "开始隐藏模块...\n");

    /* 1. 从模块链表中移除 */
    list_del(&THIS_MODULE->list);

    /* 2. 从sysfs中移除 */
    kobject_del(&THIS_MODULE->mkobj.kobj);

    /* TODO: 3. hook seq_read隐藏/proc/modules */
    /* TODO: 4. 清理dmesg中的模块名 */

    bl_printk(BL_LOG_INFO, "模块隐藏完成\n");
    return 0;
}

/**
 * bl_unhide_module - 取消隐藏模块（退出时调用）
 */
void bl_unhide_module(void)
{
    /* 退出时尝试恢复，但可能已经无法恢复 */
    bl_printk(BL_LOG_INFO, "模块退出\n");
}

/* ============================================
 * 模块初始化
 * ============================================
 */

/**
 * bl_init - 模块初始化入口
 */
static int __init bl_init(void)
{
    int ret = 0;

    bl_printk(BL_LOG_INFO, "========================================\n");
    bl_printk(BL_LOG_INFO, "  冰冷 (BingLeng) 内核模块加载中...\n");
    bl_printk(BL_LOG_INFO, "  版本: 1.0.0\n");
    bl_printk(BL_LOG_INFO, "========================================\n");

    /* 初始化授权链表和锁 */
    INIT_LIST_HEAD(&bl_auth_list);
    spin_lock_init(&bl_auth_lock);

    /* 步骤1: 检测Android版本 */
    bl_printk(BL_LOG_INFO, "[1/10] 检测Android版本...\n");
    ret = bl_detect_android_version();
    if (ret < 0) {
        bl_printk(BL_LOG_WARNING, "Android版本检测失败，继续...\n");
    }

    /* 步骤2: 检测GKI版本 */
    bl_printk(BL_LOG_INFO, "[2/10] 检测GKI版本...\n");
    ret = bl_detect_gki_version();
    if (ret < 0) {
        bl_printk(BL_LOG_WARNING, "GKI版本检测失败，继续...\n");
    }

    /* 步骤3: 初始化kallsyms */
    bl_printk(BL_LOG_INFO, "[3/10] 初始化内核符号查找...\n");
    ret = bl_kallsyms_init();
    if (ret < 0) {
        bl_printk(BL_LOG_ERR, "kallsyms初始化失败\n");
        goto error;
    }

    /* 步骤4: 初始化GKI兼容处理 */
    bl_printk(BL_LOG_INFO, "[4/10] 初始化GKI兼容处理...\n");
    ret = bl_gki_init();
    if (ret < 0) {
        bl_printk(BL_LOG_WARNING, "GKI兼容初始化失败，继续...\n");
    }

    /* 步骤5: 初始化ARM64 hook */
    bl_printk(BL_LOG_INFO, "[5/10] 初始化ARM64 hook...\n");
    ret = bl_hook_init();
    if (ret < 0) {
        bl_printk(BL_LOG_WARNING, "ARM64 hook初始化失败，继续...\n");
    }

    /* 步骤6: 初始化系统调用hook */
    bl_printk(BL_LOG_INFO, "[6/10] 初始化系统调用hook...\n");
    ret = bl_syscall_hook_init();
    if (ret < 0) {
        bl_printk(BL_LOG_WARNING, "系统调用hook初始化失败，继续...\n");
    }

    /* 步骤7: 注册字符设备 */
    bl_printk(BL_LOG_INFO, "[7/10] 注册字符设备...\n");

    /* 分配设备号 */
    ret = alloc_chrdev_region(&bl_devt, 0, 1, BINGLENG_DEVICE_NAME);
    if (ret < 0) {
        bl_printk(BL_LOG_ERR, "分配设备号失败: %d\n", ret);
        goto error_syscall_hook;
    }

    /* 初始化字符设备 */
    cdev_init(&bl_cdev, &bl_fops);
    bl_cdev.owner = THIS_MODULE;

    /* 添加字符设备 */
    ret = cdev_add(&bl_cdev, bl_devt, 1);
    if (ret < 0) {
        bl_printk(BL_LOG_ERR, "添加字符设备失败: %d\n", ret);
        goto error_region;
    }

    /* 创建设备类 */
    bl_class = class_create(THIS_MODULE, BINGLENG_MODULE_NAME);
    if (IS_ERR(bl_class)) {
        ret = PTR_ERR(bl_class);
        bl_printk(BL_LOG_ERR, "创建设备类失败: %d\n", ret);
        goto error_cdev;
    }

    /* 创建设备节点 */
    bl_device = device_create(bl_class, NULL, bl_devt, NULL, BINGLENG_DEVICE_NAME);
    if (IS_ERR(bl_device)) {
        ret = PTR_ERR(bl_device);
        bl_printk(BL_LOG_ERR, "创建设备节点失败: %d\n", ret);
        goto error_class;
    }

    /* 步骤8: 初始化令牌验证 */
    bl_printk(BL_LOG_INFO, "[8/10] 初始化令牌验证...\n");
    ret = bl_token_init();
    if (ret < 0) {
        bl_printk(BL_LOG_ERR, "令牌初始化失败: %d\n", ret);
        goto error_device;
    }

    /* 步骤9: 隐藏模块 */
    bl_printk(BL_LOG_INFO, "[9/10] 隐藏模块...\n");
    ret = bl_hide_module();
    if (ret < 0) {
        bl_printk(BL_LOG_WARNING, "模块隐藏失败\n");
    }

    bl_initialized = 1;
    bl_printk(BL_LOG_INFO, "========================================\n");
    bl_printk(BL_LOG_INFO, "  冰冷 (BingLeng) 内核模块加载成功!\n");
    bl_printk(BL_LOG_INFO, "========================================\n");

    return 0;

    /* 错误处理 - 按相反顺序清理 */
error_device:
    device_destroy(bl_class, bl_devt);
error_class:
    class_destroy(bl_class);
error_cdev:
    cdev_del(&bl_cdev);
error_region:
    unregister_chrdev_region(bl_devt, 1);
error_syscall_hook:
    bl_syscall_hook_exit();
error_hook:
    bl_hook_exit();
error_gki:
    bl_gki_exit();
error_kallsyms:
    bl_kallsyms_exit();
error:
    bl_printk(BL_LOG_ERR, "模块初始化失败\n");
    return ret;
}

/* ============================================
 * 模块退出
 * ============================================
 */

/**
 * bl_exit - 模块退出函数
 */
static void __exit bl_exit(void)
{
    bl_printk(BL_LOG_INFO, "========================================\n");
    bl_printk(BL_LOG_INFO, "  冰冷 (BingLeng) 内核模块卸载中...\n");
    bl_printk(BL_LOG_INFO, "========================================\n");

    bl_initialized = 0;

    /* 1. 取消隐藏 */
    bl_unhide_module();

    /* 2. 清理令牌 */
    bl_token_exit();

    /* 3. 销毁设备节点 */
    if (bl_device) {
        device_destroy(bl_class, bl_devt);
    }

    /* 4. 销毁设备类 */
    if (bl_class) {
        class_destroy(bl_class);
    }

    /* 5. 删除字符设备 */
    cdev_del(&bl_cdev);

    /* 6. 释放设备号 */
    unregister_chrdev_region(bl_devt, 1);

    /* 7. 清理系统调用hook */
    bl_syscall_hook_exit();

    /* 8. 清理ARM64 hook */
    bl_hook_exit();

    /* 9. 清理GKI兼容 */
    bl_gki_exit();

    /* 10. 清理kallsyms */
    bl_kallsyms_exit();

    bl_printk(BL_LOG_INFO, "冰冷 (BingLeng) 内核模块卸载完成\n");
}

/* ============================================
 * 模块信息
 * ============================================
 */

module_init(bl_init);
module_exit(bl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("冰冷开发组");
MODULE_DESCRIPTION("冰冷 - Android 16 内核级 Root 管理器");
MODULE_VERSION("1.0.0");
