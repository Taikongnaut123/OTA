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

从 [Releases](https://github.com/Taikongnaut123/OTA/releases/tag/latest) 页面下载最新版本。

---

### 📥 x86_64 平台

#### 方式一：AppImage（推荐，零依赖）

```bash
# 1. 下载 AppImage 文件
wget https://github.com/Taikongnaut123/OTA/releases/download/latest/OTA-x86_64.AppImage

# 2. 添加执行权限
chmod +x OTA-x86_64.AppImage

# 3. 直接运行（无需安装任何依赖）
./OTA-x86_64.AppImage
```

#### 方式二：tar.gz 压缩包

```bash
# 1. 下载并解压
wget https://github.com/Taikongnaut123/OTA/releases/download/latest/OTA-linux-x64.tar.gz
tar -xzf OTA-linux-x64.tar.gz
cd release

# 2. 安装 Qt5 运行时（如果未安装）
sudo apt-get install -y libqt5core5a libqt5gui5 libqt5widgets5  # Ubuntu/Debian
# 或
sudo dnf install -y qt5-qtbase qt5-qtbase-gui                     # Fedora/RHEL
# 或
sudo pacman -S qt5-base                                           # Arch Linux

# 3. 运行
chmod +x OTA
./OTA
```

---

### 📥 ARM64 平台

```bash
# 1. 下载并解压
wget https://github.com/Taikongnaut123/OTA/releases/download/latest/OTA-linux-arm64.tar.gz
tar -xzf OTA-linux-arm64.tar.gz
cd release

# 2. 安装 Qt5 运行时（如果未安装）
sudo apt-get update
sudo apt-get install -y libqt5core5a libqt5gui5 libqt5widgets5

# 3. 运行
chmod +x OTA
./OTA
```

---

### 🔍 运行环境依赖

| 平台 | 发行版本 | 运行时依赖 |
|------|---------|-----------|
| **x86_64** | AppImage | ❌ 无依赖（自包含） |
| **x86_64** | tar.gz | ✅ Qt5 运行时库 |
| **ARM64** | tar.gz | ✅ Qt5 运行时库 |

**所需 Qt 模块：**
- `libqt5core5a` - Qt5 核心库
- `libqt5gui5` - Qt5 GUI 库
- `libqt5widgets5` - Qt5 Widgets 库

> 💡 **推荐**：x86_64 用户优先使用 AppImage 版本，开箱即用无需安装依赖。

---

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

- Qt 5.12 或更高版本（推荐 5.15）
- GCC/Clang 编译器（支持 C++11）
- Make 或 CMake
- qttools5-dev-tools（如需翻译功能）

### 本地编译

```bash
# 克隆仓库
git clone https://github.com/Taikongnaut123/OTA.git
cd OTA

# 编译
mkdir build
cd build
qmake ../OTA.pro CONFIG+=release
make -j$(nproc)

# 运行
./release/OTA
```

### 开发依赖安装

**Ubuntu/Debian：**
```bash
sudo apt-get install -y \
    qtbase5-dev \
    qt5-qmake \
    qttools5-dev-tools \
    build-essential
```

**Fedora/RHEL：**
```bash
sudo dnf install -y \
    qt5-qtbase-devel \
    qt5-qttools-devel \
    make gcc-c++
```

---

## 📦 CI/CD 发布流程

本项目使用 GitHub Actions 实现多平台自动化编译和发布。

### 🔄 自动构建触发

推送到 `main` 分支时自动触发构建：

```bash
git add .
git commit -m "your changes"
git push origin main
```

**两个独立的 CI 流程会并行运行：**

| CI 文件 | 平台 | 产物 |
|---------|------|------|
| `build-and-release.yml` | x86_64 | AppImage + tar.gz |
| `build-and-release-arm64.yml` | ARM64 | tar.gz |

### 🏗️ 构建流程说明

#### x86_64 构建流程：
1. ✅ 安装 Qt 5.15.2（通过 install-qt-action）
2. ✅ 编译 Release 版本
3. ✅ 创建 AppImage（使用 linuxdeploy）
4. ✅ 打包 tar.gz（可执行文件 + 配置文件）
5. ✅ 生成 SHA256 校验和
6. ✅ 上传到 `latest` Release

#### ARM64 构建流程：
1. ✅ 设置 QEMU + Docker Buildx
2. ✅ 在 ARM64 容器中编译（arm64v8/ubuntu:22.04）
3. ✅ 打包 tar.gz
4. ✅ 生成 SHA256 校验和
5. ✅ 上传到 `latest` Release

### 📦 发布产物

每次构建会生成以下文件并上传到 [`latest` Release](https://github.com/Taikongnaut123/OTA/releases/tag/latest)：

**x86_64 平台：**
- `OTA-x86_64.AppImage` - AppImage 自包含包（推荐）
- `OTA-x86_64.AppImage.sha256` - AppImage 校验和
- `OTA-linux-x64.tar.gz` - tar.gz 打包（可执行文件 + 配置 + 说明）
- `OTA-linux-x64.tar.gz.sha256` - tar.gz 校验和

**ARM64 平台：**
- `OTA-linux-arm64.tar.gz` - ARM64 打包
- `OTA-linux-arm64.tar.gz.sha256` - ARM64 校验和

### 🔐 校验文件完整性

```bash
# 下载文件和校验和
wget https://github.com/Taikongnaut123/OTA/releases/download/latest/OTA-linux-x64.tar.gz
wget https://github.com/Taikongnaut123/OTA/releases/download/latest/OTA-linux-x64.tar.gz.sha256

# 验证文件完整性
sha256sum -c OTA-linux-x64.tar.gz.sha256
```

### ⚙️ 手动触发构建

除了自动触发，也可以手动触发构建：

1. 进入 [Actions 页面](https://github.com/Taikongnaut123/OTA/actions)
2. 选择对应的 workflow：
   - `Build and Release OTA (x86_64)` - x86_64 平台
   - `Build and Release OTA (ARM64)` - ARM64 平台
3. 点击 "Run workflow" 按钮
4. 选择分支（通常是 `main`）并运行

---

## 🔧 项目结构

```
OTA/
├── .github/
│   └── workflows/
│       ├── build-and-release.yml       # x86_64 CI 配置
│       └── build-and-release-arm64.yml # ARM64 CI 配置
├── main.cpp                            # 主程序入口
├── mainwindow.h/cpp                    # 主窗口实现
├── mainwindow.ui                       # UI 设计文件
├── ConfigManager.h/cpp                 # 配置管理器（YAML 解析）
├── config.yaml                         # 运行时配置文件
├── OTA.pro                             # Qt 项目文件
├── OTA.desktop                         # Desktop entry（AppImage 用）
├── OTA.svg                             # 应用图标
├── README.md                           # 本文件
└── README_CONFIG.md                    # 配置文件详细说明
```
```

## 📋 开发工作流

### 日常开发

```bash
# 1. 创建功能分支
git checkout -b feature/your-feature

# 2. 开发和测试
# ... 编写代码 ...

# 3. 提交更改
git add .
git commit -m "Add: your feature description"

# 4. 推送到远程
git push origin feature/your-feature

# 5. 创建 Pull Request 合并到 main
```

### 发布新版本

```bash
# 1. 更新版本号（config.yaml）
vim config.yaml  # 修改 version.current 和 version.latest

# 2. 提交版本变更
git add config.yaml
git commit -m "Bump version to v1.x.x"

# 3. 推送到 main 分支（自动触发 CI）
git push origin main

# 4. 等待 CI 完成（约 10-15 分钟）
# - x86_64 构建：约 8 分钟
# - ARM64 构建：约 12 分钟（QEMU 模拟较慢）

# 5. 检查 Release 页面的产物
# https://github.com/Taikongnaut123/OTA/releases/tag/latest
```

> 💡 **提示**：推送到 `main` 分支会自动触发两个平台的构建，无需手动创建 tag。

## 🐛 故障排查

### 运行时错误：libGL error / OpenGL 错误

**错误现象：**
```
libGL error: glx: failed to create dri3 screen
libGL error: failed to load driver: rockchip
```

**原因：** 缺少图形显示环境或 OpenGL 库

**解决方案：**

#### 方案1：安装 OpenGL 库（推荐）

```bash
# Ubuntu/Debian - 基础方案（通常足够）
sudo apt-get install -y libgl1-mesa-glx

# 如果仍有问题，再安装完整的 Qt 平台插件依赖
sudo apt-get install -y \
    libgl1-mesa-glx \
    libxcb-xinerama0 \
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-render-util0 \
    libxkbcommon-x11-0
```

#### 方案2：使用软件渲染

如果是远程 SSH 环境或无显示器服务器：

```bash
# 安装虚拟显示
sudo apt-get install -y xvfb

# 使用 Xvfb 运行
xvfb-run ./OTA

# 或设置环境变量使用软件渲染
export QT_QPA_PLATFORM=offscreen
./OTA
```

#### 方案3：X11 转发（SSH 远程）

如果通过 SSH 连接：

```bash
# 本地机器启用 X11 转发
ssh -X user@remote-host

# 或编辑 ~/.ssh/config
Host remote-host
    ForwardX11 yes
    ForwardX11Trusted yes

# 远程机器确保安装 X11 相关包
sudo apt-get install -y xauth x11-apps

# 测试 X11 转发
xclock  # 应该弹出时钟窗口
```

#### 方案4：VNC 远程桌面

```bash
# 安装 VNC 服务器
sudo apt-get install -y tigervnc-standalone-server

# 启动 VNC
vncserver :1

# 从本地连接（使用 VNC 客户端连接到 remote-host:5901）
```

---

### CI 构建失败

1. 检查 [Actions 日志](https://github.com/Taikongnaut123/OTA/actions)找出具体错误
2. 常见问题：
   - **Qt 版本不兼容**：修改 workflow 中的 Qt 版本
   - **编译错误**：本地先测试编译是否通过
   - **权限问题**：检查 GitHub token 权限
   - **ARM64 构建慢**：QEMU 模拟较慢，正常需要 10-15 分钟

---

### 本地编译失败

```bash
# 检查 Qt 安装
qmake --version

# 检查依赖
ldd build/release/OTA

# 清理重新构建
rm -rf build
mkdir build && cd build
qmake ../OTA.pro CONFIG+=release
make clean && make -j$(nproc)
```

---

### 运行时找不到配置文件

```bash
# 确保 config.yaml 在可执行文件同目录
ls -la config.yaml

# 或使用绝对路径
./OTA --config /path/to/config.yaml
```

## 📄 许可证

本项目遵循 MIT 许可证。

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

---

**注意**：首次设置时，请在 `config.yaml` 中配置正确的脚本路径。
