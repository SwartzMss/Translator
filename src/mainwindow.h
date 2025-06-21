#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QMessageBox>
#include <QSettings>
#include "translator.h"

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTranslateClicked();
    void onTranslationFinished(const QString &translatedText, const QString &detectedLang);
    void onTranslationError(const QString &errorMessage);
    void onSettingsClicked();
    void onClearClicked();
    void onSwapLanguagesClicked();

private:
    void setupUi();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    void showSettingsDialog();
    void updateLanguageComboBoxes();
    QString getLanguageCode(const QString &languageName);
    QString getLanguageName(const QString &languageCode);
    void showError(const QString &message);
    void showInfo(const QString &message);

    // UI组件
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    
    // 输入区域
    QLabel *m_inputLabel;
    QTextEdit *m_inputTextEdit;
    
    // 语言选择区域
    QHBoxLayout *m_languageLayout;
    QLabel *m_fromLabel;
    QComboBox *m_fromLanguageCombo;
    QPushButton *m_swapButton;
    QLabel *m_toLabel;
    QComboBox *m_toLanguageCombo;
    
    // 翻译按钮
    QPushButton *m_translateButton;
    
    // 输出区域
    QLabel *m_outputLabel;
    QTextEdit *m_outputTextEdit;
    
    // 状态栏
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    
    // 工具栏
    QPushButton *m_clearButton;
    QPushButton *m_settingsButton;
    
    // 翻译服务
    Translator *m_translator;
    
    // 设置
    QSettings *m_settings;
};

#endif // MAINWINDOW_H 