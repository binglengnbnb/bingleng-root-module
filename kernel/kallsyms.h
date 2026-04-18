/*
 * 冰冷 (BingLeng) - 内核符号查找头文件
 *
 * 功能：提供查找内核符号地址的功能，支持多种方法
 *
 * 作者：冰冷开发组
 */

#ifndef BINGLENG_KALLSYMS_H
#define BINGLENG_KALLSYMS_H

#include <linux/types.h>

/* ============================================
 * 关键符号名称定义
 * ============================================
 */
#define KSYM_sys_setuid             "sys_setuid"
#define KSYM___arm64_sys_setuid     "__arm64_sys_setuid"
#define KSYM_sys_execve             "sys_execve"
#define KSYM_sys_openat             "sys_openat"
#define KSYM_prepare_creds          "prepare_creds"
#define KSYM_commit_creds           "commit_creds"
#define KSYM_selinux_inode_permission "selinux_inode_permission"
#define KSYM_seq_read               "seq_read"
#define KSYM_kallsyms_lookup_name   "kallsyms_lookup_name"

/* ============================================
 * 函数指针类型定义
 * ============================================
 */
typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);

/* ============================================
 * 函数声明
 * ============================================
 */

/* 初始化kallsyms模块 */
int bl_kallsyms_init(void);

/* 清理kallsyms模块 */
void bl_kallsyms_exit(void);

/* 查找内核符号地址 */
unsigned long bl_kallsyms_lookup(const char *name);

/* 获取关键符号地址 */
unsigned long bl_get_sys_setuid(void);
unsigned long bl_get_sys_execve(void);
unsigned long bl_get_sys_openat(void);
unsigned long bl_get_prepare_creds(void);
unsigned long bl_get_commit_creds(void);
unsigned long bl_get_selinux_inode_permission(void);
unsigned long bl_get_seq_read(void);

#endif /* BINGLENG_KALLSYMS_H */
