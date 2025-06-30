#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>
#include <QDir>
#include <QDebug>

enum LogLevel {
    LOG_DEBUG = 0,
    LOG_INFO = 1,
    LOG_WARNING = 2,
    LOG_ERROR = 3,
    LOG_CRITICAL = 4
};

class Logger : public QObject
{
    Q_OBJECT

public:
    static Logger* instance();
    
    // 日志记录方法
    void debug(const QString &message,
               const char *file = nullptr, int line = 0);
    void info(const QString &message,
              const char *file = nullptr, int line = 0);
    void warning(const QString &message,
                 const char *file = nullptr, int line = 0);
    void error(const QString &message,
               const char *file = nullptr, int line = 0);
    void critical(const QString &message,
                  const char *file = nullptr, int line = 0);

    
    // 配置方法
    void setLogLevel(LogLevel level);
    void setLogToFile(bool enabled);
    void setLogToConsole(bool enabled);
    
    // 日志文件管理
    void rotateLogFiles();
    void clearLogs();

private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();
    
    void writeLog(LogLevel level, const QString &message,
                  const char *file, int line);
    QString levelToString(LogLevel level);
    QString formatMessage(LogLevel level, const QString &message,
                          const char *file, int line);
    void ensureLogDirectory();
    
    static Logger* m_instance;
    static QMutex m_mutex;
    
    QFile m_logFile;
    QTextStream m_logStream;
    QMutex m_writeMutex;
    
    LogLevel m_currentLevel;
    bool m_logToFile;
    bool m_logToConsole;
    
    
    QString m_logFilePath;
    qint64 m_currentLogSize;
};

// 便捷宏定义
#define LOG_DEBUG(msg) \
    Logger::instance()->debug(msg, __FILE__, __LINE__)
#define LOG_INFO(msg) \
    Logger::instance()->info(msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) \
    Logger::instance()->warning(msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) \
    Logger::instance()->error(msg, __FILE__, __LINE__)
#define LOG_CRITICAL(msg) \
    Logger::instance()->critical(msg, __FILE__, __LINE__)

#endif // LOGGER_H 