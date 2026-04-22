QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    ConfigManager.cpp

HEADERS += \
    mainwindow.h \
    ConfigManager.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    OTA_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# 将配置文件复制到构建目录
CONFIG(debug, debug|release) {
    DESTDIR = $$OUT_PWD/debug
} else {
    DESTDIR = $$OUT_PWD/release
}

# 复制 config.yaml 到构建目录
copydata.commands = $(COPY_FILE) $$shell_path($$PWD/config.yaml) $$shell_path($$DESTDIR)
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    config.yaml
