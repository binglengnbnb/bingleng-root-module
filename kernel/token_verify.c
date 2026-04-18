/*
 * 冰冷 (BingLeng) - 令牌验证实现
 *
 * 功能：实现64位令牌生成、验证和授权管理
 *
 * 作者：冰冷开发组
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include "token_verify.h"
#include "utils.h"
#include "bingleng_main.h"

/* ============================================
 * xorshift64* 随机数生成器
 * ============================================
 */

static uint64_t xorshift64star(uint64_t x)
{
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    return x * 0x2545F4914F6CDD1DULL;
}

/* ============================================
 * 令牌生成
 * ============================================
 */

/**
 * bl_generate_token - 生成64位随机令牌
 */
uint64_t bl_generate_token(void)
{
    uint64_t token;
    get_random_bytes(&token, sizeof(token));
    token = xorshift64star(token);
    bl_printk(BL_LOG_DEBUG, "生成令牌: 0x%llx\n", token);
    return token;
}

/* ============================================
 * 授权管理
 * ============================================
 */

/**
 * bl_add_auth - 添加授权记录
 */
int bl_add_auth(uid_t uid, uint64_t token, pid_t pid, const char *pkg_name)
{
    struct bl_auth_entry *entry;
    unsigned long flags;

    bl_printk(BL_LOG_INFO, "添加授权: uid=%d, pid=%d, pkg=%s\n", uid, pid, pkg_name);

    /* 分配内存 */
    entry = bl_kmalloc(sizeof(struct bl_auth_entry), GFP_KERNEL);
    if (!entry) {
        bl_printk(BL_LOG_ERR, "分配授权记录内存失败\n");
        return -ENOMEM;
    }

    /* 初始化 */
    entry->uid = uid;
    entry->token = token;
    entry->pid = pid;
    entry->expire = jiffies + msecs_to_jiffies(BINGLENG_TOKEN_EXPIRE * 1000);
    bl_strlcpy(entry->pkg_name, pkg_name, sizeof(entry->pkg_name));
    INIT_LIST_HEAD(&entry->list);

    /* 加锁添加到链表 */
    flags = bl_spin_lock_irqsave(&bl_auth_lock);
    list_add_tail(&entry->list, &bl_auth_list);
    bl_spin_unlock_irqrestore(&bl_auth_lock, flags);

    return 0;
}

/**
 * bl_verify_token - 验证令牌和UID
 */
int bl_verify_token(uid_t uid, uint64_t token)
{
    struct bl_auth_entry *entry;
    unsigned long flags;
    int found = 0;

    /* 首先清理过期令牌 */
    bl_cleanup_expired_tokens();

    /* 查找授权记录 */
    flags = bl_spin_lock_irqsave(&bl_auth_lock);
    list_for_each_entry(entry, &bl_auth_list, list) {
        if (entry->uid == uid && entry->token == token) {
            /* 检查是否过期 */
            if (time_after(jiffies, entry->expire)) {
                bl_printk(BL_LOG_WARNING, "令牌已过期: uid=%d\n", uid);
            } else {
                found = 1;
                /* 刷新过期时间 */
                entry->expire = jiffies + msecs_to_jiffies(BINGLENG_TOKEN_EXPIRE * 1000);
            }
            break;
        }
    }
    bl_spin_unlock_irqrestore(&bl_auth_lock, flags);

    if (found) {
        bl_printk(BL_LOG_DEBUG, "令牌验证成功: uid=%d\n", uid);
    } else {
        bl_printk(BL_LOG_WARNING, "令牌验证失败: uid=%d\n", uid);
    }

    return found;
}

/**
 * bl_revoke_token - 撤销授权
 */
void bl_revoke_token(uid_t uid)
{
    struct bl_auth_entry *entry, *tmp;
    unsigned long flags;

    bl_printk(BL_LOG_INFO, "撤销授权: uid=%d\n", uid);

    flags = bl_spin_lock_irqsave(&bl_auth_lock);
    list_for_each_entry_safe(entry, tmp, &bl_auth_list, list) {
        if (entry->uid == uid) {
            list_del(&entry->list);
            bl_kfree(entry);
            bl_printk(BL_LOG_DEBUG, "已移除授权: uid=%d\n", uid);
        }
    }
    bl_spin_unlock_irqrestore(&bl_auth_lock, flags);
}

/**
 * bl_get_auth_count - 获取当前授权数量
 */
int bl_get_auth_count(void)
{
    struct bl_auth_entry *entry;
    unsigned long flags;
    int count = 0;

    flags = bl_spin_lock_irqsave(&bl_auth_lock);
    list_for_each_entry(entry, &bl_auth_list, list) {
        count++;
    }
    bl_spin_unlock_irqrestore(&bl_auth_lock, flags);

    return count;
}

/**
 * bl_cleanup_expired_tokens - 清理过期令牌
 */
void bl_cleanup_expired_tokens(void)
{
    struct bl_auth_entry *entry, *tmp;
    unsigned long flags;

    flags = bl_spin_lock_irqsave(&bl_auth_lock);
    list_for_each_entry_safe(entry, tmp, &bl_auth_list, list) {
        if (time_after(jiffies, entry->expire)) {
            bl_printk(BL_LOG_DEBUG, "清理过期令牌: uid=%d\n", entry->uid);
            list_del(&entry->list);
            bl_kfree(entry);
        }
    }
    bl_spin_unlock_irqrestore(&bl_auth_lock, flags);
}

/* ============================================
 * 初始化和清理
 * ============================================
 */

int bl_token_init(void)
{
    bl_printk(BL_LOG_INFO, "初始化令牌验证模块\n");
    /* 链表和锁已在bingleng_main.c中初始化 */
    return 0;
}

void bl_token_exit(void)
{
    struct bl_auth_entry *entry, *tmp;
    unsigned long flags;

    bl_printk(BL_LOG_INFO, "清理令牌验证模块\n");

    /* 清理所有授权记录 */
    flags = bl_spin_lock_irqsave(&bl_auth_lock);
    list_for_each_entry_safe(entry, tmp, &bl_auth_list, list) {
        list_del(&entry->list);
        bl_kfree(entry);
    }
    bl_spin_unlock_irqrestore(&bl_auth_lock, flags);
}
