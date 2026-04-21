# OTA 升级工具

S100 控制器升级和恢复工具，支持从配置文件读取脚本路径，无需重新编译。

## ✨ 特性

- ✅ 支持升级和恢复功能
- ✅ 配置文件化：从 `config.yaml` 读取所有配置
- ✅ 实时显示执行进度
- ✅ 友好的用户界面
- ✅ 自动化 CI/CD 流程

## 🚀 快速开始

### 下载安装

从 [Releases](../../releases) 页面下载最新版本：

**方式一：AppImage（推荐）**
```bash
# 1. 下载 AppImage 文件
wget https://github.com/YOUR_USERNAME/OTA/releases/latest/download/OTA-x86_64.AppImage

# 2. 添加执行权限
chmod +x OTA-x86_64.AppImage

# 3. 运行
./OTA-x86_64.AppImage
```

**方式二：tar.gz 压缩包**
```bash
# 1. 下载并解压
wget https://github.com/YOUR_USERNAME/OTA/releases/latest/download/OTA-linux-x64.tar.gz
tar -xzf OTA-linux-x64.tar.gz
cd release

# 2. 安装 Qt5（如果未安装）
sudo apt-get install qt5-default  # Ubuntu/Debian
# 或
sudo yum install qt5-qtbase       # CentOS/RHEL

# 3. 运行
chmod +x OTA
./OTA
```

### 配置

编辑 `config.yaml` 文件来配置脚本路径和其他选项：

```yaml
upgrade:
  script_path: "/opt/upgrade_s100.sh"
  timeout: 3000

recovery:
  script_path: "/opt/recovery_s100.sh"
  timeout: 3000

version:
  current: "v1.0.0.0"
  latest: "v1.0.1"
```

修改配置后只需重启应用，无需重新编译。

详细配置说明请查看 [README_CONFIG.md](README_CONFIG.md)

## 🏗️ 开发构建

### 环境要求

- Qt 5.12 或更高版本
- GCC/Clang 编译器
- Make

### 本地编译

```bash
# 克隆仓库
git clone https://github.com/YOUR_USERNAME/OTA.git
cd OTA

# 编译
mkdir build
cd build
qmake ../OTA.pro
make

# 运行
./OTA
```

## 📦 CI/CD 发布流程

本项目使用 GitHub Actions 自动化编译和发布流程。

### 自动发布新版本

只需创建并推送一个 tag，CI 会自动编译并创建 Release：

```bash
# 1. 创建 tag（遵循语义化版本）
git tag v1.0.0

# 2. 推送 tag 到远程仓库
git push origin v1.0.0
```

**CI 会自动执行以下步骤：**

1. ✅ 检出代码
2. ✅ 安装 Qt 5.15.2
3. ✅ 编译 Release 版本
4. ✅ 创建 AppImage（包含所有依赖）
5. ✅ 打包编译产物（可执行文件 + 配置文件）
6. ✅ 生成 SHA256 校验和
7. ✅ 创建 GitHub Release
8. ✅ 上传所有打包文件到 Release

### 发布产物

每个 Release 包含：

- `OTA-linux-x64.tar.gz` - 完整打包（可执行文件 + 配置 + 说明）
- `OTA-linux-x64.tar.gz.sha256` - SHA256 校验和
- `OTA-x86_64.AppImage` - AppImage 格式（推荐）
- `OTA.AppImage.sha256` - AppImage 校验和

### 手动触发构建

如果需要手动触发构建（不创建 Release）：

1. 进入 GitHub 仓库的 Actions 页面
2. 选择 "Build and Release OTA" workflow
3. 点击 "Run workflow" 按钮
4. 选择分支并运行

## 🔧 项目结构

```
OTA/
├── .github/
│   └── workflows/
│       └── build-and-release.yml  # CI/CD 配置
├── main.cpp                       # 主程序入口
├── mainwindow.h/cpp               # 主窗口
├── mainwindow.ui                  # UI 设计文件
├── ConfigManager.h/cpp            # 配置管理器
├── config.yaml                    # 配置文件
├── OTA.pro                        # Qt 项目文件
├── README.md                      # 本文件
└── README_CONFIG.md               # 配置说明
```

## 📋 版本发布清单

发布新版本时的检查清单：

- [ ] 更新 `config.yaml` 中的版本号
- [ ] 测试所有功能是否正常
- [ ] 更新 CHANGELOG（如果有）
- [ ] 创建 tag：`git tag v1.x.x`
- [ ] 推送 tag：`git push origin v1.x.x`
- [ ] 等待 CI 完成（约 5-10 分钟）
- [ ] 检查 Release 页面的产物
- [ ] 下载并测试发布的安装包

## 🐛 故障排查

### CI 构建失败

1. 检查 Actions 日志找出具体错误
2. 常见问题：
   - Qt 版本不兼容：修改 workflow 中的 Qt 版本
   - 编译错误：本地先测试编译是否通过
   - 权限问题：检查 GitHub token 权限

### 本地编译失败

```bash
# 检查 Qt 安装
qmake --version

# 检查依赖
ldd build/OTA

# 清理重新构建
rm -rf build
mkdir build && cd build
qmake ../OTA.pro
make clean && make
```

## 📄 许可证

本项目遵循 MIT 许可证。

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

---

**注意**：首次设置时，请在 `config.yaml` 中配置正确的脚本路径。
