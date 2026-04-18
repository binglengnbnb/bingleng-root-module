#
# 冰冷 (BingLeng) - 内核模块 Makefile
#
# 编译命令示例:
#   make KERNEL_SRC=/path/to/kernel CROSS_COMPILE=aarch64-linux-gnu- ANDROID_API=36 modules
#   make KERNEL_SRC=/path/to/kernel CROSS_COMPILE=aarch64-linux-gnu- kpm
#
# [TODO: 根据目标设备内核版本调整 KERNEL_SRC 和 CROSS_COMPILE]
#

MODULE_NAME := bingleng

# 源文件列表
obj-m += $(MODULE_NAME).o
$(MODULE_NAME)-objs := \
    bingleng_main.o \
    utils.o \
    kallsyms.o \
    token_verify.o \
    gki_compat.o \
    arm64_hook.o \
    syscall_hook.o \
    priv_escalation.o \
    hide.o

# 内核源码路径（需要用户指定）
KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build

# 交叉编译工具链（需要用户指定）
CROSS_COMPILE ?= aarch64-linux-gnu-

# Android API 级别
ANDROID_API ?= 36

# 编译标志
ccflags-y := -Wall -Wextra
ccflags-y += -DCONFIG_ANDROID_API=$(ANDROID_API)

# 根据 Android API 定义兼容性宏
ifeq ($(shell test $(ANDROID_API) -ge 29 && echo 1),1)
    ccflags-y += -DCONFIG_ANDROID_10_COMPAT
endif
ifeq ($(shell test $(ANDROID_API) -ge 31 && echo 1),1)
    ccflags-y += -DCONFIG_ANDROID_12_COMPAT
endif
ifeq ($(shell test $(ANDROID_API) -ge 36 && echo 1),1)
    ccflags-y += -DCONFIG_ANDROID_16_COMPAT
endif

# 默认目标
.PHONY: all modules clean kpm

all: modules

modules:
	@echo "=========================================="
	@echo "  编译冰冷内核模块"
	@echo "  Android API: $(ANDROID_API)"
	@echo "=========================================="
	make -C $(KERNEL_SRC) M=$(shell pwd) \
		CROSS_COMPILE=$(CROSS_COMPILE) \
		ARCH=arm64 \
		modules

clean:
	@echo "清理..."
	make -C $(KERNEL_SRC) M=$(shell pwd) clean
	rm -f *.o *.ko *.mod.o *.mod.c *.symvers *.order .*.cmd
	rm -rf .tmp_versions
	rm -f $(MODULE_NAME).kpm

# KPM (Kernel Package Module) 打包
kpm: modules
	@echo "=========================================="
	@echo "  打包 KPM 文件"
	@echo "=========================================="
	# TODO: 实现KPM打包逻辑
	# 这里只是一个占位符，实际需要根据KPM格式实现
	@echo "KPM打包功能待实现"
	@echo "生成的 .ko 文件: $(MODULE_NAME).ko"

# 帮助信息
help:
	@echo "冰冷 (BingLeng) 内核模块编译"
	@echo ""
	@echo "可用目标:"
	@echo "  modules  - 编译内核模块 (.ko)"
	@echo "  kpm      - 编译并打包为 KPM"
	@echo "  clean    - 清理编译文件"
	@echo "  help     - 显示此帮助信息"
	@echo ""
	@echo "变量:"
	@echo "  KERNEL_SRC     - 内核源码路径 (必需)"
	@echo "  CROSS_COMPILE  - 交叉编译工具链前缀 (必需)"
	@echo "  ANDROID_API    - Android API 级别 (默认: 36)"
	@echo ""
	@echo "示例:"
	@echo "  make KERNEL_SRC=~/kernel CROSS_COMPILE=aarch64-linux-gnu- modules"
	@echo "  make KERNEL_SRC=~/kernel CROSS_COMPILE=aarch64-linux-gnu- ANDROID_API=36 kpm"
