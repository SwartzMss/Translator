#include <QApplication>
#include <QStyleFactory>
#include <QLocalServer>
#include <QLocalSocket>
#include "mainwindow.h"
#include "config.h"
#include "logger.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 单实例检查
    const QString serverName = "TranslatorSingleInstance";
    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (socket.waitForConnected(100)) {
        // 已有实例在运行，通知其激活并退出
        socket.write("activate");
        socket.flush();
        socket.waitForBytesWritten(100);
        socket.disconnectFromServer();
        return 0;
    }

    QLocalServer server;
    QLocalServer::removeServer(serverName);
    server.listen(serverName);
    
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

    QObject::connect(&server, &QLocalServer::newConnection, [&window, &server]() {
        QLocalSocket *client = server.nextPendingConnection();
        if (client) {
            client->readAll();
            client->close();
            client->deleteLater();
        }
        window.show();
        window.raise();
        window.activateWindow();
    });

    window.show();
    
    LOG_INFO("主窗口显示完成，进入事件循环", "Main");
    
    int result = app.exec();
    
    LOG_INFO("应用程序退出", "Main");
    
    return result;
} 