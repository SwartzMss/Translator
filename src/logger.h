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
    void debug(const QString &message, const QString &module = "General");
    void info(const QString &message, const QString &module = "General");
    void warning(const QString &message, const QString &module = "General");
    void error(const QString &message, const QString &module = "General");
    void critical(const QString &message, const QString &module = "General");
    
    // 业务流程日志
    void logTranslationStart(const QString &text, const QString &fromLang, const QString &toLang);
    void logTranslationSuccess(const QString &originalText, const QString &translatedText, 
                              const QString &fromLang, const QString &toLang, const QString &detectedLang);
    void logTranslationError(const QString &text, const QString &error, const QString &fromLang, const QString &toLang);
    void logApiCall(const QString &url, const QString &params);
    void logApiResponse(const QString &response, bool success);
    void logUserAction(const QString &action, const QString &details = "");
    void logSettingsChange(const QString &setting, const QString &oldValue, const QString &newValue);
    
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
    
    void writeLog(LogLevel level, const QString &message, const QString &module);
    QString levelToString(LogLevel level);
    QString formatMessage(LogLevel level, const QString &message, const QString &module);
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
#define LOG_DEBUG(msg, module) Logger::instance()->debug(msg, module)
#define LOG_INFO(msg, module) Logger::instance()->info(msg, module)
#define LOG_WARNING(msg, module) Logger::instance()->warning(msg, module)
#define LOG_ERROR(msg, module) Logger::instance()->error(msg, module)
#define LOG_CRITICAL(msg, module) Logger::instance()->critical(msg, module)

#define LOG_TRANSLATION_START(text, from, to) Logger::instance()->logTranslationStart(text, from, to)
#define LOG_TRANSLATION_SUCCESS(orig, trans, from, to, detected) Logger::instance()->logTranslationSuccess(orig, trans, from, to, detected)
#define LOG_TRANSLATION_ERROR(text, error, from, to) Logger::instance()->logTranslationError(text, error, from, to)
#define LOG_API_CALL(url, params) Logger::instance()->logApiCall(url, params)
#define LOG_API_RESPONSE(response, success) Logger::instance()->logApiResponse(response, success)
#define LOG_USER_ACTION(action, details) Logger::instance()->logUserAction(action, details)
#define LOG_SETTINGS_CHANGE(setting, oldVal, newVal) Logger::instance()->logSettingsChange(setting, oldVal, newVal)

#endif // LOGGER_H 