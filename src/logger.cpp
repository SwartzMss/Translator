#include "logger.h"
#include "config.h"
#include <QStandardPaths>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <QFileInfo>

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
    
    info("日志系统初始化完成", __FILE__, __LINE__);
}

Logger::~Logger()
{
    if (m_logFile.isOpen()) {
        info("日志系统关闭", __FILE__, __LINE__);
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

void Logger::debug(const QString &message,
                   const char *file, int line)
{
    writeLog(LOG_DEBUG, message, file, line);
}

void Logger::info(const QString &message,
                  const char *file, int line)
{
    writeLog(LOG_INFO, message, file, line);
}

void Logger::warning(const QString &message,
                     const char *file, int line)
{
    writeLog(LOG_WARNING, message, file, line);
}

void Logger::error(const QString &message,
                   const char *file, int line)
{
    writeLog(LOG_ERROR, message, file, line);
}

void Logger::critical(const QString &message,
                      const char *file, int line)
{
    writeLog(LOG_CRITICAL, message, file, line);
}


void Logger::writeLog(LogLevel level, const QString &message,
                      const char *file, int line)
{
    if (level < m_currentLevel) return;

    QString formattedMessage = formatMessage(level, message, file, line);
    
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

QString Logger::formatMessage(LogLevel level, const QString &message,
                              const char *file, int line)
{
    QString timestamp = QDateTime::currentDateTime().toString(QString::fromUtf8(Config::LOG_DATE_FORMAT));
    QString levelStr = levelToString(level);
    QString threadId = QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()));
    QString fileName;
    if (file) {
        fileName = QFileInfo(QString::fromUtf8(file)).fileName();
    }

    return QString("[%1] [%2] [%3:%4] [T:%5] %6")
           .arg(timestamp)
           .arg(levelStr)
           .arg(fileName)
           .arg(line)
           .arg(threadId)
           .arg(message);
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