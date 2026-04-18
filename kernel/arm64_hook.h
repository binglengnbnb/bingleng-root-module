/*
 * 冰冷 (BingLeng) - ARM64 inline hook 头文件
 *
 * 功能：提供ARM64 inline hook功能，支持KCFI兼容
 *
 * 作者：冰冷开发组
 */

#ifndef BINGLENG_ARM64_HOOK_H
#define BINGLENG_ARM64_HOOK_H

#include <linux/types.h>

/* ============================================
 * Hook信息结构
 * ============================================
 */
struct bl_hook {
    void *orig_func;         /* 原始函数地址 */
    void *hook_func;         /* Hook函数地址 */
    unsigned long orig_inst[4]; /* 保存的原始指令 */
    unsigned long cfi_typeid;   /* 保存的CFI类型ID */
    int installed;           /* 是否已安装 */
};

/* ============================================
 * 函数声明
 * ============================================
 */

/* 初始化hook模块 */
int bl_hook_init(void);

/* 清理hook模块 */
void bl_hook_exit(void);

/* 安装inline hook */
int bl_install_hook(struct bl_hook *hook);

/* 移除inline hook */
int bl_remove_hook(struct bl_hook *hook);

/* 调用原始函数（跳板） */
unsigned long bl_call_orig(struct bl_hook *hook, ...);

/* 内存权限修改 */
int bl_make_page_writable(void *addr);
int bl_make_page_readonly(void *addr);

/* 刷新指令缓存 */
void bl_flush_icache_range(void *start, void *end);

#endif /* BINGLENG_ARM64_HOOK_H */
