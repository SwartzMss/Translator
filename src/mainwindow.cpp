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
    loadSettings();
    setupConnections();
    createTrayIcon();

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
    
    // API配置区域 - 直接显示在主窗口
    QGroupBox *apiGroup = new QGroupBox("API配置", this);
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

    // DeepSeek Key
    QHBoxLayout *deepSeekLayout = new QHBoxLayout();
    QLabel *deepSeekLabel = new QLabel("DeepSeek Key:", this);
    deepSeekLabel->setMinimumWidth(80);
    m_deepSeekKeyEdit = new QLineEdit(this);
    m_deepSeekKeyEdit->setPlaceholderText("请输入DeepSeek API Key");
    m_deepSeekKeyEdit->setEchoMode(QLineEdit::Password);
    deepSeekLayout->addWidget(deepSeekLabel);
    deepSeekLayout->addWidget(m_deepSeekKeyEdit);

    // Proxy Host
    QHBoxLayout *proxyHostLayout = new QHBoxLayout();
    QLabel *proxyHostLabel = new QLabel("代理IP:", this);
    proxyHostLabel->setMinimumWidth(80);
    m_proxyHostEdit = new QLineEdit(this);
    m_proxyHostEdit->setPlaceholderText("例如 127.0.0.1");
    proxyHostLayout->addWidget(proxyHostLabel);
    proxyHostLayout->addWidget(m_proxyHostEdit);

    // Proxy Port
    QHBoxLayout *proxyPortLayout = new QHBoxLayout();
    QLabel *proxyPortLabel = new QLabel("端口:", this);
    proxyPortLabel->setMinimumWidth(80);
    m_proxyPortEdit = new QLineEdit(this);
    m_proxyPortEdit->setPlaceholderText("如 7890");
    proxyPortLayout->addWidget(proxyPortLabel);
    proxyPortLayout->addWidget(m_proxyPortEdit);

    // Proxy User
    QHBoxLayout *proxyUserLayout = new QHBoxLayout();
    QLabel *proxyUserLabel = new QLabel("用户名:", this);
    proxyUserLabel->setMinimumWidth(80);
    m_proxyUserEdit = new QLineEdit(this);
    m_proxyUserEdit->setPlaceholderText("可选");
    proxyUserLayout->addWidget(proxyUserLabel);
    proxyUserLayout->addWidget(m_proxyUserEdit);

    // Proxy Password
    QHBoxLayout *proxyPassLayout = new QHBoxLayout();
    QLabel *proxyPassLabel = new QLabel("密码:", this);
    proxyPassLabel->setMinimumWidth(80);
    m_proxyPasswordEdit = new QLineEdit(this);
    m_proxyPasswordEdit->setPlaceholderText("可选");
    m_proxyPasswordEdit->setEchoMode(QLineEdit::Password);
    proxyPassLayout->addWidget(proxyPassLabel);
    proxyPassLayout->addWidget(m_proxyPasswordEdit);

    // CA Certificate
    QHBoxLayout *certLayout = new QHBoxLayout();
    QLabel *certLabel = new QLabel("证书文件:", this);
    certLabel->setMinimumWidth(80);
    m_caCertPathEdit = new QLineEdit(this);
    m_caCertPathEdit->setPlaceholderText("可选，CA证书路径");
    m_browseCertButton = new QPushButton("浏览...", this);
    certLayout->addWidget(certLabel);
    certLayout->addWidget(m_caCertPathEdit);
    certLayout->addWidget(m_browseCertButton);
    
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
    apiLayout->addLayout(deepSeekLayout);
    apiLayout->addLayout(proxyHostLayout);
    apiLayout->addLayout(proxyPortLayout);
    apiLayout->addLayout(proxyUserLayout);
    apiLayout->addLayout(proxyPassLayout);
    apiLayout->addLayout(certLayout);
    apiLayout->addLayout(apiButtonLayout);
    
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
    m_mainLayout->addWidget(m_tabWidget);
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
    connect(m_setApiButton, &QPushButton::clicked, this, &MainWindow::onSetApiClicked);
    connect(m_testApiButton, &QPushButton::clicked, this, &MainWindow::onTestApiClicked);
    connect(m_swapButton, &QPushButton::clicked, this, &MainWindow::onSwapLanguagesClicked);
    connect(m_browseCertButton, &QPushButton::clicked, this, &MainWindow::onBrowseCertClicked);
    
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
    LOG_DEBUG("翻译完成");
    
    m_translateOutputEdit->setText(translatedText);
    m_progressBar->setVisible(false);
    m_translateButton->setEnabled(true);
    m_translateButton->setText("翻译");
    m_polishButton->setEnabled(true);
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
    if (m_testApiButton->text() == "测试中...") {
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

void MainWindow::onSetApiClicked()
{
    QString appId = m_appIdEdit->text().trimmed();
    QString secretKey = m_secretKeyEdit->text().trimmed();
    QString deepSeekKey = m_deepSeekKeyEdit->text().trimmed();
    QString proxyHost = m_proxyHostEdit->text().trimmed();
    QString proxyPortStr = m_proxyPortEdit->text().trimmed();
    QString proxyUser = m_proxyUserEdit->text().trimmed();
    QString proxyPassword = m_proxyPasswordEdit->text().trimmed();
    QString caCertPath = m_caCertPathEdit->text().trimmed();

    if (appId.isEmpty() || secretKey.isEmpty()) {
        showError("App ID和Secret Key不能为空");
        return;
    }

    // 设置到翻译器
    m_translator->setApiCredentials(appId, secretKey);

    quint16 proxyPort = proxyPortStr.toUShort();
    m_translator->setNetworkProxy(proxyHost, proxyPort, proxyUser, proxyPassword);
    m_polisher->setNetworkProxy(proxyHost, proxyPort, proxyUser, proxyPassword);
    m_translator->addCaCertificate(caCertPath);
    m_polisher->addCaCertificate(caCertPath);

    // DeepSeek Key 可选
    if (!deepSeekKey.isEmpty()) {
        m_polisher->setApiKey(deepSeekKey);
    }
    
    // 保存到配置文件
    m_settings->setValue("appId", appId);
    m_settings->setValue("secretKey", secretKey);
    m_settings->setValue("deepSeekKey", deepSeekKey);
    m_settings->setValue("proxyHost", proxyHost);
    m_settings->setValue("proxyPort", proxyPortStr);
    m_settings->setValue("proxyUser", proxyUser);
    m_settings->setValue("proxyPassword", proxyPassword);
    m_settings->setValue("caCertPath", caCertPath);
    
    showInfo("API设置已保存并生效");
    
    LOG_INFO("API设置 - 设置并保存API密钥");
    LOG_INFO(QString("appId: %1***").arg(appId.left(4)));
    LOG_INFO("secretKey: ******");
    LOG_INFO("deepSeekKey: ******");
    if (!proxyHost.isEmpty()) {
        LOG_INFO(QString("proxy: %1:%2").arg(proxyHost).arg(proxyPort));
    } else {
        LOG_INFO("proxy: <none>");
    }
    if (!caCertPath.isEmpty()) {
        LOG_INFO(QString("cert: %1").arg(caCertPath));
    }
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
    QString proxyHost = m_settings->value("proxyHost", "").toString();
    QString proxyPortStr = m_settings->value("proxyPort", "").toString();
    QString proxyUser = m_settings->value("proxyUser", "").toString();
    QString proxyPassword = m_settings->value("proxyPassword", "").toString();
    QString caCertPath = m_settings->value("caCertPath", "").toString();
    
    m_appIdEdit->setText(appId);
    m_secretKeyEdit->setText(secretKey);
    m_deepSeekKeyEdit->setText(deepSeekKey);
    m_proxyHostEdit->setText(proxyHost);
    m_proxyPortEdit->setText(proxyPortStr);
    m_proxyUserEdit->setText(proxyUser);
    m_proxyPasswordEdit->setText(proxyPassword);
    m_caCertPathEdit->setText(caCertPath);
    
    // 设置到翻译器
    if (!appId.isEmpty() && !secretKey.isEmpty()) {
        m_translator->setApiCredentials(appId, secretKey);
        LOG_INFO("API设置已加载");
    }
    if (!deepSeekKey.isEmpty()) {
        m_polisher->setApiKey(deepSeekKey);
    }
    quint16 proxyPort = proxyPortStr.toUShort();
    m_translator->setNetworkProxy(proxyHost, proxyPort, proxyUser, proxyPassword);
    m_polisher->setNetworkProxy(proxyHost, proxyPort, proxyUser, proxyPassword);
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
    
    LOG_DEBUG("设置保存完成");
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
