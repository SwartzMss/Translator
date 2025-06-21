#include "mainwindow.h"
#include "config.h"
#include "logger.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QApplication>
#include <QScreen>
#include <QInputDialog>
#include <QLineEdit>
#include <QStyle>
#include <QFont>
#include <QFontMetrics>
#include <QMessageBox>
#include <QSettings>
#include <QCoreApplication>
#include <QGuiApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_translator(new Translator(this))
    , m_settings(nullptr)
{
    LOG_DEBUG("主窗口构造函数开始", "MainWindow");
    
    // 设置配置文件路径到exe同级目录
    QString exePath = QCoreApplication::applicationDirPath();
    QString configPath = exePath + "/translator.ini";
    m_settings = new QSettings(configPath, QSettings::IniFormat, this);
    
    LOG_INFO(QString("配置文件路径: %1").arg(configPath), "MainWindow");
    
    setupUi();
    setupConnections();
    loadSettings();
    updateLanguageComboBoxes();
    
    // 设置窗口属性
    setWindowTitle("百度翻译工具");
    setMinimumSize(600, 500);
    
    // 居中显示窗口
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    LOG_DEBUG("主窗口构造函数完成", "MainWindow");
    LOG_USER_ACTION("应用启动", "主窗口创建完成");
}

MainWindow::~MainWindow()
{
    LOG_INFO("主窗口销毁", "MainWindow");
    saveSettings();
}

void MainWindow::setupUi()
{
    LOG_DEBUG("开始设置UI", "MainWindow");
    
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // API配置区域 - 直接显示在主窗口
    QGroupBox *apiGroup = new QGroupBox("百度翻译API配置", this);
    QVBoxLayout *apiLayout = new QVBoxLayout(apiGroup);
    apiLayout->setSpacing(10);
    
    // App ID
    QHBoxLayout *appIdLayout = new QHBoxLayout();
    QLabel *appIdLabel = new QLabel("App ID:", this);
    appIdLabel->setMinimumWidth(80);
    m_appIdEdit = new QLineEdit(this);
    m_appIdEdit->setPlaceholderText("请输入百度翻译API的App ID");
    appIdLayout->addWidget(appIdLabel);
    appIdLayout->addWidget(m_appIdEdit);
    
    // Secret Key
    QHBoxLayout *secretKeyLayout = new QHBoxLayout();
    QLabel *secretKeyLabel = new QLabel("Secret Key:", this);
    secretKeyLabel->setMinimumWidth(80);
    m_secretKeyEdit = new QLineEdit(this);
    m_secretKeyEdit->setPlaceholderText("请输入百度翻译API的Secret Key");
    m_secretKeyEdit->setEchoMode(QLineEdit::Password);
    secretKeyLayout->addWidget(secretKeyLabel);
    secretKeyLayout->addWidget(m_secretKeyEdit);
    
    // API配置按钮
    QHBoxLayout *apiButtonLayout = new QHBoxLayout();
    m_setApiButton = new QPushButton("设置并保存", this);
    m_setApiButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #28a745;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 8px 15px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #218838;"
        "}"
    );
    
    m_testApiButton = new QPushButton("检测连接", this);
    m_testApiButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #17a2b8;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 8px 15px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #138496;"
        "}"
    );
    
    apiButtonLayout->addWidget(m_setApiButton);
    apiButtonLayout->addWidget(m_testApiButton);
    apiButtonLayout->addStretch();
    
    apiLayout->addLayout(appIdLayout);
    apiLayout->addLayout(secretKeyLayout);
    apiLayout->addLayout(apiButtonLayout);
    
    // 输入区域
    m_inputLabel = new QLabel("输入要翻译的文本:", this);
    m_inputLabel->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    
    m_inputTextEdit = new QTextEdit(this);
    m_inputTextEdit->setPlaceholderText("请输入要翻译的文本...");
    m_inputTextEdit->setMaximumHeight(120);
    m_inputTextEdit->setFont(QFont("Microsoft YaHei", 10));
    
    // 语言选择区域
    m_languageLayout = new QHBoxLayout();
    m_languageLayout->setSpacing(10);
    
    m_fromLabel = new QLabel("从:", this);
    m_fromLanguageCombo = new QComboBox(this);
    m_fromLanguageCombo->setMinimumWidth(120);
    
    m_swapButton = new QPushButton("⇄", this);
    m_swapButton->setToolTip("交换语言");
    m_swapButton->setMaximumWidth(40);
    m_swapButton->setStyleSheet("QPushButton { font-size: 16px; font-weight: bold; }");
    
    m_toLabel = new QLabel("到:", this);
    m_toLanguageCombo = new QComboBox(this);
    m_toLanguageCombo->setMinimumWidth(120);
    
    m_languageLayout->addWidget(m_fromLabel);
    m_languageLayout->addWidget(m_fromLanguageCombo);
    m_languageLayout->addWidget(m_swapButton);
    m_languageLayout->addWidget(m_toLabel);
    m_languageLayout->addWidget(m_toLanguageCombo);
    m_languageLayout->addStretch();
    
    // 翻译按钮
    m_translateButton = new QPushButton("翻译", this);
    m_translateButton->setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
    m_translateButton->setMinimumHeight(40);
    m_translateButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #4CAF50;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 10px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #45a049;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #3d8b40;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #cccccc;"
        "    color: #666666;"
        "}"
    );
    
    // 输出区域
    m_outputLabel = new QLabel("翻译结果:", this);
    m_outputLabel->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));
    
    m_outputTextEdit = new QTextEdit(this);
    m_outputTextEdit->setReadOnly(true);
    m_outputTextEdit->setPlaceholderText("翻译结果将显示在这里...");
    m_outputTextEdit->setMaximumHeight(120);
    m_outputTextEdit->setFont(QFont("Microsoft YaHei", 10));
    m_outputTextEdit->setStyleSheet(
        "QTextEdit {"
        "    background-color: #f8f9fa;"
        "    border: 1px solid #dee2e6;"
        "    border-radius: 5px;"
        "}"
    );
    
    // 状态栏
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_progressBar->setTextVisible(false);
    
    m_statusLabel = new QLabel("就绪", this);
    m_statusLabel->setStyleSheet("color: #666666;");
    
    QHBoxLayout *statusLayout = new QHBoxLayout();
    statusLayout->addWidget(m_progressBar);
    statusLayout->addWidget(m_statusLabel);
    
    // 组装布局 - API配置区域移到最上面
    m_mainLayout->addWidget(apiGroup);
    m_mainLayout->addWidget(m_inputLabel);
    m_mainLayout->addWidget(m_inputTextEdit);
    m_mainLayout->addLayout(m_languageLayout);
    m_mainLayout->addWidget(m_translateButton);
    m_mainLayout->addWidget(m_outputLabel);
    m_mainLayout->addWidget(m_outputTextEdit);
    m_mainLayout->addLayout(statusLayout);
    
    // 设置样式
    setStyleSheet(
        "QMainWindow {"
        "    background-color: #ffffff;"
        "}"
        "QLabel {"
        "    color: #333333;"
        "}"
        "QTextEdit {"
        "    border: 1px solid #cccccc;"
        "    border-radius: 5px;"
        "    padding: 5px;"
        "}"
        "QComboBox {"
        "    border: 1px solid #cccccc;"
        "    border-radius: 3px;"
        "    padding: 5px;"
        "    background-color: white;"
        "    color: #333333;"
        "    font-size: 10px;"
        "}"
        "QComboBox:hover {"
        "    border: 1px solid #007bff;"
        "}"
        "QComboBox:focus {"
        "    border: 2px solid #007bff;"
        "}"
        "QComboBox::drop-down {"
        "    border: none;"
        "    width: 20px;"
        "}"
        "QComboBox::down-arrow {"
        "    image: none;"
        "    border-left: 5px solid transparent;"
        "    border-right: 5px solid transparent;"
        "    border-top: 5px solid #666666;"
        "    margin-right: 5px;"
        "}"
        "QComboBox QAbstractItemView {"
        "    border: 1px solid #cccccc;"
        "    border-radius: 3px;"
        "    background-color: white;"
        "    selection-background-color: #007bff;"
        "    selection-color: white;"
        "    outline: none;"
        "}"
        "QComboBox QAbstractItemView::item {"
        "    padding: 8px 12px;"
        "    border: none;"
        "    background-color: transparent;"
        "    color: #333333;"
        "}"
        "QComboBox QAbstractItemView::item:hover {"
        "    background-color: #e3f2fd;"
        "    color: #333333;"
        "}"
        "QComboBox QAbstractItemView::item:selected {"
        "    background-color: #007bff;"
        "    color: white;"
        "    font-weight: bold;"
        "}"
        "QPushButton {"
        "    border: 1px solid #cccccc;"
        "    border-radius: 3px;"
        "    padding: 5px 10px;"
        "    background-color: #f8f9fa;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e9ecef;"
        "}"
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 1px solid #cccccc;"
        "    border-radius: 5px;"
        "    margin-top: 10px;"
        "    padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 5px 0 5px;"
        "}"
    );
    
    LOG_DEBUG("UI设置完成", "MainWindow");
}

void MainWindow::setupConnections()
{
    LOG_DEBUG("开始设置信号连接", "MainWindow");
    
    // 连接翻译器信号
    connect(m_translator, &Translator::translationFinished,
            this, &MainWindow::onTranslationFinished);
    connect(m_translator, &Translator::translationError,
            this, &MainWindow::onTranslationError);
    
    // 连接UI信号
    connect(m_translateButton, &QPushButton::clicked, this, &MainWindow::onTranslateClicked);
    connect(m_setApiButton, &QPushButton::clicked, this, &MainWindow::onSetApiClicked);
    connect(m_testApiButton, &QPushButton::clicked, this, &MainWindow::onTestApiClicked);
    connect(m_swapButton, &QPushButton::clicked, this, &MainWindow::onSwapLanguagesClicked);
    
    LOG_DEBUG("信号连接设置完成", "MainWindow");
}

void MainWindow::updateLanguageComboBoxes()
{
    LOG_DEBUG("更新语言选择框", "MainWindow");
    
    m_fromLanguageCombo->clear();
    m_toLanguageCombo->clear();
    
    for (int i = 0; i < Config::LANGUAGE_NAMES.size(); ++i) {
        m_fromLanguageCombo->addItem(Config::LANGUAGE_NAMES[i], Config::SUPPORTED_LANGUAGES[i]);
        m_toLanguageCombo->addItem(Config::LANGUAGE_NAMES[i], Config::SUPPORTED_LANGUAGES[i]);
    }
    
    // 设置默认语言
    m_fromLanguageCombo->setCurrentText("自动检测");
    m_toLanguageCombo->setCurrentText("英语");
    
    LOG_INFO(QString("语言选择框已更新，支持 %1 种语言").arg(Config::LANGUAGE_NAMES.size()), "MainWindow");
}

void MainWindow::onTranslateClicked()
{
    QString text = m_inputTextEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        LOG_USER_ACTION("翻译按钮点击", "输入文本为空");
        showError("请输入要翻译的文本");
        return;
    }
    
    QString fromLang = getLanguageCode(m_fromLanguageCombo->currentText());
    QString toLang = getLanguageCode(m_toLanguageCombo->currentText());
    
    LOG_USER_ACTION("翻译按钮点击", QString("文本长度: %1, 从: %2, 到: %3").arg(text.length()).arg(fromLang).arg(toLang));
    
    if (!m_translator->isApiConfigured()) {
        LOG_WARNING("API未配置", "MainWindow");
        showError("请先配置API密钥");
        return;
    }
    
    // 更新UI状态
    m_translateButton->setEnabled(false);
    m_translateButton->setText("翻译中...");
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // 无限进度条
    m_statusLabel->setText("正在翻译...");
    
    LOG_INFO("开始翻译请求", "MainWindow");
    
    // 开始翻译
    m_translator->translateText(text, fromLang, toLang);
}

void MainWindow::onTranslationFinished(const QString &translatedText, const QString &detectedLang)
{
    LOG_DEBUG("翻译完成", "MainWindow");
    
    m_outputTextEdit->setText(translatedText);
    m_progressBar->setVisible(false);
    m_translateButton->setEnabled(true);
    m_translateButton->setText("翻译");
    m_statusLabel->setText(QString("翻译完成 (检测语言: %1)").arg(detectedLang));
    
    // 如果是API测试，恢复按钮状态
    if (m_testApiButton->text() == "测试中...") {
        m_testApiButton->setEnabled(true);
        m_testApiButton->setText("检测连接");
        
        QString message = QString("API连接成功！\n测试翻译：你好 → %1\n检测语言：%2")
                         .arg(translatedText)
                         .arg(detectedLang);
        showInfo(message);
    }
    
    LOG_USER_ACTION("翻译完成", QString("翻译结果: %1").arg(translatedText.left(50)));
}

void MainWindow::onTranslationError(const QString &errorMessage)
{
    LOG_ERROR("翻译失败", errorMessage);
    
    m_progressBar->setVisible(false);
    m_translateButton->setEnabled(true);
    m_translateButton->setText("翻译");
    m_statusLabel->setText("翻译失败");
    
    // 如果是API测试，恢复按钮状态
    if (m_testApiButton->text() == "测试中...") {
        m_testApiButton->setEnabled(true);
        m_testApiButton->setText("检测连接");
        showError("API连接失败：" + errorMessage);
    } else {
        showError("翻译失败：" + errorMessage);
    }
    
    LOG_USER_ACTION("翻译失败", errorMessage);
}

void MainWindow::onSetApiClicked()
{
    QString appId = m_appIdEdit->text().trimmed();
    QString secretKey = m_secretKeyEdit->text().trimmed();
    
    if (appId.isEmpty() || secretKey.isEmpty()) {
        showError("App ID和Secret Key不能为空");
        return;
    }
    
    // 设置到翻译器
    m_translator->setApiCredentials(appId, secretKey);
    
    // 保存到配置文件
    m_settings->setValue("appId", appId);
    m_settings->setValue("secretKey", secretKey);
    
    showInfo("API设置已保存并生效");
    
    LOG_USER_ACTION("API设置", "设置并保存API密钥");
    LOG_SETTINGS_CHANGE("appId", "", appId);
    LOG_SETTINGS_CHANGE("secretKey", "", secretKey);
}

void MainWindow::onTestApiClicked()
{
    QString appId = m_appIdEdit->text().trimmed();
    QString secretKey = m_secretKeyEdit->text().trimmed();
    
    if (appId.isEmpty() || secretKey.isEmpty()) {
        showError("请先输入App ID和Secret Key");
        return;
    }
    
    // 临时设置API密钥进行测试
    m_translator->setApiCredentials(appId, secretKey);
    
    // 开始测试翻译
    m_testApiButton->setEnabled(false);
    m_testApiButton->setText("测试中...");
    m_statusLabel->setText("正在测试API连接...");
    
    LOG_USER_ACTION("API测试", "开始测试API连接");
    
    // 测试翻译"你好"为英文
    m_translator->translateText("你好", "zh", "en");
}

void MainWindow::onSwapLanguagesClicked()
{
    QString fromText = m_fromLanguageCombo->currentText();
    QString toText = m_toLanguageCombo->currentText();
    
    // 避免交换自动检测
    if (fromText == "自动检测") {
        LOG_WARNING("尝试交换自动检测语言", "MainWindow");
        showInfo("自动检测模式无法交换语言");
        return;
    }
    
    m_fromLanguageCombo->setCurrentText(toText);
    m_toLanguageCombo->setCurrentText(fromText);
    
    LOG_USER_ACTION("语言交换", QString("从 %1 交换到 %2").arg(fromText).arg(toText));
    LOG_DEBUG("语言选择已交换", "MainWindow");
}

void MainWindow::loadSettings()
{
    LOG_DEBUG("开始加载设置", "MainWindow");
    
    // 加载API设置
    QString appId = m_settings->value("appId", "").toString();
    QString secretKey = m_settings->value("secretKey", "").toString();
    
    m_appIdEdit->setText(appId);
    m_secretKeyEdit->setText(secretKey);
    
    // 设置到翻译器
    if (!appId.isEmpty() && !secretKey.isEmpty()) {
        m_translator->setApiCredentials(appId, secretKey);
        LOG_INFO("API设置已加载", "MainWindow");
    }
    
    // 加载语言设置
    QString fromLang = m_settings->value("fromLanguage", "auto").toString();
    QString toLang = m_settings->value("toLanguage", "en").toString();
    
    // 设置语言选择
    int fromIndex = m_fromLanguageCombo->findData(fromLang);
    if (fromIndex >= 0) {
        m_fromLanguageCombo->setCurrentIndex(fromIndex);
    }
    
    int toIndex = m_toLanguageCombo->findData(toLang);
    if (toIndex >= 0) {
        m_toLanguageCombo->setCurrentIndex(toIndex);
    }
    
    LOG_DEBUG("设置加载完成", "MainWindow");
}

void MainWindow::saveSettings()
{
    LOG_DEBUG("开始保存设置", "MainWindow");
    
    // 保存语言设置
    QString fromLang = m_fromLanguageCombo->currentData().toString();
    QString toLang = m_toLanguageCombo->currentData().toString();
    
    m_settings->setValue("fromLanguage", fromLang);
    m_settings->setValue("toLanguage", toLang);
    
    LOG_DEBUG("设置保存完成", "MainWindow");
}

QString MainWindow::getLanguageCode(const QString &languageName)
{
    int index = Config::LANGUAGE_NAMES.indexOf(languageName);
    if (index >= 0 && index < Config::SUPPORTED_LANGUAGES.size()) {
        return Config::SUPPORTED_LANGUAGES[index];
    }
    return "auto";
}

QString MainWindow::getLanguageName(const QString &languageCode)
{
    int index = Config::SUPPORTED_LANGUAGES.indexOf(languageCode);
    if (index >= 0 && index < Config::LANGUAGE_NAMES.size()) {
        return Config::LANGUAGE_NAMES[index];
    }
    return "未知语言";
}

void MainWindow::showError(const QString &message)
{
    LOG_ERROR(QString("显示错误对话框: %1").arg(message), "MainWindow");
    QMessageBox::critical(this, "错误", message);
}

void MainWindow::showInfo(const QString &message)
{
    LOG_INFO(QString("显示信息对话框: %1").arg(message), "MainWindow");
    QMessageBox::information(this, "提示", message);
} 