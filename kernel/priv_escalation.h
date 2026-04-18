/*
 * 冰冷 (BingLeng) - 权限提升头文件
 *
 * 功能：提供权限提升和SELinux绕过的函数声明
 *
 * 作者：冰冷开发组
 */

#ifndef BINGLENG_PRIV_ESCALATION_H
#define BINGLENG_PRIV_ESCALATION_H

#include <linux/types.h>

/* ============================================
 * 函数声明
 * ============================================
 */

/* 提升当前进程的权限 */
int bl_escalate_privs(void);

/* 绕过SELinux */
int bl_bypass_selinux(void);

#endif /* BINGLENG_PRIV_ESCALATION_H */
