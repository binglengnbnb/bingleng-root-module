/*
 * 冰冷 (BingLeng) - 工具函数头文件
 *
 * 功能：提供内存管理、日志输出、锁操作等基础工具函数
 *
 * 作者：冰冷开发组
 */

#ifndef BINGLENG_UTILS_H
#define BINGLENG_UTILS_H

#include <linux/types.h>
#include <linux/spinlock.h>

/* ============================================
 * 日志级别定义
 * ============================================
 */
#define BL_LOG_EMERG   0
#define BL_LOG_ALERT   1
#define BL_LOG_CRIT    2
#define BL_LOG_ERR     3
#define BL_LOG_WARNING 4
#define BL_LOG_NOTICE  5
#define BL_LOG_INFO    6
#define BL_LOG_DEBUG   7

/* ============================================
 * 函数声明
 * ============================================
 */

/* 内存分配函数：优先使用kmalloc，失败则尝试vmalloc */
void *bl_kmalloc(size_t size, gfp_t flags);

/* 内存释放函数：自动检测kmalloc/vmalloc并正确释放 */
void bl_kfree(const void *ptr);

/* 日志输出函数：带级别控制 */
void bl_printk(int level, const char *fmt, ...);

/* 安全字符串复制函数：带边界检查 */
size_t bl_strlcpy(char *dest, const char *src, size_t size);

/* 自旋锁封装 */
static inline void bl_spin_lock_init(spinlock_t *lock)
{
    spin_lock_init(lock);
}

static inline void bl_spin_lock(spinlock_t *lock)
{
    spin_lock(lock);
}

static inline void bl_spin_unlock(spinlock_t *lock)
{
    spin_unlock(lock);
}

static inline unsigned long bl_spin_lock_irqsave(spinlock_t *lock)
{
    unsigned long flags;
    spin_lock_irqsave(lock, flags);
    return flags;
}

static inline void bl_spin_unlock_irqrestore(spinlock_t *lock, unsigned long flags)
{
    spin_unlock_irqrestore(lock, flags);
}

#endif /* BINGLENG_UTILS_H */
