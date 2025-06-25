#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QTabWidget>
#include <QMessageBox>
#include <QSettings>
#include <QLineEdit>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "translator.h"
#include "deepseekclient.h"

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

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onTranslateClicked();
    void onPolishClicked();
    void onTranslationFinished(const QString &translatedText, const QString &detectedLang);
    void onTranslationError(const QString &errorMessage);
    void onPolishFinished(const QString &polishedText);
    void onPolishError(const QString &errorMessage);
    void onSetApiClicked();
    void onTestApiClicked();
    void onSwapLanguagesClicked();
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowTriggered();
    void onExitTriggered();

private:
    void setupUi();
    void setupConnections();
    void loadSettings();
    void saveSettings();
    void updateLanguageComboBoxes();
    QString getLanguageCode(const QString &languageName);
    QString getLanguageName(const QString &languageCode);
    void showError(const QString &message);
    void showInfo(const QString &message);
    void createTrayIcon();

    // UI组件
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    QTabWidget *m_tabWidget;

    // 翻译Tab组件
    QWidget *m_translateTab;
    QLabel *m_translateInputLabel;
    QTextEdit *m_translateInputEdit;
    QHBoxLayout *m_languageLayout;
    QLabel *m_fromLabel;
    QComboBox *m_fromLanguageCombo;
    QPushButton *m_swapButton;
    QLabel *m_toLabel;
    QComboBox *m_toLanguageCombo;
    QPushButton *m_translateButton;
    QLabel *m_translateOutputLabel;
    QTextEdit *m_translateOutputEdit;

    // 润色Tab组件
    QWidget *m_polishTab;
    QLabel *m_polishInputLabel;
    QTextEdit *m_polishInputEdit;
    QPushButton *m_polishButton;
    QLabel *m_polishOutputLabel;
    QTextEdit *m_polishOutputEdit;
    
    // API配置区域
    QLineEdit *m_appIdEdit;
    QLineEdit *m_secretKeyEdit;
    QLineEdit *m_deepSeekKeyEdit;
    QPushButton *m_setApiButton;
    QPushButton *m_testApiButton;
    
    // 状态栏
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    
    // 翻译服务
    Translator *m_translator;
    DeepSeekClient *m_polisher;
    
    // 设置
    QSettings *m_settings;

    // 系统托盘
    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayMenu;
    QAction *m_showAction;
    QAction *m_exitAction;
};

#endif // MAINWINDOW_H 