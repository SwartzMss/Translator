#include <QApplication>
#include <QStyleFactory>
#include "mainwindow.h"
#include "config.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 初始化日志系统
    Logger::instance()->setLogLevel(LOG_INFO);
    Logger::instance()->setLogToFile(true);
    Logger::instance()->setLogToConsole(true);
    
    LOG_INFO("应用程序启动", "Main");
    
    // 设置应用信息
    app.setApplicationName(Config::APP_NAME_STR);
    app.setApplicationVersion(Config::APP_VERSION_STR);
    app.setOrganizationName("Translator");
    
    // 设置应用样式
    app.setStyle(QStyleFactory::create("Fusion"));
    
    LOG_INFO("创建主窗口", "Main");
    
    // 创建并显示主窗口
    MainWindow window;
    window.show();
    
    LOG_INFO("主窗口显示完成，进入事件循环", "Main");
    
    int result = app.exec();
    
    LOG_INFO("应用程序退出", "Main");
    
    return result;
} 