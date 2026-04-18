/*
 * 冰冷 (BingLeng) - 系统调用拦截头文件
 *
 * 功能：提供系统调用拦截的函数声明
 *
 * 作者：冰冷开发组
 */

#ifndef BINGLENG_SYSCALL_HOOK_H
#define BINGLENG_SYSCALL_HOOK_H

#include <linux/types.h>
#include <asm/ptrace.h>
#include "arm64_hook.h"

/* ============================================
 * 函数声明
 * ============================================
 */

/* 初始化系统调用hook */
int bl_syscall_hook_init(void);

/* 清理系统调用hook */
void bl_syscall_hook_exit(void);

/* Hook后的系统调用 */
asmlinkage long hooked_setuid(uid_t uid);
asmlinkage long hooked_execve(const char __user *filename,
                               const char __user *const __user *argv,
                               const char __user *const __user *envp);
asmlinkage long hooked_openat(int dfd, const char __user *filename,
                               int flags, umode_t mode);

#endif /* BINGLENG_SYSCALL_HOOK_H */
