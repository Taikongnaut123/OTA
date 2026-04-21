# 配置文件使用说明

## 修改内容

已将硬编码的脚本路径改为从 `config.yaml` 配置文件读取。

## 文件列表

### 新增文件
- **ConfigManager.h** - 配置管理器头文件
- **ConfigManager.cpp** - 配置管理器实现
- **config.yaml** - 配置文件（可随时修改，无需重新编译）

### 修改文件
- **mainwindow.h** - 添加 ConfigManager 引用
- **mainwindow.cpp** - 使用配置管理器读取配置
- **OTA.pro** - 添加新源文件，配置文件自动复制

## 配置文件说明

`config.yaml` 文件包含以下配置项：

```yaml
upgrade:
  script_path: "/opt/robot/scripts/upgrade_s100.sh"  # 升级脚本路径
  timeout: 3000                                       # 超时时间（毫秒）
  confirm_before_upgrade: true                        # 升级前确认

version:
  current: "v1.0.0.0"  # 当前版本号
  latest: "v1.0.1"     # 最新版本号

ui:
  window_title: "S100 控制器升级工具"
  upgrade_button_text: "升级"
  upgrading_button_text: "升级中..."

logging:
  enabled: true
  log_path: "/var/log/ota_upgrade.log"
  log_level: "info"
```

## 使用方法

1. **修改配置**：直接编辑 `config.yaml` 文件
2. **重启程序**：修改配置后重启应用即可生效
3. **无需编译**：配置文件可随时修改，不需要重新编译代码

## 编译和运行

```bash
cd /home/ubuntu/zhaokai/qt_workspace/OTA
qmake OTA.pro
make
./OTA
```

## 优势

✅ **配置灵活**：修改配置无需重新编译  
✅ **易于维护**：配置集中管理，清晰明了  
✅ **环境适配**：不同环境可使用不同配置文件  
✅ **扩展性好**：可随时添加新的配置项

## 注意事项

- 配置文件路径默认为当前工作目录的 `config.yaml`
- 如果配置文件加载失败，程序会使用默认值并弹出警告
- 修改配置文件后需要重启程序才能生效
