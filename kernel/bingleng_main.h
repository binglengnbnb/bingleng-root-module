/*
 * 冰冷 (BingLeng) - 主头文件
 *
 * 功能：定义内核模块的主要数据结构、ioctl命令、函数声明
 *
 * 作者：冰冷开发组
 */

#ifndef BINGLENG_MAIN_H
#define BINGLENG_MAIN_H

#include <linux/types.h>
#include <linux/ioctl.h>

/* ============================================
 * 模块配置
 * ============================================
 */

/* 模块名称 */
#define BINGLENG_MODULE_NAME "bingleng"

/* 设备名称 */
#define BINGLENG_DEVICE_NAME "bingleng"

/* 字符设备主设备号（0表示动态分配 */
#define BINGLENG_MAJOR 0

/* 默认令牌过期时间（秒） */
#define BINGLENG_TOKEN_EXPIRE 300

/* ============================================
 * Ioctl 命令定义
 * ============================================
 */

/* 魔法数字，用于ioctl命令标识 */
#define BINGLENG_IOC_MAGIC 'B'

/* 获取64位令牌 */
#define BL_IOCTL_GET_TOKEN    _IOR(BINGLENG_IOC_MAGIC, 0x01, uint64_t)

/* 验证授权 */
#define BL_IOCTL_VERIFY      _IOWR(BINGLENG_IOC_MAGIC, 0x02, struct bl_auth_request)

/* 撤销授权 */
#define BL_IOCTL_REVOKE     _IOW(BINGLENG_IOC_MAGIC, 0x03, uid_t)

/* 获取状态 */
#define BL_IOCTL_STATUS     _IOR(BINGLENG_IOC_MAGIC, 0x04, struct bl_status)

/* ============================================
 * 数据结构定义
 * ============================================
 */

/* 授权请求结构 */
struct bl_auth_request {
    uid_t uid;           /* 用户ID */
    uint64_t token;      /* 64位令牌 */
    pid_t pid;           /* 进程ID */
    char pkg_name[64];   /* 包名 */
};

/* 状态信息结构 */
struct bl_status {
    int initialized;      /* 是否初始化 */
    int hide_enabled;   /* 是否启用隐藏 */
    int debug_enabled;  /* 是否启用调试 */
    int auth_count;     /* 当前授权数 */
};

/* 授权记录结构 */
struct bl_auth_entry {
    struct list_head list;   /* 链表节点 */
    uid_t uid;           /* 用户ID */
    uint64_t token;        /* 64位令牌 */
    unsigned long expire;  /* 过期时间（jiffies） */
    pid_t pid;           /* 进程ID */
    char pkg_name[64];    /* 包名 */
};

/* ============================================
 * 全局变量声明（在bingleng_main.c中定义）
 * ============================================
 */

/* 模块参数 */
extern int hide;         /* 隐藏模块：1=隐藏，0=显示 */
extern int debug;        /* 调试日志：1=启用，0=禁用 */

/* 授权链表和自旋锁 */
extern struct list_head bl_auth_list;
extern spinlock_t bl_auth_lock;

/* 字符设备类和设备 */
extern struct class *bl_class;
extern struct device *bl_device;

/* ============================================
 * 函数声明
 * ============================================
 */

/* 初始化函数 */
int bl_init(void);
void bl_exit(void);

/* 令牌验证函数 */
int bl_token_init(void);
void bl_token_exit(void);
uint64_t bl_generate_token(void);
int bl_verify_token(uid_t uid, uint64_t token);
void bl_revoke_token(uid_t uid);
void bl_cleanup_expired_tokens(void);

/* 模块隐藏函数 */
int bl_hide_module(void);
void bl_unhide_module(void);

/* GKI兼容函数 */
int bl_gki_init(void);
void bl_gki_exit(void);
int bl_detect_android_version(void);
int bl_detect_gki_version(void);
int bl_is_kcfi_enabled(void);

#endif /* BINGLENG_MAIN_H */
