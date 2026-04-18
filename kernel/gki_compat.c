/*
 * 冰冷 (BingLeng) - GKI 3.0 兼容实现
 *
 * 功能：提供Android版本检测、GKI版本检测、KCFI处理等功能
 *
 * 作者：冰冷开发组
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/string.h>
#include "gki_compat.h"
#include "utils.h"

/* ============================================
 * 全局变量
 * ============================================
 */
static int android_api_level = 0;
static int gki_version = 0;
static int kcfi_enabled = 0;

/* ============================================
 * Android 版本检测
 * ============================================
 */

/**
 * bl_detect_android_version - 检测Android版本
 */
int bl_detect_android_version(void)
{
    struct file *file = NULL;
    char buf[128];
    loff_t pos = 0;
    ssize_t nread;
    int api = 0;

    bl_printk(BL_LOG_INFO, "检测Android版本...\n");

    /* 尝试读取/build.prop */
    file = filp_open("/build.prop", O_RDONLY, 0);
    if (IS_ERR(file)) {
        /* 尝试读取/system/build.prop */
        file = filp_open("/system/build.prop", O_RDONLY, 0);
        if (IS_ERR(file)) {
            bl_printk(BL_LOG_WARNING, "无法读取build.prop\n");
            return -ENOENT;
        }
    }

    /* 查找ro.build.version.sdk */
    while ((nread = kernel_read(file, buf, sizeof(buf) - 1, &pos)) > 0) {
        buf[nread] = '\0';
        char *line = strstr(buf, "ro.build.version.sdk=");
        if (line) {
            if (sscanf(line, "ro.build.version.sdk=%d", &api) == 1) {
                android_api_level = api;
                bl_printk(BL_LOG_INFO, "检测到Android API级别: %d\n", api);
                break;
            }
        }
    }

    filp_close(file, NULL);

    if (android_api_level == 0) {
        bl_printk(BL_LOG_WARNING, "Android版本检测失败\n");
        return -EINVAL;
    }

    return android_api_level;
}

/* ============================================
 * GKI 版本检测
 * ============================================
 */

/**
 * bl_detect_gki_version - 检测GKI版本
 */
int bl_detect_gki_version(void)
{
    struct file *file = NULL;
    char buf[256];
    loff_t pos = 0;
    ssize_t nread;

    bl_printk(BL_LOG_INFO, "检测GKI版本...\n");

    /* 尝试读取/sys/kernel/gki/gki_info */
    file = filp_open("/sys/kernel/gki/gki_info", O_RDONLY, 0);
    if (!IS_ERR(file)) {
        nread = kernel_read(file, buf, sizeof(buf) - 1, &pos);
        if (nread > 0) {
            buf[nread] = '\0';
            /* 解析GKI版本 */
            int major;
            if (sscanf(buf, "GKI: %d", &major) == 1) {
                gki_version = major;
                bl_printk(BL_LOG_INFO, "检测到GKI版本: %d\n", major);
                filp_close(file, NULL);
                return gki_version;
            }
        }
        filp_close(file, NULL);
    }

    /* 备用方法：从内核版本字符串判断 */
    const char *kernel_ver = utsname()->release;
    bl_printk(BL_LOG_DEBUG, "内核版本: %s\n", kernel_ver);

    /* TODO: 更精确的GKI检测逻辑 */
    if (strstr(kernel_ver, "android13") || strstr(kernel_ver, "android14") || 
        strstr(kernel_ver, "android15") || strstr(kernel_ver, "android16")) {
        gki_version = 2; /* 默认GKI 2.0+ */
        if (strstr(kernel_ver, "android16")) {
            gki_version = 3;
        }
    }

    bl_printk(BL_LOG_INFO, "GKI版本: %d\n", gki_version);
    return gki_version;
}

/* ============================================
 * KCFI 检测
 * ============================================
 */

/**
 * bl_is_kcfi_enabled - 检测KCFI是否启用
 */
int bl_is_kcfi_enabled(void)
{
    /* TODO: 检测__kcfi_typeid符号是否存在 */
    /* TODO: 检查GKI版本是否>=3 */
    
    kcfi_enabled = (gki_version >= 3) ? 1 : 0;
    bl_printk(BL_LOG_INFO, "KCFI启用状态: %d\n", kcfi_enabled);
    return kcfi_enabled;
}

/* ============================================
 * KCFI 类型 ID 处理
 * ============================================
 */

/**
 * bl_kcfi_get_typeid - 获取函数的CFI类型ID
 */
unsigned long bl_kcfi_get_typeid(void *func)
{
    /* TODO: 实现KCFI类型ID获取 */
    /* [TODO: 查阅内核源码了解KCFI类型ID存储位置] */
    return 0;
}

/**
 * bl_kcfi_set_typeid - 设置函数的CFI类型ID
 */
int bl_kcfi_set_typeid(void *func, unsigned long typeid)
{
    /* TODO: 实现KCFI类型ID设置 */
    /* [TODO: 查阅内核源码了解如何修改KCFI类型ID] */
    return 0;
}

/* ============================================
 * 模块签名绕过
 * ============================================
 */

/**
 * bl_disable_module_sig - 禁用模块签名验证
 */
int bl_disable_module_sig(void)
{
    /* TODO: 实现模块签名绕过 */
    /* [TODO: 查阅内核源码了解如何禁用模块签名验证] */
    bl_printk(BL_LOG_WARNING, "模块签名绕过未实现\n");
    return 0;
}

/* ============================================
 * 初始化和清理
 * ============================================
 */

int bl_gki_init(void)
{
    bl_printk(BL_LOG_INFO, "初始化GKI兼容模块\n");
    
    /* 检测GKI版本 */
    bl_detect_gki_version();
    
    /* 检测KCFI */
    bl_is_kcfi_enabled();
    
    return 0;
}

void bl_gki_exit(void)
{
    bl_printk(BL_LOG_INFO, "清理GKI兼容模块\n");
}
