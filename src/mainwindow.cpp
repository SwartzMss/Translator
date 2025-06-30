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
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QFileDialog>
#include "deepseekclient.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_translator(new Translator(this))
    , m_polisher(new DeepSeekClient(this))
    , m_settings(nullptr)
{
    LOG_DEBUG("主窗口构造函数开始");
    
    // 设置配置文件路径到exe同级目录
    QString exePath = QCoreApplication::applicationDirPath();
    QString configPath = exePath + "/translator.ini";
    m_settings = new QSettings(configPath, QSettings::IniFormat, this);
    
    LOG_INFO(QString("配置文件路径: %1").arg(configPath));
    
    setupUi();
    updateLanguageComboBoxes();
    setupConnections();
    createTrayIcon();
    loadSettings();

    // 设置窗口属性
    setWindowTitle("百度翻译工具");
    setMinimumSize(600, 500);
    Qt::WindowFlags flags = windowFlags();
    flags &= ~Qt::WindowMinimizeButtonHint;
    flags &= ~Qt::WindowMaximizeButtonHint;
    setWindowFlags(flags);
    
    // 居中显示窗口
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    LOG_DEBUG("主窗口构造函数完成");
    LOG_INFO("应用启动 - 主窗口创建完成");
}

MainWindow::~MainWindow()
{
    LOG_INFO("主窗口销毁");
    saveSettings();
}

void MainWindow::setupUi()
{
    LOG_DEBUG("开始设置UI");
    
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // 创建TabWidget
    m_tabWidget = new QTabWidget(this);

    // ------- 翻译Tab -------
    m_translateTab = new QWidget(this);
    QVBoxLayout *translateLayout = new QVBoxLayout(m_translateTab);

    m_translateInputLabel = new QLabel("输入要翻译的文本:", this);
    m_translateInputLabel->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));

    m_translateInputEdit = new QTextEdit(this);
    m_translateInputEdit->setPlaceholderText("请输入要翻译的文本...");
    m_translateInputEdit->setMaximumHeight(120);
    m_translateInputEdit->setFont(QFont("Microsoft YaHei", 10));
    
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
    
    // 操作按钮
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

    // 翻译Tab输出区域
    m_translateOutputLabel = new QLabel("翻译结果:", this);
    m_translateOutputLabel->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));

    m_translateOutputEdit = new QTextEdit(this);
    m_translateOutputEdit->setReadOnly(true);
    m_translateOutputEdit->setPlaceholderText("翻译结果将显示在这里...");
    m_translateOutputEdit->setMaximumHeight(120);
    m_translateOutputEdit->setFont(QFont("Microsoft YaHei", 10));
    m_translateOutputEdit->setStyleSheet(
        "QTextEdit {"
        "    background-color: #f8f9fa;"
        "    border: 1px solid #dee2e6;"
        "    border-radius: 5px;"
        "}"
    );

    translateLayout->addWidget(m_translateInputLabel);
    translateLayout->addWidget(m_translateInputEdit);
    translateLayout->addLayout(m_languageLayout);
    QHBoxLayout *translateButtonLayout = new QHBoxLayout();
    translateButtonLayout->addWidget(m_translateButton);
    translateButtonLayout->addStretch();
    translateLayout->addLayout(translateButtonLayout);
    translateLayout->addWidget(m_translateOutputLabel);
    translateLayout->addWidget(m_translateOutputEdit);

    m_tabWidget->addTab(m_translateTab, "翻译");

    // ------- 润色Tab -------
    m_polishTab = new QWidget(this);
    QVBoxLayout *polishLayout = new QVBoxLayout(m_polishTab);

    m_polishInputLabel = new QLabel("输入要润色的英文文本:", this);
    m_polishInputLabel->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));

    m_polishInputEdit = new QTextEdit(this);
    m_polishInputEdit->setPlaceholderText("请输入要润色的英文文本...");
    m_polishInputEdit->setMaximumHeight(120);
    m_polishInputEdit->setFont(QFont("Microsoft YaHei", 10));

    m_polishButton = new QPushButton("润色", this);
    m_polishButton->setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
    m_polishButton->setMinimumHeight(40);
    m_polishButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #007bff;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 10px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #0069d9;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #005cbf;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #cccccc;"
        "    color: #666666;"
        "}"
    );

    m_polishOutputLabel = new QLabel("润色结果:", this);
    m_polishOutputLabel->setFont(QFont("Microsoft YaHei", 10, QFont::Bold));

    m_polishOutputEdit = new QTextEdit(this);
    m_polishOutputEdit->setReadOnly(true);
    m_polishOutputEdit->setPlaceholderText("润色结果将显示在这里...");
    m_polishOutputEdit->setMaximumHeight(120);
    m_polishOutputEdit->setFont(QFont("Microsoft YaHei", 10));
    m_polishOutputEdit->setStyleSheet(
        "QTextEdit {"
        "    background-color: #f8f9fa;"
        "    border: 1px solid #dee2e6;"
        "    border-radius: 5px;"
        "}"
    );

    polishLayout->addWidget(m_polishInputLabel);
    polishLayout->addWidget(m_polishInputEdit);
    QHBoxLayout *polishButtonLayout = new QHBoxLayout();
    polishButtonLayout->addWidget(m_polishButton);
    polishButtonLayout->addStretch();
    polishLayout->addLayout(polishButtonLayout);
    polishLayout->addWidget(m_polishOutputLabel);
    polishLayout->addWidget(m_polishOutputEdit);

    m_tabWidget->addTab(m_polishTab, "润色");
    
    // 设置按钮
    m_settingsButton = new QPushButton("设置", this);
    m_settingsButton->setText("设置");
    m_settingsButton->setToolTip("配置API密钥和代理设置");
    m_settingsButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #6c757d;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 8px 15px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #5a6268;"
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
    statusLayout->addStretch();
    statusLayout->addWidget(m_settingsButton);
    
    // 组装布局
    m_mainLayout->addWidget(m_tabWidget);
    m_mainLayout->addLayout(statusLayout);
    
    // 创建设置对话框
    createSettingsDialog();
    
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
        "QTabWidget::pane {"
        "    border: 1px solid #cccccc;"
        "    border-radius: 5px;"
        "}"
        "QTabBar::tab {"
        "    background-color: #f8f9fa;"
        "    border: 1px solid #cccccc;"
        "    padding: 8px 16px;"
        "    margin-right: 2px;"
        "}"
        "QTabBar::tab:selected {"
        "    background-color: white;"
        "    border-bottom: 2px solid #007bff;"
        "}"
    );
    
    LOG_DEBUG("UI设置完成");
}

void MainWindow::setupConnections()
{
    LOG_DEBUG("开始设置信号连接");
    
    // 连接翻译器信号
    connect(m_translator, &Translator::translationFinished,
            this, &MainWindow::onTranslationFinished);
    connect(m_translator, &Translator::translationError,
            this, &MainWindow::onTranslationError);
    connect(m_polisher, &DeepSeekClient::polishFinished,
            this, &MainWindow::onPolishFinished);
    connect(m_polisher, &DeepSeekClient::polishError,
            this, &MainWindow::onPolishError);
    
    // 连接UI信号
    connect(m_translateButton, &QPushButton::clicked, this, &MainWindow::onTranslateClicked);
    connect(m_polishButton, &QPushButton::clicked, this, &MainWindow::onPolishClicked);
    connect(m_settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    connect(m_swapButton, &QPushButton::clicked, this, &MainWindow::onSwapLanguagesClicked);
    
    LOG_DEBUG("信号连接设置完成");
}

void MainWindow::updateLanguageComboBoxes()
{
    LOG_DEBUG("更新语言选择框");
    
    m_fromLanguageCombo->clear();
    m_toLanguageCombo->clear();
    
    for (int i = 0; i < Config::LANGUAGE_NAMES.size(); ++i) {
        m_fromLanguageCombo->addItem(Config::LANGUAGE_NAMES[i], Config::SUPPORTED_LANGUAGES[i]);
        m_toLanguageCombo->addItem(Config::LANGUAGE_NAMES[i], Config::SUPPORTED_LANGUAGES[i]);
    }
    
    // 设置默认语言：英文 -> 中文
    m_fromLanguageCombo->setCurrentText("英语");
    m_toLanguageCombo->setCurrentText("中文");
    
    LOG_INFO(QString("语言选择框已更新，支持 %1 种语言").arg(Config::LANGUAGE_NAMES.size()));
}

void MainWindow::onTranslateClicked()
{
    QString text = m_translateInputEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        LOG_INFO("翻译按钮点击 - 输入文本为空");
        showError("请输入要翻译的文本");
        return;
    }
    
    QString fromLang = getLanguageCode(m_fromLanguageCombo->currentText());
    QString toLang = getLanguageCode(m_toLanguageCombo->currentText());
    
    LOG_INFO(QString("翻译按钮点击 - 文本长度: %1, 从: %2, 到: %3").arg(text.length()).arg(fromLang).arg(toLang));
    
    if (!m_translator->isApiConfigured()) {
        LOG_WARNING("API未配置");
        showError("请先配置API密钥");
        return;
    }
    
    // 更新UI状态
    m_translateButton->setEnabled(false);
    m_translateButton->setText("翻译中...");
    m_polishButton->setEnabled(false);
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // 无限进度条
    m_statusLabel->setText("正在翻译...");
    
    LOG_INFO("开始翻译请求");
    
    // 开始翻译
    m_translator->translateText(text, fromLang, toLang);
}

void MainWindow::onPolishClicked()
{
    QString text = m_polishInputEdit->toPlainText().trimmed();
    if (text.isEmpty()) {
        showError("请输入要润色的文本");
        return;
    }

    QString apiKey = m_deepSeekKeyEdit->text().trimmed();
    if (apiKey.isEmpty()) {
        showError("请先配置DeepSeek Key");
        return;
    }

    m_polisher->setApiKey(apiKey);

    m_polishButton->setEnabled(false);
    m_polishButton->setText("润色中...");
    m_translateButton->setEnabled(false);
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0);
    m_statusLabel->setText("正在润色...");

    m_polisher->polishText(text);
}

void MainWindow::onTranslationFinished(const QString &translatedText, const QString &detectedLang)
{
    m_translateOutputEdit->setText(translatedText);
    m_progressBar->setVisible(false);
    m_translateButton->setEnabled(true);
    m_translateButton->setText("翻译");
    m_polishButton->setEnabled(true);
    m_statusLabel->setText("翻译完成");
    
    // 如果是API测试，显示成功信息
    if (m_testApiButton && m_testApiButton->text() == "测试中...") {
        m_testApiButton->setEnabled(true);
        m_testApiButton->setText("检测连接");
        QString message = QString("API连接成功！\n测试翻译结果：%1\n检测到的语言：%2")
                         .arg(translatedText)
                         .arg(detectedLang);
        showInfo(message);
    }
    
    LOG_INFO(QString("翻译完成 - 翻译结果: %1").arg(translatedText.left(50)));
}

void MainWindow::onTranslationError(const QString &errorMessage)
{
    LOG_ERROR(QString("翻译失败: %1").arg(errorMessage));
    
    m_progressBar->setVisible(false);
    m_translateButton->setEnabled(true);
    m_translateButton->setText("翻译");
    m_polishButton->setEnabled(true);
    m_statusLabel->setText("翻译失败");
    
    // 如果是API测试，恢复按钮状态
    if (m_testApiButton && m_testApiButton->text() == "测试中...") {
        m_testApiButton->setEnabled(true);
        m_testApiButton->setText("检测连接");
        showError("API连接失败：" + errorMessage);
    } else {
        showError("翻译失败：" + errorMessage);
    }
    
    LOG_INFO(QString("翻译失败 - %1").arg(errorMessage));
}

void MainWindow::onPolishFinished(const QString &polishedText)
{
    m_polishOutputEdit->setText(polishedText);
    m_progressBar->setVisible(false);
    m_translateButton->setEnabled(true);
    m_polishButton->setEnabled(true);
    m_polishButton->setText("润色");
    m_statusLabel->setText("润色完成");
}

void MainWindow::onPolishError(const QString &errorMessage)
{
    m_progressBar->setVisible(false);
    m_translateButton->setEnabled(true);
    m_polishButton->setEnabled(true);
    m_polishButton->setText("润色");
    m_statusLabel->setText("润色失败");
    showError("润色失败：" + errorMessage);
}

void MainWindow::onSettingsClicked()
{
    // 显示设置对话框
    m_settingsDialog->show();
    m_settingsDialog->raise();
    m_settingsDialog->activateWindow();
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
    
    LOG_INFO("API测试 - 开始测试API连接");
    
    // 测试翻译"你好"为英文
    m_translator->translateText("你好", "zh", "en");
}

void MainWindow::onSwapLanguagesClicked()
{
    QString fromText = m_fromLanguageCombo->currentText();
    QString toText = m_toLanguageCombo->currentText();
    
    // 避免交换自动检测
    if (fromText == "自动检测") {
        LOG_WARNING("尝试交换自动检测语言");
        showInfo("自动检测模式无法交换语言");
        return;
    }
    
    m_fromLanguageCombo->setCurrentText(toText);
    m_toLanguageCombo->setCurrentText(fromText);
    
    LOG_INFO(QString("语言交换 - 从 %1 交换到 %2").arg(fromText).arg(toText));
    LOG_DEBUG("语言选择已交换");
}

void MainWindow::loadSettings()
{
    LOG_DEBUG("开始加载设置");
    
    // 加载API设置
    QString appId = m_settings->value("appId", "").toString();
    QString secretKey = m_settings->value("secretKey", "").toString();
    QString deepSeekKey = m_settings->value("deepSeekKey", "").toString();
    bool proxyEnabled = m_settings->value("proxyEnabled", false).toBool();
    QString proxyHost = m_settings->value("proxyHost", "").toString();
    QString proxyPortStr = m_settings->value("proxyPort", "").toString();
    QString proxyUser = m_settings->value("proxyUser", "").toString();
    QString proxyPassword = m_settings->value("proxyPassword", "").toString();
    QString caCertPath = m_settings->value("caCertPath", "").toString();
    
    // 设置到对话框组件（如果已创建）
    if (m_appIdEdit) {
        m_appIdEdit->setText(appId);
        m_secretKeyEdit->setText(secretKey);
        m_deepSeekKeyEdit->setText(deepSeekKey);
        m_proxyEnableCheck->setChecked(proxyEnabled);
        m_proxyHostEdit->setText(proxyHost);
        m_proxyPortEdit->setText(proxyPortStr);
        m_proxyUserEdit->setText(proxyUser);
        m_proxyPasswordEdit->setText(proxyPassword);
        m_caCertPathEdit->setText(caCertPath);

        m_proxyHostEdit->setEnabled(proxyEnabled);
        m_proxyPortEdit->setEnabled(proxyEnabled);
        m_proxyUserEdit->setEnabled(proxyEnabled);
        m_proxyPasswordEdit->setEnabled(proxyEnabled);
        m_caCertPathEdit->setEnabled(proxyEnabled);
        m_browseCertButton->setEnabled(proxyEnabled);
    }
    
    // 设置到翻译器
    if (!appId.isEmpty() && !secretKey.isEmpty()) {
        m_translator->setApiCredentials(appId, secretKey);
        LOG_INFO("API设置已加载");
    }
    if (!deepSeekKey.isEmpty()) {
        m_polisher->setApiKey(deepSeekKey);
    }
    quint16 proxyPort = proxyPortStr.toUShort();
    if (proxyEnabled) {
        m_translator->setProxy(proxyHost, proxyPort, proxyUser, proxyPassword);
        m_polisher->setProxy(proxyHost, proxyPort, proxyUser, proxyPassword);
    } else {
        m_translator->setProxy(QString(), 0);
        m_polisher->setProxy(QString(), 0);
    }
    m_translator->addCaCertificate(caCertPath);
    m_polisher->addCaCertificate(caCertPath);
    
    // 加载语言设置，默认为英文 -> 中文
    QString fromLang = m_settings->value("fromLanguage", "en").toString();
    QString toLang = m_settings->value("toLanguage", "zh").toString();
    
    // 设置语言选择
    int fromIndex = m_fromLanguageCombo->findData(fromLang);
    if (fromIndex >= 0) {
        m_fromLanguageCombo->setCurrentIndex(fromIndex);
    }
    
    int toIndex = m_toLanguageCombo->findData(toLang);
    if (toIndex >= 0) {
        m_toLanguageCombo->setCurrentIndex(toIndex);
    }
    
    LOG_DEBUG("设置加载完成");
}

void MainWindow::saveSettings()
{
    LOG_DEBUG("开始保存设置");
    
    // 保存语言设置
    QString fromLang = m_fromLanguageCombo->currentData().toString();
    QString toLang = m_toLanguageCombo->currentData().toString();
    
    m_settings->setValue("fromLanguage", fromLang);
    m_settings->setValue("toLanguage", toLang);
    
    // 保存API配置
    QString appId = m_appIdEdit->text().trimmed();
    QString secretKey = m_secretKeyEdit->text().trimmed();
    QString deepSeekKey = m_deepSeekKeyEdit->text().trimmed();
    
    m_settings->setValue("appId", appId);
    m_settings->setValue("secretKey", secretKey);
    m_settings->setValue("deepSeekKey", deepSeekKey);
    
    // 保存代理设置
    bool proxyEnabled = m_proxyEnableCheck->isChecked();
    QString proxyHost = m_proxyHostEdit->text().trimmed();
    QString proxyPortStr = m_proxyPortEdit->text().trimmed();
    QString proxyUser = m_proxyUserEdit->text().trimmed();
    QString proxyPassword = m_proxyPasswordEdit->text().trimmed();
    QString caCertPath = m_caCertPathEdit->text().trimmed();
    
    m_settings->setValue("proxyEnabled", proxyEnabled);
    m_settings->setValue("proxyHost", proxyHost);
    m_settings->setValue("proxyPort", proxyPortStr);
    m_settings->setValue("proxyUser", proxyUser);
    m_settings->setValue("proxyPassword", proxyPassword);
    m_settings->setValue("caCertPath", caCertPath);
    
    // 应用配置到翻译器
    if (!appId.isEmpty() && !secretKey.isEmpty()) {
        m_translator->setApiCredentials(appId, secretKey);
    }
    if (!deepSeekKey.isEmpty()) {
        m_polisher->setApiKey(deepSeekKey);
    }
    
    quint16 proxyPort = proxyPortStr.toUShort();
    if (proxyEnabled) {
        m_translator->setProxy(proxyHost, proxyPort, proxyUser, proxyPassword);
        m_polisher->setProxy(proxyHost, proxyPort, proxyUser, proxyPassword);
    } else {
        m_translator->setProxy(QString(), 0);
        m_polisher->setProxy(QString(), 0);
    }
    m_translator->addCaCertificate(caCertPath);
    m_polisher->addCaCertificate(caCertPath);
    
    LOG_DEBUG("设置保存完成");
    LOG_INFO("所有配置已保存并生效");
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
    LOG_ERROR(QString("显示错误对话框: %1").arg(message));
    QMessageBox::critical(this, "错误", message);
}

void MainWindow::showInfo(const QString &message)
{
    LOG_INFO(QString("显示信息对话框: %1").arg(message));
    QMessageBox::information(this, "提示", message);
}

void MainWindow::createTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(QIcon(":/icon.ico"), this);

    m_trayMenu = new QMenu(this);
    m_showAction = new QAction(tr("显示"), this);
    m_exitAction = new QAction(tr("退出"), this);

    m_trayMenu->addAction(m_showAction);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction(m_exitAction);

    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->setToolTip(tr("Translator"));
    m_trayIcon->show();

    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &MainWindow::onTrayActivated);
    connect(m_showAction, &QAction::triggered,
            this, &MainWindow::onShowTriggered);
    connect(m_exitAction, &QAction::triggered,
            this, &MainWindow::onExitTriggered);
}

void MainWindow::onTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        onShowTriggered();
    }
}

void MainWindow::onShowTriggered()
{
    this->showNormal();
    this->activateWindow();
}

void MainWindow::onExitTriggered()
{
    qApp->quit();
}

void MainWindow::onBrowseCertClicked()
{
    QString file = QFileDialog::getOpenFileName(this, "选择证书文件", QString(),
                                               "Certificates (*.pem *.crt *.cer);;All Files (*)");
    if (!file.isEmpty()) {
        m_caCertPathEdit->setText(file);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_trayIcon && m_trayIcon->isVisible()) {
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::createSettingsDialog()
{
    LOG_DEBUG("创建设置对话框");
    
    m_settingsDialog = new QDialog(this);
    m_settingsDialog->setWindowTitle("设置");
    m_settingsDialog->setModal(true);
    m_settingsDialog->setMinimumSize(500, 400);
    
    QVBoxLayout *dialogLayout = new QVBoxLayout(m_settingsDialog);
    
    // 创建标签页控件
    m_settingsTabWidget = new QTabWidget(m_settingsDialog);
    
    // ------- API配置Tab -------
    m_apiTab = new QWidget(m_settingsDialog);
    QVBoxLayout *apiLayout = new QVBoxLayout(m_apiTab);
    apiLayout->setSpacing(15);
    apiLayout->setContentsMargins(20, 20, 20, 20);
    
    // App ID
    QHBoxLayout *appIdLayout = new QHBoxLayout();
    QLabel *appIdLabel = new QLabel("App ID:", m_apiTab);
    appIdLabel->setMinimumWidth(100);
    m_appIdEdit = new QLineEdit(m_apiTab);
    m_appIdEdit->setPlaceholderText("请输入百度翻译API的App ID");
    appIdLayout->addWidget(appIdLabel);
    appIdLayout->addWidget(m_appIdEdit);
    
    // Secret Key
    QHBoxLayout *secretKeyLayout = new QHBoxLayout();
    QLabel *secretKeyLabel = new QLabel("Secret Key:", m_apiTab);
    secretKeyLabel->setMinimumWidth(100);
    m_secretKeyEdit = new QLineEdit(m_apiTab);
    m_secretKeyEdit->setPlaceholderText("请输入百度翻译API的Secret Key");
    m_secretKeyEdit->setEchoMode(QLineEdit::Password);
    secretKeyLayout->addWidget(secretKeyLabel);
    secretKeyLayout->addWidget(m_secretKeyEdit);

    // DeepSeek Key
    QHBoxLayout *deepSeekLayout = new QHBoxLayout();
    QLabel *deepSeekLabel = new QLabel("DeepSeek Key:", m_apiTab);
    deepSeekLabel->setMinimumWidth(100);
    m_deepSeekKeyEdit = new QLineEdit(m_apiTab);
    m_deepSeekKeyEdit->setPlaceholderText("请输入DeepSeek API Key");
    m_deepSeekKeyEdit->setEchoMode(QLineEdit::Password);
    deepSeekLayout->addWidget(deepSeekLabel);
    deepSeekLayout->addWidget(m_deepSeekKeyEdit);
    
    // API配置按钮
    QHBoxLayout *apiButtonLayout = new QHBoxLayout();
    m_testApiButton = new QPushButton("检测连接", m_apiTab);
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
    
    apiButtonLayout->addWidget(m_testApiButton);
    apiButtonLayout->addStretch();
    
    apiLayout->addLayout(appIdLayout);
    apiLayout->addLayout(secretKeyLayout);
    apiLayout->addLayout(deepSeekLayout);
    apiLayout->addLayout(apiButtonLayout);
    apiLayout->addStretch();
    
    m_settingsTabWidget->addTab(m_apiTab, "API配置");
    
    // ------- 代理设置Tab -------
    m_proxyTab = new QWidget(m_settingsDialog);
    QVBoxLayout *proxyLayout = new QVBoxLayout(m_proxyTab);
    proxyLayout->setSpacing(15);
    proxyLayout->setContentsMargins(20, 20, 20, 20);

    m_proxyEnableCheck = new QCheckBox("启用代理", m_proxyTab);
    proxyLayout->addWidget(m_proxyEnableCheck);

    QHBoxLayout *proxyHostLayout = new QHBoxLayout();
    QLabel *proxyHostLabel = new QLabel("代理IP:", m_proxyTab);
    proxyHostLabel->setMinimumWidth(100);
    m_proxyHostEdit = new QLineEdit(m_proxyTab);
    m_proxyHostEdit->setPlaceholderText("例如 127.0.0.1");
    proxyHostLayout->addWidget(proxyHostLabel);
    proxyHostLayout->addWidget(m_proxyHostEdit);

    QHBoxLayout *proxyPortLayout = new QHBoxLayout();
    QLabel *proxyPortLabel = new QLabel("端口:", m_proxyTab);
    proxyPortLabel->setMinimumWidth(100);
    m_proxyPortEdit = new QLineEdit(m_proxyTab);
    m_proxyPortEdit->setPlaceholderText("如 7890");
    proxyPortLayout->addWidget(proxyPortLabel);
    proxyPortLayout->addWidget(m_proxyPortEdit);

    QHBoxLayout *proxyUserLayout = new QHBoxLayout();
    QLabel *proxyUserLabel = new QLabel("用户名:", m_proxyTab);
    proxyUserLabel->setMinimumWidth(100);
    m_proxyUserEdit = new QLineEdit(m_proxyTab);
    m_proxyUserEdit->setPlaceholderText("可选");
    proxyUserLayout->addWidget(proxyUserLabel);
    proxyUserLayout->addWidget(m_proxyUserEdit);

    QHBoxLayout *proxyPassLayout = new QHBoxLayout();
    QLabel *proxyPassLabel = new QLabel("密码:", m_proxyTab);
    proxyPassLabel->setMinimumWidth(100);
    m_proxyPasswordEdit = new QLineEdit(m_proxyTab);
    m_proxyPasswordEdit->setPlaceholderText("可选");
    m_proxyPasswordEdit->setEchoMode(QLineEdit::Password);
    proxyPassLayout->addWidget(proxyPassLabel);
    proxyPassLayout->addWidget(m_proxyPasswordEdit);

    QHBoxLayout *certLayout = new QHBoxLayout();
    QLabel *certLabel = new QLabel("证书文件:", m_proxyTab);
    certLabel->setMinimumWidth(100);
    m_caCertPathEdit = new QLineEdit(m_proxyTab);
    m_caCertPathEdit->setPlaceholderText("可选，CA证书路径");
    m_browseCertButton = new QPushButton("浏览...", m_proxyTab);
    certLayout->addWidget(certLabel);
    certLayout->addWidget(m_caCertPathEdit);
    certLayout->addWidget(m_browseCertButton);

    proxyLayout->addLayout(proxyHostLayout);
    proxyLayout->addLayout(proxyPortLayout);
    proxyLayout->addLayout(proxyUserLayout);
    proxyLayout->addLayout(proxyPassLayout);
    proxyLayout->addLayout(certLayout);
    proxyLayout->addStretch();
    
    m_settingsTabWidget->addTab(m_proxyTab, "代理设置");
    
    // 对话框按钮
    QHBoxLayout *dialogButtonLayout = new QHBoxLayout();
    QPushButton *okButton = new QPushButton("确定", m_settingsDialog);
    QPushButton *cancelButton = new QPushButton("取消", m_settingsDialog);
    
    okButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #007bff;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 8px 20px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #0069d9;"
        "}"
    );
    
    cancelButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #6c757d;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    padding: 8px 20px;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    background-color: #5a6268;"
        "}"
    );
    
    dialogButtonLayout->addStretch();
    dialogButtonLayout->addWidget(okButton);
    dialogButtonLayout->addWidget(cancelButton);
    
    dialogLayout->addWidget(m_settingsTabWidget);
    dialogLayout->addLayout(dialogButtonLayout);
    
    // 连接信号
    connect(m_testApiButton, &QPushButton::clicked, this, &MainWindow::onTestApiClicked);
    connect(m_browseCertButton, &QPushButton::clicked, this, &MainWindow::onBrowseCertClicked);
    connect(okButton, &QPushButton::clicked, this, &MainWindow::onSettingsOkClicked);
    connect(cancelButton, &QPushButton::clicked, m_settingsDialog, &QDialog::reject);
    connect(m_proxyEnableCheck, &QCheckBox::toggled, [this](bool checked) {
        m_proxyHostEdit->setEnabled(checked);
        m_proxyPortEdit->setEnabled(checked);
        m_proxyUserEdit->setEnabled(checked);
        m_proxyPasswordEdit->setEnabled(checked);
        m_caCertPathEdit->setEnabled(checked);
        m_browseCertButton->setEnabled(checked);
    });
    
    // 设置对话框样式
    m_settingsDialog->setStyleSheet(
        "QDialog {"
        "    background-color: #ffffff;"
        "}"
        "QLabel {"
        "    color: #333333;"
        "    font-weight: bold;"
        "}"
        "QLineEdit {"
        "    border: 1px solid #cccccc;"
        "    border-radius: 3px;"
        "    padding: 5px;"
        "    background-color: white;"
        "}"
        "QLineEdit:focus {"
        "    border: 2px solid #007bff;"
        "}"
        "QCheckBox {"
        "    color: #333333;"
        "    font-weight: bold;"
        "}"
        "QTabWidget::pane {"
        "    border: 1px solid #cccccc;"
        "    border-radius: 5px;"
        "}"
        "QTabBar::tab {"
        "    background-color: #f8f9fa;"
        "    border: 1px solid #cccccc;"
        "    padding: 8px 16px;"
        "    margin-right: 2px;"
        "}"
        "QTabBar::tab:selected {"
        "    background-color: white;"
        "    border-bottom: 2px solid #007bff;"
        "}"
    );
    
    LOG_DEBUG("设置对话框创建完成");
}

void MainWindow::onSettingsOkClicked()
{
    // 保存设置并关闭对话框
    saveSettings();
    showInfo("所有配置已保存并生效");
    m_settingsDialog->accept();
}
