/*
 * 冰冷 (BingLeng) - GKI 3.0 兼容头文件
 *
 * 功能：提供Android版本检测、GKI版本检测、KCFI处理等功能
 *
 * 作者：冰冷开发组
 */

#ifndef BINGLENG_GKI_COMPAT_H
#define BINGLENG_GKI_COMPAT_H

#include <linux/types.h>

/* ============================================
 * 函数声明
 * ============================================
 */

/* 初始化GKI兼容模块 */
int bl_gki_init(void);

/* 清理GKI兼容模块 */
void bl_gki_exit(void);

/* 检测Android版本，返回API级别 */
int bl_detect_android_version(void);

/* 检测GKI版本，返回主版本号（如3表示GKI 3.0）*/
int bl_detect_gki_version(void);

/* 检测KCFI是否启用 */
int bl_is_kcfi_enabled(void);

/* 获取函数的CFI类型ID */
unsigned long bl_kcfi_get_typeid(void *func);

/* 设置函数的CFI类型ID */
int bl_kcfi_set_typeid(void *func, unsigned long typeid);

/* 禁用模块签名验证（可选） */
int bl_disable_module_sig(void);

#endif /* BINGLENG_GKI_COMPAT_H */
