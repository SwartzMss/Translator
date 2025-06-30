QT += core widgets network

CONFIG += c++17

# 项目信息
TARGET = Translator
TEMPLATE = app
# VERSION = 1.0.0

# 应用信息
DEFINES += APP_NAME=\"Translator\"
DEFINES += APP_VERSION=\"1.0.0\"

# 源文件
SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/translator.cpp \
    src/logger.cpp \
    src/deepseekclient.cpp \
    src/tlshttpproxy.cpp \
    src/networkmanager.cpp

# 头文件
HEADERS += \
    src/mainwindow.h \
    src/translator.h \
    src/config.h \
    src/logger.h \
    src/deepseekclient.h \
    src/tlshttpproxy.h \
    src/networkmanager.h

# 资源文件
RESOURCES += \
    resources/resources.qrc

# UI文件
FORMS += \
    ui/mainwindow.ui

# 默认规则使构建目录
DESTDIR = $$PWD/bin/debug

# Windows特定配置
RC_ICONS = resources/icon.ico
# VERSION = 1.0.0.0
QMAKE_TARGET_COMPANY = "Translator"
QMAKE_TARGET_PRODUCT = "Translator"
QMAKE_TARGET_DESCRIPTION = "Qt翻译应用"
QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2024"
DLL_SRC_DIR = $$PWD/depend/libcurl/bin
DLL_DST_DIR = $$DESTDIR

QMAKE_POST_LINK += if not exist "$$DLL_DST_DIR" mkdir "$$DLL_DST_DIR" && \
QMAKE_POST_LINK += for %%f in ("$$DLL_SRC_DIR\\*.dll") do copy /Y "%%f" "$$DLL_DST_DIR\\"

# 编译选项 - 只保留MSVC
QMAKE_CXXFLAGS += -W3

# 始终为debug配置
DEFINES += DEBUG

# 包含路径
INCLUDEPATH += src
# libcurl 头文件和库路径
INCLUDEPATH += $$PWD/depend/libcurl/include
LIBS += $$PWD/depend/libcurl/lib/libcurl.lib

# 输出信息
message(构建目标: $$TARGET)
message(输出目录: $$DESTDIR)
message(Qt版本: $$[QT_VERSION])
message(平台: Windows)

SOURCES -= src/networkproxy.cpp
HEADERS -= src/networkproxy.h 