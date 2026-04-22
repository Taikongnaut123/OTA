#include "ConfigManager.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRegularExpression>
#include <QByteArray>

ConfigManager &ConfigManager::instance()
{
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::loadConfig(const QString &configPath)
{
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "无法打开配置文件:" << configPath;
        return false;
    }

    QString content = QTextStream(&file).readAll();
    file.close();

    parseYaml(content);

    qDebug() << "配置加载成功，共" << m_config.size() << "项配置";
    return true;
}

void ConfigManager::parseYaml(const QString &content)
{
    m_config.clear();

    QStringList lines = content.split('\n');
    QString currentSection;

    for (const QString &line : lines)
    {
        QString trimmedLine = line.trimmed();

        // 跳过空行和注释
        if (trimmedLine.isEmpty() || trimmedLine.startsWith('#'))
        {
            continue;
        }

        // 解析键值对
        int colonPos = trimmedLine.indexOf(':');
        if (colonPos > 0)
        {
            QString key = trimmedLine.left(colonPos).trimmed();
            QString value = trimmedLine.mid(colonPos + 1).trimmed();

            // 处理嵌套（简单支持一级嵌套）
            if (value.isEmpty())
            {
                currentSection = key;
            }
            else
            {
                // 去除引号
                value = trimQuotes(value);

                // 如果有缩进，说明是嵌套的
                if (line.startsWith("  ") && !currentSection.isEmpty())
                {
                    m_config[currentSection + "." + key] = value;
                }
                else
                {
                    m_config[key] = value;
                    currentSection.clear();
                }
            }
        }
    }
}

QString ConfigManager::trimQuotes(const QString &str) const
{
    QString result = str.trimmed();

    // 去除单引号或双引号
    if ((result.startsWith('"') && result.endsWith('"')) ||
        (result.startsWith('\'') && result.endsWith('\'')))
    {
        result = result.mid(1, result.length() - 2);
    }

    return result;
}

QString ConfigManager::getString(const QString &key, const QString &defaultValue) const
{
    return m_config.value(key, defaultValue).toString();
}

int ConfigManager::getInt(const QString &key, int defaultValue) const
{
    bool ok;
    int value = m_config.value(key, defaultValue).toInt(&ok);
    return ok ? value : defaultValue;
}

bool ConfigManager::getBool(const QString &key, bool defaultValue) const
{
    QVariant value = m_config.value(key);
    if (value.type() == QVariant::Bool)
    {
        return value.toBool();
    }

    QString strValue = value.toString().toLower();
    if (strValue == "true" || strValue == "yes" || strValue == "1")
    {
        return true;
    }
    else if (strValue == "false" || strValue == "no" || strValue == "0")
    {
        return false;
    }

    return defaultValue;
}

QString ConfigManager::getUpgradeScriptPath() const
{
    return getString("upgrade.script_path", "");
}

QString ConfigManager::getCurrentVersionScriptPath() const
{
    return getString("current_version.script_path", "");
}

QString ConfigManager::getLatestVersionScriptPath() const
{
    return getString("latest_version.script_path", "");
}

int ConfigManager::getUpgradeTimeout() const
{
    return getInt("upgrade.timeout", 3000);
}

QString ConfigManager::getRecoveryScriptPath() const
{
    return getString("recovery.script_path", "");
}

int ConfigManager::getRecoveryTimeout() const
{
    return getInt("recovery.timeout", 3000);
}

QString ConfigManager::getPassword(const QString &key) const
{
    // 从配置文件读取Base64编码的密码
    QString encodedPassword = getString(key, "");

    if (encodedPassword.isEmpty())
    {
        qWarning() << "密码配置项为空:" << key;
        return "";
    }

    // Base64解码
    QByteArray encodedData = encodedPassword.toLatin1();
    QByteArray decodedData = QByteArray::fromBase64(encodedData);
    QString password = QString::fromUtf8(decodedData);

    qDebug() << "成功解码密码:" << key;
    return password;
}
