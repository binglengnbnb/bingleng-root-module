/*
 * 冰冷 (BingLeng) - ARM64 inline hook 实现
 *
 * 功能：提供ARM64 inline hook功能，支持KCFI兼容
 *
 * 作者：冰冷开发组
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <asm/cacheflush.h>
#include <asm/pgtable.h>
#include "arm64_hook.h"
#include "utils.h"

/* BRK #0x800 指令编码 */
#define BRK_INSN 0xd4201000

/* ============================================
 * 内存权限修改
 * ============================================
 */

/**
 * bl_make_page_writable - 设置内存页为可写
 */
int bl_make_page_writable(void *addr)
{
    struct page *page;
    pte_t *pte;
    spinlock_t *ptl;

    if (!addr) {
        return -EINVAL;
    }

    page = virt_to_page(addr);
    if (!page) {
        return -EFAULT;
    }

    /* 获取页表项 */
    pte = virt_to_pte(addr, &ptl);
    if (!pte) {
        return -EFAULT;
    }

    /* 设置可写权限 */
    *pte = pte_mkwrite(*pte);
    pte_unmap_unlock(pte, ptl);

    return 0;
}

/**
 * bl_make_page_readonly - 设置内存页为只读
 */
int bl_make_page_readonly(void *addr)
{
    struct page *page;
    pte_t *pte;
    spinlock_t *ptl;

    if (!addr) {
        return -EINVAL;
    }

    page = virt_to_page(addr);
    if (!page) {
        return -EFAULT;
    }

    pte = virt_to_pte(addr, &ptl);
    if (!pte) {
        return -EFAULT;
    }

    /* 清除可写权限 */
    *pte = pte_wrprotect(*pte);
    pte_unmap_unlock(pte, ptl);

    return 0;
}

/**
 * bl_flush_icache_range - 刷新指令缓存
 */
void bl_flush_icache_range(void *start, void *end)
{
    __flush_icache_range((unsigned long)start, (unsigned long)end);
}

/* ============================================
 * Hook 安装和移除
 * ============================================
 */

/**
 * bl_install_hook - 安装inline hook
 */
int bl_install_hook(struct bl_hook *hook)
{
    unsigned long *func_ptr;

    if (!hook || !hook->orig_func || !hook->hook_func) {
        return -EINVAL;
    }

    if (hook->installed) {
        bl_printk(BL_LOG_WARNING, "Hook已安装\n");
        return 0;
    }

    bl_printk(BL_LOG_INFO, "安装hook: orig=%p, hook=%p\n", hook->orig_func, hook->hook_func);

    /* 保存原始指令 */
    func_ptr = (unsigned long *)hook->orig_func;
    hook->orig_inst[0] = func_ptr[0];
    hook->orig_inst[1] = func_ptr[1];
    hook->orig_inst[2] = func_ptr[2];
    hook->orig_inst[3] = func_ptr[3];

    /* TODO: 保存KCFI类型ID */
    hook->cfi_typeid = 0;

    /* 设置内存为可写 */
    if (bl_make_page_writable(hook->orig_func) < 0) {
        bl_printk(BL_LOG_ERR, "设置内存可写失败\n");
        return -EFAULT;
    }

    /* 替换为BRK #0x800指令 */
    func_ptr[0] = BRK_INSN;

    /* 刷新指令缓存 */
    bl_flush_icache_range(hook->orig_func, hook->orig_func + 16);

    /* 恢复内存只读 */
    bl_make_page_readonly(hook->orig_func);

    hook->installed = 1;
    bl_printk(BL_LOG_INFO, "Hook安装成功\n");

    return 0;
}

/**
 * bl_remove_hook - 移除inline hook
 */
int bl_remove_hook(struct bl_hook *hook)
{
    unsigned long *func_ptr;

    if (!hook || !hook->orig_func) {
        return -EINVAL;
    }

    if (!hook->installed) {
        bl_printk(BL_LOG_WARNING, "Hook未安装\n");
        return 0;
    }

    bl_printk(BL_LOG_INFO, "移除hook: orig=%p\n", hook->orig_func);

    /* 设置内存为可写 */
    if (bl_make_page_writable(hook->orig_func) < 0) {
        bl_printk(BL_LOG_ERR, "设置内存可写失败\n");
        return -EFAULT;
    }

    /* 恢复原始指令 */
    func_ptr = (unsigned long *)hook->orig_func;
    func_ptr[0] = hook->orig_inst[0];
    func_ptr[1] = hook->orig_inst[1];
    func_ptr[2] = hook->orig_inst[2];
    func_ptr[3] = hook->orig_inst[3];

    /* 刷新指令缓存 */
    bl_flush_icache_range(hook->orig_func, hook->orig_func + 16);

    /* 恢复内存只读 */
    bl_make_page_readonly(hook->orig_func);

    hook->installed = 0;
    bl_printk(BL_LOG_INFO, "Hook移除成功\n");

    return 0;
}

/**
 * bl_call_orig - 调用原始函数
 */
unsigned long bl_call_orig(struct bl_hook *hook, ...)
{
    /* TODO: 实现跳板代码调用原始函数 */
    /* [TODO: 实现跳板代码，执行原始指令并跳转回原函数] */
    bl_printk(BL_LOG_WARNING, "bl_call_orig未实现\n");
    return 0;
}

/* ============================================
 * 初始化和清理
 * ============================================
 */

int bl_hook_init(void)
{
    bl_printk(BL_LOG_INFO, "初始化ARM64 hook模块\n");
    /* TODO: 注册die notifier用于异常处理 */
    return 0;
}

void bl_hook_exit(void)
{
    bl_printk(BL_LOG_INFO, "清理ARM64 hook模块\n");
}
