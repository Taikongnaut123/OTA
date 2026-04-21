#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ConfigManager.h"
#include <QMessageBox>
#include <QTimer>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), process(nullptr), currentOperation(Upgrade)
{
    ui->setupUi(this);

    // 加载配置文件
    ConfigManager &config = ConfigManager::instance();
    if (!config.loadConfig("config.yaml"))
    {
        QMessageBox::warning(this, "配置加载警告",
                             "无法加载配置文件 config.yaml，将使用默认配置。");
    }

    // ========== 初始化列表 ==========
    // 1. 设置单选模式（核心）
    ui->versionListWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    // 2. 设置点击时选中整行（好看）
    ui->versionListWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 3. 模拟你获取到的列表数据（可以从数据库/配置/网络来）
    QStringList deviceList = {
        "设备1：正常",
        "设备2：异常",
        "设备3：离线",
        "设备4：运行中"
    };

    // 4. 显示到界面
    ui->versionListWidget->addItems(deviceList);
    // 选中项改变时，自动触发槽函数
    connect(ui->versionListWidget, &QListWidget::itemSelectionChanged,
            this, &MainWindow::getSelectedItem);

}

MainWindow::~MainWindow()
{
    if (process)
    {
        process->kill();
        process->deleteLater();
    }
    delete ui;
}

void MainWindow::getSelectedItem()
{
    // 获取当前选中的项
    QListWidgetItem *item = ui->versionListWidget->currentItem();

    if(item)
    {
        // 拿到值！
        QString value = item->text();

        // 你可以在这里使用这个 value
        qDebug() << "当前选中：" << value;
    }
}


void MainWindow::on_upgrade_button_clicked()
{
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
        return;
    }
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

void MainWindow::check_current_version_lable()
{
    // 从配置文件获取版本号
    ConfigManager &config = ConfigManager::instance();

    ui->current_version_label_value->setText(config.getCurrentVersion());
    ui->latest_version_label_value->setText(config.getLatestVersion());
}

void MainWindow::processOutput()
{
    QString output = process->readAllStandardOutput();
    qDebug() << "输出:" << output;

    // 在输入框显示最新输出
    ui->inputLineEdit->setText(output.trimmed());
}

void MainWindow::processError()
{
    QString error = process->readAllStandardError();
    qDebug() << "错误:" << error;

    ui->inputLineEdit->setText("错误: " + error.trimmed());
}

void MainWindow::processFinished(int exitCode, QProcess::ExitStatus status)
{
    ui->upgrade_button->setEnabled(true);
    ui->recovery_button->setEnabled(true);

    QString operationName = (currentOperation == Upgrade) ? "升级" : "恢复";
    QString buttonText = (currentOperation == Upgrade) ? "升级" : "恢复";

    QPushButton *currentButton = (currentOperation == Upgrade) ? ui->upgrade_button : ui->recovery_button;
    currentButton->setText(buttonText);

    ui->inputLineEdit->setReadOnly(false);

    if (exitCode == 0 && status == QProcess::NormalExit)
    {
        // 操作成功
        ui->inputLineEdit->setText("✓ " + operationName + "成功！");
        statusBar()->showMessage(operationName + "成功", 5000);

        QMessageBox::information(this, operationName + "成功",
                                 QString("S100 已成功%1到最新版本！\n\n新版本将在下次启动时生效。").arg(operationName));

        // 刷新版本信息
        QTimer::singleShot(1000, this, &MainWindow::check_current_version_lable);
    }
    else
    {
        // 操作失败
        ui->inputLineEdit->setText("✗ " + operationName + "失败！");
        statusBar()->showMessage(operationName + "失败", 5000);

        QString errorOutput = process->readAllStandardError();

        QMessageBox::critical(this, operationName + "失败",
                              QString("%1失败！\n\n错误码: %2\n\n详细信息:\n%3")
                                  .arg(operationName)
                                  .arg(exitCode)
                                  .arg(errorOutput.isEmpty() ? "无详细错误信息" : errorOutput));
    }

    process->deleteLater();
    process = nullptr;
}

void MainWindow::updateVersionInfo()
{
    // 从配置文件获取版本号
    ConfigManager &config = ConfigManager::instance();

    ui->current_version_label_value->setText(config.getCurrentVersion());
    ui->latest_version_label_value->setText(config.getLatestVersion());
}

void MainWindow::showUpgradeDialog()
{
    executeScript(Upgrade);
}

void MainWindow::executeScript(OperationType opType)
{
    currentOperation = opType;

    QString operationName = (opType == Upgrade) ? "升级" : "恢复";
    QString buttonText = (opType == Upgrade) ? "升级" : "恢复";

    QPushButton *currentButton = (opType == Upgrade) ? ui->upgrade_button : ui->recovery_button;

    ui->upgrade_button->setEnabled(false);
    ui->recovery_button->setEnabled(false);
    currentButton->setText(operationName + "中...");

    ui->inputLineEdit->setReadOnly(true);
    ui->inputLineEdit->clear();
    ui->inputLineEdit->setText("正在执行" + operationName + "脚本...");

    process = new QProcess();

    // 连接信号
    connect(process, &QProcess::readyReadStandardOutput,
            this, &MainWindow::processOutput);
    connect(process, &QProcess::readyReadStandardError,
            this, &MainWindow::processError);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MainWindow::processFinished);

    // 从配置文件读取脚本路径
    ConfigManager &config = ConfigManager::instance();
    QString scriptPath;
    int timeout;

    if (opType == Upgrade)
    {
        scriptPath = config.getScriptPath();
        timeout = config.getTimeout();
    }
    else
    {
        scriptPath = config.getRecoveryScriptPath();
        timeout = config.getRecoveryTimeout();
    }

    // 使用 shell 执行命令，支持带参数的完整命令字符串
    process->start("/bin/bash", QStringList() << "-c" << scriptPath);

    if (!process->waitForStarted(timeout))
    {
        QMessageBox::critical(this, "启动失败",
                              "无法启动" + operationName + "脚本！\n\n请检查命令:\n" + scriptPath);

        ui->upgrade_button->setEnabled(true);
        ui->recovery_button->setEnabled(true);
        currentButton->setText(buttonText);
        ui->inputLineEdit->setReadOnly(false);
        return;
    }

    statusBar()->showMessage("正在" + operationName + "...", 0);
}
