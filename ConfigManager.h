#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QMap>
#include <QVariant>

class ConfigManager
{
public:
    static ConfigManager &instance();

    // 加载配置文件
    bool loadConfig(const QString &configPath = "config.yaml");

    // 获取配置值
    QString getString(const QString &key, const QString &defaultValue = "") const;
    int getInt(const QString &key, int defaultValue = 0) const;
    bool getBool(const QString &key, bool defaultValue = false) const;

    // 获取脚本路径（便捷方法）
    QString getUpgradeScriptPath() const;
    QString getRecoveryScriptPath() const;
    QString getCurrentVersionScriptPath() const;
    QString getLatestVersionScriptPath() const;
    int getUpgradeTimeout() const;
    int getRecoveryTimeout() const;

    // 获取解密后的密码
    QString getPassword(const QString &key) const;

private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager &) = delete;
    ConfigManager &operator=(const ConfigManager &) = delete;

    void parseYaml(const QString &content);
    QString trimQuotes(const QString &str) const;

    QMap<QString, QVariant> m_config;
};

#endif // CONFIGMANAGER_H
