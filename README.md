# ❄️ 冰冷 (BingLeng) - TB321FU

为联想Y700三代 (TB321FU) 打造的内核级Root管理器

## 📱 设备信息
- 型号: TB321FU (联想Y700三代)
- Android: 14/15
- 内核: 6.1.112-android14
- KMI: android14-6.1

## 🚀 快速开始

### 1. 准备编译
双击运行 `点击我开始.bat`

### 2. 创建GitHub仓库
访问 https://github.com/new

### 3. 推送代码
按照提示完成git push

### 4. 触发Actions
在GitHub Actions页面运行Workflow

### 5. 下载模块
从Artifacts下载bingleng.ko

## 📖 详细指南

查看 `BUILD_GUIDE.html` 了解完整步骤。

## 📦 制作刷机包

将编译好的 `bingleng.ko` 放入 `../anykernel3/` 目录，然后打包成ZIP。

## ⚠️ 重要提示

- **刷前务必备份原始boot分区！**
- 确认bootloader已解锁
- 了解KMI版本匹配的重要性

## 🔗 相关资源

- KernelSU-Next: https://github.com/KernelSU-Next/KernelSU-Next
- Y700-Gen3-Droidian: https://github.com/AkarinServer/Y700-Gen3-Droidian

## 📄 许可证

本项目仅供学习和研究使用。
