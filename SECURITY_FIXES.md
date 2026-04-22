# OTA 项目安全问题和修复建议

## 🔴 严重安全漏洞

### 1. 硬编码密码（CRITICAL）

**位置：** `mainwindow.cpp` 行 8-15

**问题：**
```cpp
#define QUERY_LATEST_VERSION_CMD "... --from-pass '123456'"
#define UPGRADE_CMD "... --from-pass 123456 ... --to-pass root ..."
```

**风险等级：** 🔴 CRITICAL
- 密码明文存储在源代码中
- 可通过 `strings` 命令从二进制提取
- Git 历史永久保存

**修复方案：**

#### 方案 1：环境变量（推荐）
```cpp
QString fromPass = qgetenv("OTA_FROM_PASSWORD");
QString toPass = qgetenv("OTA_TO_PASSWORD");

if (fromPass.isEmpty() || toPass.isEmpty()) {
    QMessageBox::critical(this, "错误", "未设置密码环境变量！");
    return;
}

QString cmd = QString("/home/linaro/ota/scripts/ota ... --from-pass '%1' --to-pass '%2'")
                .arg(fromPass).arg(toPass);
```

#### 方案 2：加密配置文件
```cpp
// 使用 Qt 的 QSettings 加密存储
QSettings settings("config.ini", QSettings::IniFormat);
settings.setValue("credentials/from_pass", encryptPassword("123456"));
```

#### 方案 3：用户输入
```cpp
// 首次运行时要求用户输入密码
QInputDialog::getText(this, "密码", "请输入升级密码:", QLineEdit::Password);
```

---

### 2. 命令注入风险（HIGH）

**位置：** `mainwindow.cpp` 行 266

**问题：**
```cpp
process->start("/bin/bash", QStringList() << "-c" << scriptPath);
```

**风险等级：** 🔴 HIGH
- config.yaml 被篡改可执行任意命令
- 例如：`script_path: "rm -rf / && echo pwned"`

**修复方案：**

#### 方案 1：命令白名单验证
```cpp
bool isCommandSafe(const QString &cmd) {
    QStringList allowedPrefixes = {
        "/home/linaro/ota/scripts/ota",
        "/opt/ota/scripts/ota"
    };
    
    for (const QString &prefix : allowedPrefixes) {
        if (cmd.trimmed().startsWith(prefix)) {
            return true;
        }
    }
    return false;
}

// 使用前验证
if (!isCommandSafe(scriptPath)) {
    QMessageBox::critical(this, "安全错误", 
        "检测到非法命令！\n\n只允许执行官方 OTA 脚本。");
    return;
}
```

#### 方案 2：参数化执行（最安全）
```cpp
// 不使用 shell，直接传递参数
QStringList parseCommand(const QString &cmd) {
    // 简单的参数解析
    return cmd.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
}

QStringList args = parseCommand(scriptPath);
if (!args.isEmpty()) {
    QString program = args.takeFirst();
    process->start(program, args);  // 不经过 shell
}
```

#### 方案 3：配置文件签名验证
```cpp
bool verifyConfigSignature(const QString &configPath) {
    // 使用 SHA256 或 GPG 签名验证配置文件完整性
    QFile file(configPath);
    QByteArray data = file.readAll();
    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);
    
    // 与已知的合法配置哈希比对
    return hash == expectedHash;
}
```

---

## ⚠️ 并发和资源管理问题

### 3. QProcess 竞态条件（MEDIUM）

**位置：** `mainwindow.cpp` - `process` 成员变量

**问题：**
- 只有一个 `process` 对象被多处使用
- `updateVersionInfo()` 同步阻塞 + `executeScript()` 异步
- 没有互斥锁或状态检查

**修复方案：**

```cpp
// mainwindow.h
private:
    QProcess *process;
    QProcess *versionQueryProcess;  // 专门用于版本查询
    bool isProcessBusy;             // 状态标志
    QMutex processMutex;            // 互斥锁

// mainwindow.cpp
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), 
      process(nullptr),
      versionQueryProcess(nullptr),
      isProcessBusy(false)
{
    process = new QProcess(this);
    versionQueryProcess = new QProcess(this);  // 独立的进程对象
    
    // 版本查询使用独立的进程和信号
    connect(versionQueryProcess, &QProcess::finished,
            this, &MainWindow::onVersionQueryFinished);
}

void MainWindow::executeScript(OperationType opType)
{
    // 检查是否正在执行
    if (isProcessBusy) {
        QMessageBox::warning(this, "操作进行中", 
            "当前有操作正在执行，请等待完成后再试！");
        return;
    }
    
    QMutexLocker locker(&processMutex);
    isProcessBusy = true;
    
    // ... 执行脚本
}

void MainWindow::processFinished(int exitCode, QProcess::ExitStatus status)
{
    QMutexLocker locker(&processMutex);
    isProcessBusy = false;
    
    // ... 原有逻辑
}
```

---

### 4. 内存泄漏风险（LOW）

**问题：**
```cpp
process = new QProcess();  // 没有设置 parent
```

**修复：**
```cpp
process = new QProcess(this);  // 设置 parent，自动管理内存
```

---

## 🐛 逻辑问题

### 5. processFinished 处理所有操作（MEDIUM）

**问题：**
```cpp
void MainWindow::processFinished(int exitCode, QProcess::ExitStatus status)
{
    // 版本查询完成也会触发成功/失败消息框
    if (exitCode == 0) {
        QMessageBox::information(...);  // ❌ 查询版本不应该弹窗
    }
}
```

**修复：**
```cpp
void MainWindow::processFinished(int exitCode, QProcess::ExitStatus status)
{
    // 只处理升级/恢复操作
    if (currentOperation == QueryVersion) {
        // 版本查询完成，静默处理
        return;
    }
    
    // 只有升级/恢复才显示消息框
    if (currentOperation == Upgrade || currentOperation == Recovery) {
        if (exitCode == 0) {
            QMessageBox::information(...);
        } else {
            QMessageBox::critical(...);
        }
    }
}
```

---

### 6. 超时参数使用错误（MEDIUM）

**问题：**
```cpp
if (!process->waitForStarted(timeout))  
// timeout 是脚本执行超时（3000ms），不是启动超时
```

**修复：**
```cpp
const int START_TIMEOUT = 5000;  // 启动超时 5 秒
const int EXEC_TIMEOUT = timeout; // 执行超时从配置读取

if (!process->waitForStarted(START_TIMEOUT)) {
    QMessageBox::critical(...);
    return;
}

// 等待执行完成（使用正确的超时）
if (!process->waitForFinished(EXEC_TIMEOUT)) {
    process->kill();
    QMessageBox::critical(this, "超时", "脚本执行超时！");
}
```

---

### 7. 拼写错误（LOW）

```cpp
void check_current_version_lable()  // ❌ lable
```

**修复：**
```cpp
void checkCurrentVersionLabel()  // ✅ 正确拼写 + 驼峰命名
```

---

## 📋 缺失功能

### 8. 日志功能未实现

**config.yaml 中配置了：**
```yaml
logging:
  enabled: true
  log_path: "/var/log/ota_upgrade.log"
  log_level: "info"
```

**但代码中未实现**

**建议实现：**
```cpp
void MainWindow::log(const QString &message, const QString &level)
{
    ConfigManager &config = ConfigManager::instance();
    if (!config.getBool("logging.enabled", false)) {
        return;
    }
    
    QString logPath = config.getString("logging.log_path", "/tmp/ota.log");
    QFile logFile(logPath);
    
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") 
            << " [" << level << "] " << message << "\n";
    }
}
```

---

## 🔒 配置文件安全建议

### 9. 配置文件权限

**当前问题：**
- config.yaml 可能被任意用户读取/修改
- 包含敏感信息（脚本路径、IP、用户名）

**建议：**
```bash
# 限制配置文件权限
chmod 600 config.yaml
chown root:root config.yaml

# 代码中检查权限
QFileInfo fileInfo("config.yaml");
QFile::Permissions perms = fileInfo.permissions();
if (perms & QFile::ReadOther || perms & QFile::WriteOther) {
    QMessageBox::warning(this, "安全警告",
        "配置文件权限过于宽松！建议设置为 600");
}
```

---

## 📊 优先级修复顺序

1. 🔴 **立即修复**
   - 移除硬编码密码
   - 添加命令白名单验证

2. ⚠️ **高优先级**
   - 修复 QProcess 竞态条件
   - 修正超时参数使用

3. ℹ️ **中优先级**
   - 实现日志功能
   - 修复 processFinished 逻辑

4. ✅ **低优先级**
   - 修正拼写错误
   - 添加配置文件权限检查

---

## 🛡️ 安全检查清单

- [ ] 移除所有硬编码密码和敏感信息
- [ ] 实现命令白名单或参数化执行
- [ ] 添加配置文件签名验证
- [ ] 修复 QProcess 并发问题
- [ ] 实现完整的日志记录
- [ ] 添加异常捕获和错误处理
- [ ] 配置文件权限检查
- [ ] 代码安全审计
- [ ] 添加单元测试

