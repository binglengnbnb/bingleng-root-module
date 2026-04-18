/*
 * 冰冷 (BingLeng) - 工具函数实现
 *
 * 功能：实现内存管理、日志输出、锁操作等基础工具函数
 *
 * 作者：冰冷开发组
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/printk.h>
#include <linux/string.h>
#include <linux/stdarg.h>
#include "utils.h"

/* 调试日志级别（由模块参数控制） */
extern int debug;

/* ============================================
 * 内存管理函数
 * ============================================
 */

/**
 * bl_kmalloc - 安全的内存分配函数
 * @size: 要分配的字节数
 * @flags: 分配标志（GFP_KERNEL等）
 *
 * 返回值：成功返回分配的内存指针，失败返回NULL
 *
 * 安全说明：优先使用kmalloc，失败时尝试vmalloc，所有返回值必须检查NULL
 */
void *bl_kmalloc(size_t size, gfp_t flags)
{
    void *ptr = NULL;

    /* 边界检查：拒绝零大小分配 */
    if (size == 0) {
        pr_err("[bingleng] bl_kmalloc: 零大小分配请求\n");
        return NULL;
    }

    /* 首先尝试使用kmalloc */
    ptr = kmalloc(size, flags);
    if (ptr != NULL) {
        return ptr;
    }

    /* kmalloc失败，尝试vmalloc */
    pr_warn("[bingleng] bl_kmalloc: kmalloc失败，尝试vmalloc (size=%zu)\n", size);
    ptr = vmalloc(size);
    if (ptr == NULL) {
        pr_err("[bingleng] bl_kmalloc: vmalloc也失败\n");
    }

    return ptr;
}
EXPORT_SYMBOL(bl_kmalloc);

/**
 * bl_kfree - 安全的内存释放函数
 * @ptr: 要释放的内存指针
 *
 * 安全说明：自动检测是kmalloc还是vmalloc分配的内存，使用对应的释放函数
 */
void bl_kfree(const void *ptr)
{
    /* 安全检查：NULL指针检查 */
    if (ptr == NULL) {
        return;
    }

    /* 判断是kmalloc还是vmalloc分配的 */
    if (is_vmalloc_addr(ptr)) {
        vfree(ptr);
    } else {
        kfree(ptr);
    }
}
EXPORT_SYMBOL(bl_kfree);

/* ============================================
 * 日志输出函数
 * ============================================
 */

/**
 * bl_printk - 带级别控制的日志输出函数
 * @level: 日志级别 (BL_LOG_*)
 * @fmt: 格式化字符串
 *
 * 安全说明：根据模块参数debug控制调试输出
 */
void bl_printk(int level, const char *fmt, ...)
{
    va_list args;
    const char *prefix = "[bingleng] ";

    /* 调试级别检查 */
    if (level > BL_LOG_INFO && !debug) {
        return;
    }

    va_start(args, fmt);

    switch (level) {
        case BL_LOG_EMERG:
            pr_emerg("%s", prefix);
            vprintk(fmt, args);
            break;
        case BL_LOG_ALERT:
            pr_alert("%s", prefix);
            vprintk(fmt, args);
            break;
        case BL_LOG_CRIT:
            pr_crit("%s", prefix);
            vprintk(fmt, args);
            break;
        case BL_LOG_ERR:
            pr_err("%s", prefix);
            vprintk(fmt, args);
            break;
        case BL_LOG_WARNING:
            pr_warn("%s", prefix);
            vprintk(fmt, args);
            break;
        case BL_LOG_NOTICE:
            pr_notice("%s", prefix);
            vprintk(fmt, args);
            break;
        case BL_LOG_INFO:
            pr_info("%s", prefix);
            vprintk(fmt, args);
            break;
        case BL_LOG_DEBUG:
            pr_debug("%s", prefix);
            vprintk(fmt, args);
            break;
        default:
            pr_info("%s", prefix);
            vprintk(fmt, args);
            break;
    }

    va_end(args);
}
EXPORT_SYMBOL(bl_printk);

/* ============================================
 * 字符串操作函数
 * ============================================
 */

/**
 * bl_strlcpy - 安全的字符串复制函数
 * @dest: 目标缓冲区
 * @src: 源字符串
 * @size: 目标缓冲区大小
 *
 * 返回值：源字符串长度
 *
 * 安全说明：确保目标缓冲区不会溢出
 */
size_t bl_strlcpy(char *dest, const char *src, size_t size)
{
    size_t src_len = strlen(src);

    /* 边界检查 */
    if (size == 0 || dest == NULL || src == NULL) {
        return 0;
    }

    if (src_len < size) {
        /* 源字符串短于目标缓冲区，完整复制 */
        memcpy(dest, src, src_len + 1);
    } else {
        /* 源字符串过长，截断复制 */
        memcpy(dest, src, size - 1);
        dest[size - 1] = '\0';
    }

    return src_len;
}
EXPORT_SYMBOL(bl_strlcpy);
