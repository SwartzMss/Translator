#include <QApplication>
#include <QStyleFactory>
#include "mainwindow.h"
#include "config.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用信息
    app.setApplicationName(Config::APP_NAME_STR);
    app.setApplicationVersion(Config::APP_VERSION_STR);
    app.setOrganizationName("Translator");
    
    // 设置应用样式
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // 创建并显示主窗口
    MainWindow window;
    window.show();
    
    return app.exec();
} 