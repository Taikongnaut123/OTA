#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include "ConfigManager.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum OperationType
    {
        Upgrade = 0,
        Recovery = 1,
        QueryLatestVersion = 2,
        QueryCurrentVersion = 3,
        QueryVersion = 5
    };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_upgrade_button_clicked();
//    void check_current_version_lable();
    void processOutput();
    void processError();
    void processFinished(int exitCode, QProcess::ExitStatus status);
    void on_recovery_button_clicked();

private:
    Ui::MainWindow *ui;
    QProcess *process;
    OperationType currentOperation;
    QMap<OperationType, QString> operations{
        {Upgrade, "升级"},
        {Recovery, "恢复"},
        {QueryVersion, "获取版本号"}};
    QMap<OperationType, QString> scripts_;
    void updateVersionInfo();
    //    void showUpgradeDialog();
    void executeScript(OperationType opType);
    QString replacePasswordPlaceholders(const QString &cmd) const;
};
#endif // MAINWINDOW_H
