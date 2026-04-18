/*
 * 冰冷 (BingLeng) - 内核符号查找实现
 *
 * 功能：提供查找内核符号地址的功能，支持多种方法
 *
 * 作者：冰冷开发组
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include "kallsyms.h"
#include "utils.h"

/* ============================================
 * 关键符号地址缓存
 * ============================================
 */
static unsigned long addr_sys_setuid = 0;
static unsigned long addr_sys_execve = 0;
static unsigned long addr_sys_openat = 0;
static unsigned long addr_prepare_creds = 0;
static unsigned long addr_commit_creds = 0;
static unsigned long addr_selinux_inode_permission = 0;
static unsigned long addr_seq_read = 0;
static unsigned long addr_kallsyms_lookup_name = 0;

/* kallsyms_lookup_name函数指针 */
static kallsyms_lookup_name_t p_kallsyms_lookup_name = NULL;

/* ============================================
 * 方法1: 通过/proc/kallsyms解析
 * ============================================
 */

/**
 * bl_kallsyms_parse_proc - 通过/proc/kallsyms查找符号
 */
static unsigned long bl_kallsyms_parse_proc(const char *name)
{
    struct file *file = NULL;
    char buf[256];
    unsigned long addr = 0;
    loff_t pos = 0;
    ssize_t nread;
    char *line, *saveptr;

    bl_printk(BL_LOG_DEBUG, "尝试通过/proc/kallsyms查找: %s\n", name);

    /* 打开/proc/kallsyms */
    file = filp_open("/proc/kallsyms", O_RDONLY, 0);
    if (IS_ERR(file)) {
        bl_printk(BL_LOG_ERR, "无法打开/proc/kallsyms: %ld\n", PTR_ERR(file));
        return 0;
    }

    /* 逐行读取 */
    while ((nread = kernel_read(file, buf, sizeof(buf) - 1, &pos)) > 0) {
        buf[nread] = '\0';

        /* 分割行 */
        line = strtok_r(buf, "\n", &saveptr);
        while (line != NULL) {
            char sym_name[KSYM_NAME_LEN];
            char sym_type;
            unsigned long sym_addr;

            /* 解析行格式: "address type name" */
            if (sscanf(line, "%lx %c %s", &sym_addr, &sym_type, sym_name) == 3) {
                if (strcmp(sym_name, name) == 0) {
                    addr = sym_addr;
                    bl_printk(BL_LOG_INFO, "找到符号 %s: 0x%lx\n", name, addr);
                    goto out;
                }
            }

            line = strtok_r(NULL, "\n", &saveptr);
        }
    }

out:
    filp_close(file, NULL);
    return addr;
}

/* ============================================
 * 方法2: 使用kallsyms_lookup_name
 * ============================================
 */

/**
 * bl_kallsyms_init - 初始化kallsyms模块
 */
int bl_kallsyms_init(void)
{
    bl_printk(BL_LOG_INFO, "初始化kallsyms模块...\n");

    /* 首先尝试通过/proc/kallsyms找到kallsyms_lookup_name本身 */
    addr_kallsyms_lookup_name = bl_kallsyms_parse_proc(KSYM_kallsyms_lookup_name);
    if (addr_kallsyms_lookup_name != 0) {
        p_kallsyms_lookup_name = (kallsyms_lookup_name_t)addr_kallsyms_lookup_name;
        bl_printk(BL_LOG_INFO, "kallsyms_lookup_name可用: 0x%lx\n", addr_kallsyms_lookup_name);
    } else {
        bl_printk(BL_LOG_WARNING, "kallsyms_lookup_name未找到，将使用/proc/kallsyms\n");
    }

    /* 查找所有关键符号 */
    addr_sys_setuid = bl_kallsyms_lookup(KSYM_sys_setuid);
    if (addr_sys_setuid == 0) {
        addr_sys_setuid = bl_kallsyms_lookup(KSYM___arm64_sys_setuid);
    }

    addr_sys_execve = bl_kallsyms_lookup(KSYM_sys_execve);
    addr_sys_openat = bl_kallsyms_lookup(KSYM_sys_openat);
    addr_prepare_creds = bl_kallsyms_lookup(KSYM_prepare_creds);
    addr_commit_creds = bl_kallsyms_lookup(KSYM_commit_creds);
    addr_selinux_inode_permission = bl_kallsyms_lookup(KSYM_selinux_inode_permission);
    addr_seq_read = bl_kallsyms_lookup(KSYM_seq_read);

    bl_printk(BL_LOG_INFO, "kallsyms初始化完成\n");
    return 0;
}

/**
 * bl_kallsyms_exit - 清理kallsyms模块
 */
void bl_kallsyms_exit(void)
{
    bl_printk(BL_LOG_INFO, "清理kallsyms模块\n");
    p_kallsyms_lookup_name = NULL;
    addr_sys_setuid = 0;
    addr_sys_execve = 0;
    addr_sys_openat = 0;
    addr_prepare_creds = 0;
    addr_commit_creds = 0;
    addr_selinux_inode_permission = 0;
    addr_seq_read = 0;
    addr_kallsyms_lookup_name = 0;
}

/**
 * bl_kallsyms_lookup - 查找内核符号
 */
unsigned long bl_kallsyms_lookup(const char *name)
{
    unsigned long addr = 0;

    /* 首先尝试使用kallsyms_lookup_name */
    if (p_kallsyms_lookup_name != NULL) {
        addr = p_kallsyms_lookup_name(name);
        if (addr != 0) {
            return addr;
        }
    }

    /* 备用方法：通过/proc/kallsyms解析 */
    addr = bl_kallsyms_parse_proc(name);
    return addr;
}

/* ============================================
 * 获取关键符号地址的函数
 * ============================================
 */

unsigned long bl_get_sys_setuid(void) { return addr_sys_setuid; }
unsigned long bl_get_sys_execve(void) { return addr_sys_execve; }
unsigned long bl_get_sys_openat(void) { return addr_sys_openat; }
unsigned long bl_get_prepare_creds(void) { return addr_prepare_creds; }
unsigned long bl_get_commit_creds(void) { return addr_commit_creds; }
unsigned long bl_get_selinux_inode_permission(void) { return addr_selinux_inode_permission; }
unsigned long bl_get_seq_read(void) { return addr_seq_read; }
