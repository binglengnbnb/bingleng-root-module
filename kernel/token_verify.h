/*
 * 冰冷 (BingLeng) - 令牌验证头文件
 *
 * 功能：实现64位令牌生成、验证和授权管理
 *
 * 作者：冰冷开发组
 */

#ifndef BINGLENG_TOKEN_VERIFY_H
#define BINGLENG_TOKEN_VERIFY_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/spinlock.h>

/* ============================================
 * 函数声明
 * ============================================
 */

/* 初始化令牌模块 */
int bl_token_init(void);

/* 清理令牌模块 */
void bl_token_exit(void);

/* 生成64位随机令牌 */
uint64_t bl_generate_token(void);

/* 验证令牌和UID */
int bl_verify_token(uid_t uid, uint64_t token);

/* 添加授权记录 */
int bl_add_auth(uid_t uid, uint64_t token, pid_t pid, const char *pkg_name);

/* 撤销授权 */
void bl_revoke_token(uid_t uid);

/* 获取当前授权数量 */
int bl_get_auth_count(void);

/* 清理过期令牌 */
void bl_cleanup_expired_tokens(void);

#endif /* BINGLENG_TOKEN_VERIFY_H */
