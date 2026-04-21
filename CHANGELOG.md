# 更新日志

所有重要的项目变更都会记录在此文件中。

格式基于 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.0.0/)，
版本号遵循 [语义化版本](https://semver.org/lang/zh-CN/)。

## [Unreleased]

### 新增
- GitHub Actions CI/CD 自动化流程
- 从 config.yaml 读取配置，无需重新编译
- 恢复功能（recovery_button）
- ConfigManager 配置管理器

### 变更
- 重构升级和恢复逻辑，使用通用的 executeScript 方法

### 修复
- 修复硬编码脚本路径问题

## [1.0.0] - 2026-04-21

### 新增
- 初始版本发布
- S100 控制器升级功能
- 实时显示执行进度
- 版本信息显示
- 友好的用户界面

---

## 版本标签说明

- `新增` - 新功能
- `变更` - 现有功能的变更
- `弃用` - 即将移除的功能
- `移除` - 已移除的功能
- `修复` - Bug 修复
- `安全` - 安全相关的修复
