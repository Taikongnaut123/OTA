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
        Upgrade,
        Recovery
    };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_upgrade_button_clicked();
    void check_current_version_lable();
    void processOutput();
    void processError();
    void processFinished(int exitCode, QProcess::ExitStatus status);
    void getSelectedItem();
    void on_recovery_button_clicked();

private:
    Ui::MainWindow *ui;
    QProcess *process;
    OperationType currentOperation;

    void updateVersionInfo();
    void showUpgradeDialog();
    void executeScript(OperationType opType);
};
#endif // MAINWINDOW_H
