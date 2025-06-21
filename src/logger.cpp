#include "logger.h"
#include "config.h"
#include <QStandardPaths>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>

Logger* Logger::m_instance = nullptr;
QMutex Logger::m_mutex;

Logger::Logger(QObject *parent)
    : QObject(parent)
    , m_currentLevel(LOG_INFO)
    , m_logToFile(true)
    , m_logToConsole(true)
    , m_currentLogSize(0)
{
    ensureLogDirectory();
    
    QString appDir = QCoreApplication::applicationDirPath();
    m_logFilePath = appDir + "/" + QString::fromUtf8(Config::LOG_FILE_PATH);
    
    if (m_logToFile) {
        m_logFile.setFileName(m_logFilePath);
        if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            m_logStream.setDevice(&m_logFile);
            m_currentLogSize = m_logFile.size();
        }
    }
    
    info("日志系统初始化完成", "Logger");
}

Logger::~Logger()
{
    if (m_logFile.isOpen()) {
        info("日志系统关闭", "Logger");
        m_logFile.close();
    }
}

Logger* Logger::instance()
{
    if (m_instance == nullptr) {
        QMutexLocker locker(&m_mutex);
        if (m_instance == nullptr) {
            m_instance = new Logger();
        }
    }
    return m_instance;
}

void Logger::debug(const QString &message, const QString &module)
{
    writeLog(LOG_DEBUG, message, module);
}

void Logger::info(const QString &message, const QString &module)
{
    writeLog(LOG_INFO, message, module);
}

void Logger::warning(const QString &message, const QString &module)
{
    writeLog(LOG_WARNING, message, module);
}

void Logger::error(const QString &message, const QString &module)
{
    writeLog(LOG_ERROR, message, module);
}

void Logger::critical(const QString &message, const QString &module)
{
    writeLog(LOG_CRITICAL, message, module);
}

void Logger::logTranslationStart(const QString &text, const QString &fromLang, const QString &toLang)
{
    QString message = QString("开始翻译 - 文本长度: %1, 从: %2, 到: %3")
                     .arg(text.length()).arg(fromLang).arg(toLang);
    info(message, "Translation");
}

void Logger::logTranslationSuccess(const QString &originalText, const QString &translatedText, 
                                  const QString &fromLang, const QString &toLang, const QString &detectedLang)
{
    QString message = QString("翻译成功 - 原文长度: %1, 译文长度: %2, 检测语言: %3")
                     .arg(originalText.length()).arg(translatedText.length()).arg(detectedLang);
    info(message, "Translation");
}

void Logger::logTranslationError(const QString &text, const QString &error, const QString &fromLang, const QString &toLang)
{
    QString message = QString("翻译失败 - 文本长度: %1, 错误: %2")
                     .arg(text.length()).arg(error);
    this->error(message, "Translation");
}

void Logger::logApiCall(const QString &url, const QString &params)
{
    QString message = QString("API调用 - URL: %1, 参数长度: %2")
                     .arg(url).arg(params.length());
    debug(message, "API");
}

void Logger::logApiResponse(const QString &response, bool success)
{
    QString message = QString("API响应 - 成功: %1, 响应长度: %2")
                     .arg(success ? "是" : "否").arg(response.length());
    
    if (success) {
        debug(message, "API");
    } else {
        error(message, "API");
    }
}

void Logger::logUserAction(const QString &action, const QString &details)
{
    QString message = QString("用户操作 - %1: %2").arg(action).arg(details);
    info(message, "UserAction");
}

void Logger::logSettingsChange(const QString &setting, const QString &oldValue, const QString &newValue)
{
    QString message = QString("设置变更 - %1: %2 -> %3")
                     .arg(setting)
                     .arg(oldValue.isEmpty() ? "空" : "***")
                     .arg(newValue.isEmpty() ? "空" : "***");
    info(message, "Settings");
}

void Logger::writeLog(LogLevel level, const QString &message, const QString &module)
{
    if (level < m_currentLevel) return;
    
    QString formattedMessage = formatMessage(level, message, module);
    
    QMutexLocker locker(&m_writeMutex);
    
    if (m_logToFile && m_logFile.isOpen()) {
        m_logStream << formattedMessage << Qt::endl;
        m_logStream.flush();
        m_currentLogSize = m_logFile.size();
    }
    
    if (m_logToConsole) {
        switch (level) {
        case LOG_DEBUG: qDebug().noquote() << formattedMessage; break;
        case LOG_INFO: qInfo().noquote() << formattedMessage; break;
        case LOG_WARNING: qWarning().noquote() << formattedMessage; break;
        case LOG_ERROR:
        case LOG_CRITICAL: qCritical().noquote() << formattedMessage; break;
        }
    }
}

QString Logger::levelToString(LogLevel level)
{
    switch (level) {
    case LOG_DEBUG: return "DEBUG";
    case LOG_INFO: return "INFO";
    case LOG_WARNING: return "WARNING";
    case LOG_ERROR: return "ERROR";
    case LOG_CRITICAL: return "CRITICAL";
    default: return "UNKNOWN";
    }
}

QString Logger::formatMessage(LogLevel level, const QString &message, const QString &module)
{
    QString timestamp = QDateTime::currentDateTime().toString(QString::fromUtf8(Config::LOG_DATE_FORMAT));
    QString levelStr = levelToString(level);
    
    return QString("[%1] [%2] [%3] %4")
           .arg(timestamp).arg(levelStr).arg(module).arg(message);
}

void Logger::ensureLogDirectory()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString logDir = appDir + "/logs";
    
    QDir dir;
    if (!dir.exists(logDir)) {
        dir.mkpath(logDir);
    }
}

void Logger::setLogLevel(LogLevel level) { m_currentLevel = level; }
void Logger::setLogToFile(bool enabled) { m_logToFile = enabled; }
void Logger::setLogToConsole(bool enabled) { m_logToConsole = enabled; }
void Logger::rotateLogFiles() { /* 简化实现 */ }
void Logger::clearLogs() { /* 简化实现 */ } 