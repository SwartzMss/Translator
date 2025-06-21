#include "mainwindow.h"
#include "config.h"
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
}

MainWindow::~MainWindow()
{
    saveSettings();
}

void MainWindow::setupUi()
{
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
}

void MainWindow::setupConnections()
{
    connect(m_translateButton, &QPushButton::clicked, this, &MainWindow::onTranslateClicked);
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    connect(m_swapButton, &QPushButton::clicked, this, &MainWindow::onSwapLanguagesClicked);
    
    connect(m_translator, &Translator::translationFinished, 
            this, &MainWindow::onTranslationFinished);
    connect(m_translator, &Translator::translationError, 
            this, &MainWindow::onTranslationError);
}

void MainWindow::updateLanguageComboBoxes()
{
    m_fromLanguageCombo->clear();
    m_toLanguageCombo->clear();
    
    for (int i = 0; i < Config::LANGUAGE_NAMES.size(); ++i) {
        m_fromLanguageCombo->addItem(Config::LANGUAGE_NAMES[i], Config::SUPPORTED_LANGUAGES[i]);
        m_toLanguageCombo->addItem(Config::LANGUAGE_NAMES[i], Config::SUPPORTED_LANGUAGES[i]);
    }
    
    // 设置默认语言
    m_fromLanguageCombo->setCurrentText("自动检测");
    m_toLanguageCombo->setCurrentText("英语");
}

void MainWindow::onTranslateClicked()
{
    QString text = m_inputTextEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        showError("请输入要翻译的文本");
        return;
    }
    
    QString fromLang = getLanguageCode(m_fromLanguageCombo->currentText());
    QString toLang = getLanguageCode(m_toLanguageCombo->currentText());
    
    if (!m_translator->isApiConfigured()) {
        showSettingsDialog();
        return;
    }
    
    // 更新UI状态
    m_translateButton->setEnabled(false);
    m_translateButton->setText("翻译中...");
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // 无限进度条
    m_statusLabel->setText("正在翻译...");
    
    // 开始翻译
    m_translator->translateText(text, fromLang, toLang);
}

void MainWindow::onTranslationFinished(const QString &translatedText, const QString &detectedLang)
{
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
    }
}

void MainWindow::onTranslationError(const QString &errorMessage)
{
    showError(errorMessage);
    
    // 恢复UI状态
    m_translateButton->setEnabled(true);
    m_translateButton->setText("翻译");
    m_progressBar->setVisible(false);
    m_statusLabel->setText("翻译失败");
}

void MainWindow::onClearClicked()
{
    m_inputTextEdit->clear();
    m_outputTextEdit->clear();
    m_statusLabel->setText("就绪");
}

void MainWindow::onSettingsClicked()
{
    showSettingsDialog();
}

void MainWindow::onSwapLanguagesClicked()
{
    QString fromText = m_fromLanguageCombo->currentText();
    QString toText = m_toLanguageCombo->currentText();
    
    // 避免交换自动检测
    if (fromText == "自动检测") {
        showInfo("自动检测模式无法交换语言");
        return;
    }
    
    m_fromLanguageCombo->setCurrentText(toText);
    m_toLanguageCombo->setCurrentText(fromText);
}

void MainWindow::showSettingsDialog()
{
    QString currentAppId = m_settings->value("appId", "").toString();
    QString currentSecretKey = m_settings->value("secretKey", "").toString();
    
    bool ok;
    QString appId = QInputDialog::getText(this, "API设置", 
                                        "请输入百度翻译API的App ID:", 
                                        QLineEdit::Normal, currentAppId, &ok);
    if (!ok) return;
    
    QString secretKey = QInputDialog::getText(this, "API设置", 
                                             "请输入百度翻译API的Secret Key:", 
                                             QLineEdit::Password, currentSecretKey, &ok);
    if (!ok) return;
    
    if (appId.trimmed().isEmpty() || secretKey.trimmed().isEmpty()) {
        showError("App ID和Secret Key不能为空");
        return;
    }
    
    m_translator->setApiCredentials(appId.trimmed(), secretKey.trimmed());
    m_settings->setValue("appId", appId.trimmed());
    m_settings->setValue("secretKey", secretKey.trimmed());
    
    showInfo("API密钥设置成功！");
}

void MainWindow::loadSettings()
{
    QString appId = m_settings->value("appId", "").toString();
    QString secretKey = m_settings->value("secretKey", "").toString();
    
    if (!appId.isEmpty() && !secretKey.isEmpty()) {
        m_translator->setApiCredentials(appId, secretKey);
    }
}

void MainWindow::saveSettings()
{
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
    QMessageBox::critical(this, "错误", message);
}

void MainWindow::showInfo(const QString &message)
{
    QMessageBox::information(this, "提示", message);
} 