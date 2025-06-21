QT += core widgets network

CONFIG += c++17

# 项目信息
TARGET = Translator
TEMPLATE = app
# VERSION = 1.0.0

# 应用信息
DEFINES += APP_NAME=\\\"Translator\\\"
DEFINES += APP_VERSION=\\\"1.0.0\\\"

# 源文件
SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/translator.cpp

# 头文件
HEADERS += \
    src/mainwindow.h \
    src/translator.h \
    src/config.h

# UI文件
FORMS += \
    ui/mainwindow.ui

# 默认规则使构建目录
DESTDIR = $$PWD/bin

# Windows特定配置
win32 {
    # Windows特定配置
    # RC_ICONS = resources/icon.ico
    # VERSION = 1.0.0.0
    QMAKE_TARGET_COMPANY = "Translator"
    QMAKE_TARGET_PRODUCT = "Translator"
    QMAKE_TARGET_DESCRIPTION = "Qt翻译应用"
    QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2024"
}

# 编译选项 - 根据编译器设置不同的警告选项
msvc {
    # MSVC编译器选项
    QMAKE_CXXFLAGS += -W3
} else {
    # GCC/MinGW编译器选项
    QMAKE_CXXFLAGS += -Wall -Wextra
}

# 调试配置
CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/bin/debug
    DEFINES += DEBUG
} else {
    DESTDIR = $$PWD/bin/release
    DEFINES += NDEBUG
}

# 包含路径
INCLUDEPATH += src

# 输出信息
message(构建目标: $$TARGET)
message(输出目录: $$DESTDIR)
message(Qt版本: $$[QT_VERSION])
message(平台: Windows) 