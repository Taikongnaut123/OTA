# 配置文件使用说明

## 概述

本项目支持通过 `config.yaml` 配置文件管理所有可变参数，无需修改代码或重新编译。

## 配置文件结构

### 完整配置示例

```yaml
# OTA 脚本路径配置
script:
  ota_script_path: "/home/linaro/ota/scripts/ota"

# 服务器配置（源服务器 - 存放升级包）
server:
  from_ip: "192.168.20.204"           # 源服务器 IP
  from_path: "/home/konka-admin/workspace"  # 升级包路径
  from_user: "konka-admin"            # 源服务器用户名
  from_pass: "123456"                 # 源服务器密码
  package_name: "tangpa"              # 包名称

# 客户端配置（目标设备 - 要升级的设备）
client:
  to_ip: "192.168.127.10"             # 目标设备 IP
  to_user: "root"                     # 目标设备用户名
  to_pass: "root"                     # 目标设备密码

# 高级配置（可选）
advanced:
  upgrade_timeout: 3000               # 升级操作超时（毫秒）
  recovery_timeout: 3000              # 恢复操作超时（毫秒）
```

## 配置项说明

### 1. OTA 脚本路径

```yaml
script:
  ota_script_path: "/home/linaro/ota/scripts/ota"
```

指定 OTA 脚本的完整路径。

### 2. 服务器配置

```yaml
server:
  from_ip: "192.168.20.204"           # 源服务器 IP 地址
  from_path: "/home/konka-admin/workspace"  # 升级包存放路径
  from_user: "konka-admin"            # SSH 登录用户名
  from_pass: "123456"                 # SSH 登录密码
  package_name: "tangpa"              # 升级包名称
```

**说明：**
- `from_*` 前缀表示源服务器配置
- 程序会从此服务器下载升级包

### 3. 客户端配置

```yaml
client:
  to_ip: "192.168.127.10"             # 目标设备 IP 地址
  to_user: "root"                     # SSH 登录用户名
  to_pass: "root"                     # SSH 登录密码
```

**说明：**
- `to_*` 前缀表示目标客户端配置
- 程序会将升级包部署到此设备

### 4. 高级配置

```yaml
advanced:
  upgrade_timeout: 3000               # 升级操作超时时间（毫秒）
  recovery_timeout: 3000              # 恢复操作超时时间（毫秒）
```

**说明：**
- 超时时间用于等待操作启动
- 默认值为 3000 毫秒（3 秒）

## 使用方式

### 修改配置

1. 编辑 `config.yaml` 文件
2. 修改相应的配置项（IP、用户名、密码等）
3. 保存文件
4. **重启应用**即可生效

### 配置变更示例

**场景 1：更换目标设备**
```yaml
client:
  to_ip: "192.168.127.20"  # 修改为新设备 IP
  to_user: "admin"         # 修改用户名
  to_pass: "newpass"       # 修改密码
```

**场景 2：更换升级包服务器**
```yaml
server:
  from_ip: "192.168.20.100"  # 新服务器 IP
  from_path: "/data/packages"  # 新路径
```

**场景 3：更换升级包**
```yaml
server:
  package_name: "new_package"  # 新包名称
```

## 动态版本管理

程序会自动查询最新版本号，无需在配置文件中指定版本号。

**工作流程：**
1. 程序启动时自动查询最新版本
2. 查询当前设备运行的版本
3. 升级时自动使用最新版本号

**命令模板：**
```bash
# 查询最新版本
${OTA_SCRIPT} --remote-latest --from-ip ${FROM_IP} ...

# 查询当前版本
${OTA_SCRIPT} --current-version --to-ip ${TO_IP} ...

# 执行升级（自动使用最新版本）
${OTA_SCRIPT} --from-ip ${FROM_IP} ... --package-version ${LATEST_VERSION} --force
```

所有占位符（如 `${FROM_IP}`、`${LATEST_VERSION}`）都会在运行时自动替换。

## 安全建议

### 保护配置文件

密码以明文形式存储在配置文件中，建议设置合适的文件权限：

```bash
# 设置配置文件仅所有者可读写
chmod 600 config.yaml

# 验证权限
ls -l config.yaml
# 输出应为：-rw------- 1 user group ... config.yaml
```

### 最佳实践

1. **不要将 config.yaml 提交到版本控制系统**
   ```bash
   echo "config.yaml" >> .gitignore
   ```

2. **使用模板文件**
   - 创建 `config.yaml.example` 作为模板
   - 在模板中使用占位符（如 `YOUR_PASSWORD`）
   - 用户复制模板并填写实际值

3. **定期更换密码**
   - 定期修改服务器和客户端密码
   - 及时更新配置文件

## 日志文件

所有操作都会记录到日志文件：

**日志位置：**
```
~/ota_upgrade.log
```

**查看日志：**
```bash
# 实时查看日志
tail -f ~/ota_upgrade.log

# 查看最近 50 行
tail -n 50 ~/ota_upgrade.log

# 搜索错误信息
grep "错误" ~/ota_upgrade.log
```

**日志内容示例：**
```
[2026-04-29 10:30:15] ==================== OTA 应用启动 ====================
[2026-04-29 10:30:15] 日志文件路径: /home/user/ota_upgrade.log
[2026-04-29 10:30:16] ========== 开始查询版本信息 ==========
[2026-04-29 10:30:16] 查询最新版本命令: /home/linaro/ota/scripts/ota ...
[2026-04-29 10:30:17] ✓ 最新版本查询成功: 1.0.3 (已保存用于升级)
[2026-04-29 10:30:17] ✓ 当前版本查询成功: 1.0.2
[2026-04-29 10:30:20] 用户点击升级按钮
[2026-04-29 10:30:20] 将升级到版本: 1.0.3
[2026-04-29 10:33:15] 升级执行成功！退出码: 0
```

## 故障排查

### 配置文件加载失败

**问题：** 程序提示"无法加载配置文件 config.yaml"

**解决：**
1. 检查文件是否存在：`ls -l config.yaml`
2. 检查 YAML 格式是否正确
3. 检查文件权限是否可读

### 连接服务器失败

**问题：** 升级时提示无法连接服务器

**检查清单：**
- ✅ IP 地址是否正确
- ✅ 网络是否连通（`ping ${IP}`）
- ✅ 用户名密码是否正确
- ✅ SSH 服务是否启动

### 版本查询失败

**问题：** 版本显示"获取失败"或"超时"

**解决：**
1. 查看日志文件了解详细错误
2. 检查 OTA 脚本路径是否正确
3. 手动执行命令测试：
   ```bash
   /home/linaro/ota/scripts/ota --remote-latest --from-ip ... --from-pass ...
   ```

## 编译和运行

```bash
cd /home/ubuntu/zhaokai/qt_workspace/OTA
qmake OTA.pro
make
./OTA
```

## 优势总结

✅ **配置灵活**：修改配置无需重新编译  
✅ **易于维护**：配置集中管理，清晰明了  
✅ **环境适配**：不同环境可使用不同配置文件  
✅ **快速部署**：修改 IP 和密码即可部署到新环境  
✅ **安全可控**：通过文件权限保护敏感信息

## 注意事项

- 配置文件路径默认为当前工作目录的 `config.yaml`
- 如果配置文件加载失败，程序会使用默认值并弹出警告
- 修改配置文件后需要重启程序才能生效
- 所有路径建议使用绝对路径，避免相对路径问题
