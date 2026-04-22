#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ConfigManager.h"
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QStandardPaths>

// 命令模板（使用占位符 ${FROM_PASS} 和 ${TO_PASS}）
// 实际密码从加密配置文件读取
#define QUERY_LATEST_VERSION_CMD "/home/linaro/ota/scripts/ota --remote-latest --from-ip 192.168.20.204 --from-path /home/konka-admin/workspace --package-name tangpa --from-user konka-admin  --from-pass '${FROM_PASS}'"

#define QUERY_CURRENT_VERSION_CMD "/home/linaro/ota/scripts/ota  --current-version --to-ip 192.168.127.10    --to-user root --to-pass '${TO_PASS}'"

#define UPGRADE_CMD "/home/linaro/ota/scripts/ota --from-ip 192.168.20.204 --from-path /home/konka-admin/workspace  --from-user konka-admin --from-pass ${FROM_PASS} --to-ip 192.168.127.10 " \
                    "--to-user root  --to-pass ${TO_PASS} --package-name tangpa --package-version ${LATEST_VERSION} --force"

#define RECOVERY_CMD "/home/linaro/ota/scripts/ota --restore  --to-ip 192.168.127.10 --to-user root  --to-pass ${TO_PASS}"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), process(nullptr), currentOperation(Upgrade), latestVersion_("")
{
    ui->setupUi(this);

    // 初始化日志文件
    QString logPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/ota_upgrade.log";
    logFile.setFileName(logPath);
    if (logFile.open(QIODevice::Append | QIODevice::Text))
    {
        log("==================== OTA 应用启动 ====================");
        log("日志文件路径: " + logPath);
    }
    else
    {
        qWarning() << "无法打开日志文件:" << logPath;
    }

    // 加载配置文件
    ConfigManager &config = ConfigManager::instance();
    if (!config.loadConfig("config.yaml"))
    {
        QMessageBox::warning(this, "配置加载警告",
                             "无法加载配置文件 config.yaml，将使用默认配置。");
    }
    else
    {

        scripts_.insert(Upgrade, config.getUpgradeScriptPath());
        scripts_.insert(Recovery, config.getRecoveryScriptPath());
        scripts_.insert(QueryLatestVersion, config.getLatestVersionScriptPath());
        scripts_.insert(QueryCurrentVersion, config.getCurrentVersionScriptPath());
    }
    process = new QProcess();

    // 连接信号
    connect(process, &QProcess::readyReadStandardOutput,
            this, &MainWindow::processOutput);
    connect(process, &QProcess::readyReadStandardError,
            this, &MainWindow::processError);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::processFinished);

    // 延迟执行版本信息更新，避免阻塞构造函数
    // 窗口显示后 500ms 开始获取版本信息
    QTimer::singleShot(500, this, &MainWindow::updateVersionInfo);
}

MainWindow::~MainWindow()
{
    log("OTA 应用退出");
    if (logFile.isOpen())
    {
        logFile.close();
    }

    if (process)
    {
        process->kill();
        process->deleteLater();
    }
    delete ui;
}

void MainWindow::on_upgrade_button_clicked()
{
    log("用户点击升级按钮");
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认升级",
        "确定要升级 S100 控制器吗？\n\n"
        "⚠ 升级过程中机器人将停止运动\n"
        "⏱ 预计耗时: 3-5 分钟\n"
        "⚡ 请确保电量充足\n\n"
        "升级期间请勿断电或关闭程序！",
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes)
    {
        log("用户取消升级");
        return;
    }
    log("开始执行升级");
    executeScript(Upgrade);
}

void MainWindow::on_recovery_button_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "确认恢复",
        "确定要恢复 S100 控制器吗？\n\n"
        "⚠ 恢复过程中机器人将停止运动\n"
        "⏱ 预计耗时: 3-5 分钟\n"
        "⚡ 请确保电量充足\n\n"
        "恢复期间请勿断电或关闭程序！",
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes)
    {
        return;
    }
    executeScript(Recovery);
}

void MainWindow::processOutput()
{
    // 版本查询操作不处理输出（在 updateVersionInfo 中同步读取）
    if (currentOperation == QueryVersion ||
        currentOperation == QueryLatestVersion ||
        currentOperation == QueryCurrentVersion)
    {
        return;
    }

    QString output = process->readAllStandardOutput();
    qDebug() << "输出:" << output;

    // 在输入框显示最新输出
    ui->inputLineEdit->setText(output.trimmed());
}

void MainWindow::processError()
{
    // 版本查询操作不处理错误输出（在 updateVersionInfo 中同步读取）
    if (currentOperation == QueryVersion ||
        currentOperation == QueryLatestVersion ||
        currentOperation == QueryCurrentVersion)
    {
        return;
    }

    QString error = process->readAllStandardError();
    qDebug() << "错误:" << error;

    ui->inputLineEdit->setText("错误: " + error.trimmed());
}

void MainWindow::processFinished(int exitCode, QProcess::ExitStatus status)
{
    // 版本查询操作不需要显示消息框，直接返回
    if (currentOperation == QueryVersion ||
        currentOperation == QueryLatestVersion ||
        currentOperation == QueryCurrentVersion)
    {
        return;
    }

    ui->upgrade_button->setEnabled(true);
    ui->recovery_button->setEnabled(true);

    QString operationName = operations.find(currentOperation).value();

    ui->inputLineEdit->setReadOnly(false);

    if (exitCode == 0 && status == QProcess::NormalExit)
    {
        // 操作成功
        log(operationName + "执行成功！退出码: " + QString::number(exitCode));
        ui->inputLineEdit->setText("✓ " + operationName + "成功！");
        statusBar()->showMessage(operationName + "成功", 5000);

        QMessageBox::information(this, operationName + "成功",
                                 QString("S100 已成功%1到最新版本！\n\n正在刷新版本信息...").arg(operationName));

        log("3秒后将自动刷新版本信息");
        // 刷新版本信息（延迟3秒，等待S100完成版本切换）
        QTimer::singleShot(3000, this, &MainWindow::updateVersionInfo);
    }
    else
    {
        // 操作失败
        QString errorOutput = process->readAllStandardError();
        log(operationName + "执行失败！退出码: " + QString::number(exitCode) + " 错误: " + errorOutput);

        ui->inputLineEdit->setText("✗ " + operationName + "失败！");
        statusBar()->showMessage(operationName + "失败", 5000);

        QMessageBox::critical(this, operationName + "失败",
                              QString("%1失败！\n\n错误码: %2\n\n详细信息:\n%3")
                                  .arg(operationName)
                                  .arg(exitCode)
                                  .arg(errorOutput.isEmpty() ? "无详细错误信息" : errorOutput));
    }
}

void MainWindow::updateVersionInfo()
{
    log("========== 开始查询版本信息 ==========");
    currentOperation = QueryVersion;
    // 设置超时时间（毫秒）
    const int timeout = 5000;

    // 获取云端最新的版本号
    QString latestCmd = replacePasswordPlaceholders(QUERY_LATEST_VERSION_CMD);
    log("查询最新版本命令: " + latestCmd);
    process->start("/bin/bash", QStringList() << "-c" << latestCmd);

    QString latest_version_value = "获取中...";
    if (process->waitForFinished(timeout))
    {
        if (process->exitCode() == 0)
        {
            latest_version_value = process->readAllStandardOutput().trimmed();
            latestVersion_ = latest_version_value; // 保存最新版本号
            log("✓ 最新版本查询成功: " + latest_version_value + " (已保存用于升级)");
        }
        else
        {
            latest_version_value = "获取失败";
            QString error = process->readAllStandardError();
            log("✗ 获取最新版本失败，退出码: " + QString::number(process->exitCode()) + " 错误: " + error);
        }
    }
    else
    {
        process->kill();
        latest_version_value = "超时";
        log("✗ 获取最新版本超时（5秒）");
    }

    // 获取本机当前运行的版本号
    QString currentCmd = replacePasswordPlaceholders(QUERY_CURRENT_VERSION_CMD);
    log("查询当前版本命令: " + currentCmd);
    process->start("/bin/bash", QStringList() << "-c" << currentCmd);

    QString current_version_value = "获取中...";
    if (process->waitForFinished(timeout))
    {
        if (process->exitCode() == 0)
        {
            current_version_value = process->readAllStandardOutput().trimmed();
            log("✓ 当前版本查询成功: " + current_version_value);
        }
        else
        {
            current_version_value = "获取失败";
            QString error = process->readAllStandardError();
            log("✗ 获取当前版本失败，退出码: " + QString::number(process->exitCode()) + " 错误: " + error);
        }
    }
    else
    {
        process->kill();
        current_version_value = "超时";
        log("✗ 获取当前版本超时（5秒）");
    }

    ui->current_version_label_value->setText(current_version_value);
    ui->latest_version_label_value->setText(latest_version_value);

    log("版本信息已更新 - 当前版本: " + current_version_value + " | 最新版本: " + latest_version_value);
    log("========== 版本查询完成 ==========\n");
}

void MainWindow::executeScript(OperationType opType)
{
    currentOperation = opType;

    QString operationName = operations.find(currentOperation).value();

    log("========== 开始执行" + operationName + "操作 ==========");

    // 如果是升级操作，检查是否已获取最新版本号
    if (opType == Upgrade && latestVersion_.isEmpty())
    {
        log("✗ 错误：尚未获取到最新版本号，无法执行升级");
        QMessageBox::critical(this, "升级失败",
                              "无法获取最新版本号！\n\n请等待版本信息加载完成后再尝试升级。");
        return;
    }

    if (opType == Upgrade)
    {
        log("将升级到版本: " + latestVersion_);
    }

    QPushButton *currentButton = (opType == Upgrade) ? ui->upgrade_button : ui->recovery_button;

    ui->upgrade_button->setEnabled(false);
    ui->recovery_button->setEnabled(false);
    currentButton->setText(operationName + "中...");

    ui->inputLineEdit->setReadOnly(true);
    ui->inputLineEdit->clear();
    ui->inputLineEdit->setText("正在执行" + operationName + "脚本...");

    // 从配置文件读取脚本路径
    ConfigManager &config = ConfigManager::instance();
    QString scriptPath;
    int timeout = 3000;
    // 如果配置文件中的脚本命令为空，就用宏定义的命令
    if (scripts_.find(opType).value().isEmpty())
    {
        log("配置文件中脚本命令为空，使用宏定义的默认命令");
        if (opType == Upgrade)
        {
            // 升级脚本命令
            scriptPath = UPGRADE_CMD;
        }
        else
        {
            // 恢复脚本命令
            scriptPath = RECOVERY_CMD;
        }
    }
    else
    { // 如果配置文件中的脚本命令不为空，就用配置文件中的命令
        log("使用配置文件中的脚本命令");
        scriptPath = scripts_.find(opType).value();

        if (opType == Upgrade)
        {
            // 升级脚本执行超时时间
            timeout = config.getUpgradeTimeout();
        }
        else
        {
            // 恢复脚本执行超时时间
            timeout = config.getRecoveryTimeout();
        }
    }

    // 替换密码占位符
    scriptPath = replacePasswordPlaceholders(scriptPath);

    log(operationName + "命令: " + scriptPath);
    log("启动超时设置: " + QString::number(timeout) + " 毫秒");

    // 使用 shell 执行命令，支持带参数的完整命令字符串
    process->start("/bin/bash", QStringList() << "-c" << scriptPath);

    if (!process->waitForStarted(timeout))
    {
        log("✗ " + operationName + "脚本启动失败！超时: " + QString::number(timeout) + "ms");

        QMessageBox::critical(this, "启动失败",
                              "无法启动" + operationName + "脚本！\n\n请检查命令:\n" + scriptPath);

        ui->upgrade_button->setEnabled(true);
        ui->recovery_button->setEnabled(true);
        ui->inputLineEdit->setReadOnly(false);
        return;
    }

    log("✓ " + operationName + "脚本已启动，等待执行完成...");
    currentButton->setText(operationName);
    statusBar()->showMessage("正在" + operationName + "...", 0);
}

QString MainWindow::replacePasswordPlaceholders(const QString &cmd) const
{
    ConfigManager &config = ConfigManager::instance();

    // 从配置文件读取解密后的密码
    QString fromPass = config.getPassword("credentials.from_pass");
    QString toPass = config.getPassword("credentials.to_pass");

    // 替换占位符
    QString result = cmd;
    result.replace("${FROM_PASS}", fromPass);
    result.replace("${TO_PASS}", toPass);
    result.replace("${LATEST_VERSION}", latestVersion_); // 替换最新版本号

    return result;
}

void MainWindow::log(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString logMessage = QString("[%1] %2\n").arg(timestamp).arg(message);

    // 输出到终端
    qDebug().noquote() << logMessage.trimmed();

    // 写入日志文件
    if (logFile.isOpen())
    {
        QTextStream stream(&logFile);
        stream << logMessage;
        stream.flush();
    }
}
