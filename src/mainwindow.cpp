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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_translator(new Translator(this))
    , m_settings(new QSettings("Translator", "TranslatorApp", this))
{
    LOG_INFO("主窗口初始化开始", "MainWindow");
    
    setupUi();
    setupConnections();
    loadSettings();
    updateLanguageComboBoxes();
    
    // 设置窗口属性
    setWindowTitle(QString::fromUtf8(Config::APP_NAME_STR) + " v" + QString::fromUtf8(Config::APP_VERSION_STR));
    setMinimumSize(Config::WINDOW_WIDTH, Config::WINDOW_HEIGHT);
    
    // 居中显示
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    LOG_INFO("主窗口初始化完成", "MainWindow");
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
    
    // 工具栏
    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    
    m_clearButton = new QPushButton("清空", this);
    m_clearButton->setToolTip("清空输入和输出");
    
    m_settingsButton = new QPushButton("设置", this);
    m_settingsButton->setToolTip("配置API密钥");
    
    toolbarLayout->addWidget(m_clearButton);
    toolbarLayout->addWidget(m_settingsButton);
    toolbarLayout->addStretch();
    
    // 状态栏
    m_progressBar = new QProgressBar(this);
    m_progressBar->setVisible(false);
    m_progressBar->setTextVisible(false);
    
    m_statusLabel = new QLabel("就绪", this);
    m_statusLabel->setStyleSheet("color: #666666;");
    
    QHBoxLayout *statusLayout = new QHBoxLayout();
    statusLayout->addWidget(m_progressBar);
    statusLayout->addWidget(m_statusLabel);
    
    // 组装布局
    m_mainLayout->addWidget(m_inputLabel);
    m_mainLayout->addWidget(m_inputTextEdit);
    m_mainLayout->addLayout(m_languageLayout);
    m_mainLayout->addWidget(m_translateButton);
    m_mainLayout->addWidget(m_outputLabel);
    m_mainLayout->addWidget(m_outputTextEdit);
    m_mainLayout->addLayout(toolbarLayout);
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
    );
    
    LOG_DEBUG("UI设置完成", "MainWindow");
}

void MainWindow::setupConnections()
{
    LOG_DEBUG("开始设置信号连接", "MainWindow");
    
    connect(m_translateButton, &QPushButton::clicked, this, &MainWindow::onTranslateClicked);
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    connect(m_swapButton, &QPushButton::clicked, this, &MainWindow::onSwapLanguagesClicked);
    
    connect(m_translator, &Translator::translationFinished, 
            this, &MainWindow::onTranslationFinished);
    connect(m_translator, &Translator::translationError, 
            this, &MainWindow::onTranslationError);
            
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
        LOG_WARNING("API未配置，显示设置对话框", "MainWindow");
        showSettingsDialog();
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
    LOG_INFO("翻译完成", "MainWindow");
    
    m_outputTextEdit->setPlainText(translatedText);
    
    // 恢复UI状态
    m_translateButton->setEnabled(true);
    m_translateButton->setText("翻译");
    m_progressBar->setVisible(false);
    
    // 更新状态信息
    QString detectedLangName = getLanguageName(detectedLang);
    m_statusLabel->setText(QString("翻译完成 (检测到: %1)").arg(detectedLangName));
    
    // 如果源语言是自动检测，更新源语言选择
    if (m_fromLanguageCombo->currentText() == "自动检测" && detectedLang != "auto") {
        m_fromLanguageCombo->setCurrentText(detectedLangName);
        LOG_DEBUG(QString("自动更新源语言为: %1").arg(detectedLangName), "MainWindow");
    }
    
    LOG_USER_ACTION("翻译完成", QString("检测语言: %1, 译文长度: %2").arg(detectedLangName).arg(translatedText.length()));
}

void MainWindow::onTranslationError(const QString &errorMessage)
{
    LOG_ERROR(QString("翻译错误: %1").arg(errorMessage), "MainWindow");
    
    showError(errorMessage);
    
    // 恢复UI状态
    m_translateButton->setEnabled(true);
    m_translateButton->setText("翻译");
    m_progressBar->setVisible(false);
    m_statusLabel->setText("翻译失败");
    
    LOG_USER_ACTION("翻译失败", errorMessage);
}

void MainWindow::onClearClicked()
{
    LOG_USER_ACTION("清空按钮点击", "清空输入和输出");
    
    m_inputTextEdit->clear();
    m_outputTextEdit->clear();
    m_statusLabel->setText("就绪");
    
    LOG_DEBUG("界面已清空", "MainWindow");
}

void MainWindow::onSettingsClicked()
{
    LOG_USER_ACTION("设置按钮点击", "打开API设置对话框");
    showSettingsDialog();
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

void MainWindow::showSettingsDialog()
{
    QString currentAppId = m_settings->value("appId", "").toString();
    QString currentSecretKey = m_settings->value("secretKey", "").toString();
    
    bool ok;
    QString appId = QInputDialog::getText(this, "API设置", 
                                        "请输入百度翻译API的App ID:", 
                                        QLineEdit::Normal, currentAppId, &ok);
    if (!ok) {
        LOG_USER_ACTION("设置对话框", "用户取消App ID输入");
        return;
    }
    
    QString secretKey = QInputDialog::getText(this, "API设置", 
                                             "请输入百度翻译API的Secret Key:", 
                                             QLineEdit::Password, currentSecretKey, &ok);
    if (!ok) {
        LOG_USER_ACTION("设置对话框", "用户取消Secret Key输入");
        return;
    }
    
    if (appId.trimmed().isEmpty() || secretKey.trimmed().isEmpty()) {
        LOG_WARNING("API设置失败", "App ID或Secret Key为空");
        showError("App ID和Secret Key不能为空");
        return;
    }
    
    m_translator->setApiCredentials(appId.trimmed(), secretKey.trimmed());
    m_settings->setValue("appId", appId.trimmed());
    m_settings->setValue("secretKey", secretKey.trimmed());
    
    LOG_USER_ACTION("API设置", "API密钥设置成功");
    showInfo("API密钥设置成功！");
}

void MainWindow::loadSettings()
{
    LOG_DEBUG("加载应用设置", "MainWindow");
    
    QString appId = m_settings->value("appId", "").toString();
    QString secretKey = m_settings->value("secretKey", "").toString();
    
    if (!appId.isEmpty() && !secretKey.isEmpty()) {
        m_translator->setApiCredentials(appId, secretKey);
        LOG_INFO("已加载保存的API设置", "MainWindow");
    } else {
        LOG_INFO("未找到保存的API设置", "MainWindow");
    }
}

void MainWindow::saveSettings()
{
    LOG_DEBUG("保存应用设置", "MainWindow");
    // 设置已在showSettingsDialog中保存
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