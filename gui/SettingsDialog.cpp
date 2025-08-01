#include "gui/SettingsDialog.h"
#include "services/NetworkService.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QStackedWidget>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QMessageBox>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent)
{
    setupUi();
    // 当登录成功时，显示账户页面
    connect(NetworkService::instance(), &NetworkService::loginSuccess, this, &SettingsDialog::showAccountPage);
    // 当注册成功时，显示提示信息
    connect(NetworkService::instance(), &NetworkService::registrationSuccess, this, [this](){
        QMessageBox::information(this, tr("注册申请已提交"),
                                 tr("感谢您的注册！\n您的账户申请已成功提交，我们将在24小时内进行审核。"));
        showLoginPage();
    });

    connect(NetworkService::instance(), &NetworkService::loginFailed, this, [this](const QString& error){
        QMessageBox::warning(this, tr("登录失败"), error);
    });

    // 【新增】连接注册失败的信号
    connect(NetworkService::instance(), &NetworkService::registrationFailed, this, [this](const QString& error){
        QMessageBox::warning(this, tr("注册失败"), error);
    });
    // (您也可以连接登录/注册失败的信号，以显示错误提示)
}

void SettingsDialog::setupUi()
{
    setWindowTitle(tr("设置与账户"));
    setMinimumWidth(450);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // --- 服务器设置 ---
    QGroupBox* serverGroupBox = new QGroupBox(tr("服务器"));
    QFormLayout* serverLayout = new QFormLayout(serverGroupBox);
    m_serverComboBox = new QComboBox();
    m_serverComboBox->addItem(tr("官方服务器 (chronovault.app)"));
    m_serverComboBox->addItem(tr("自定义服务器"));
    m_serverComboBox->setEditable(true);
    serverLayout->addRow(tr("服务器地址:"), m_serverComboBox);

    // --- 主堆叠窗口 (用于切换登录/注册/账户信息) ---
    m_mainStack = new QStackedWidget();

    // --- 页面1: 登录页 (Login Page) ---
    QWidget* loginPage = new QWidget();
    QVBoxLayout* loginPageLayout = new QVBoxLayout(loginPage);
    QFormLayout* loginForm = new QFormLayout();
    m_loginUsernameEdit = new QLineEdit();
    m_loginUsernameEdit->setPlaceholderText(tr("用户名或邮箱"));
    m_loginPasswordEdit = new QLineEdit();
    m_loginPasswordEdit->setEchoMode(QLineEdit::Password);
    loginForm->addRow(tr("账户:"), m_loginUsernameEdit);
    loginForm->addRow(tr("密码:"), m_loginPasswordEdit);
    QPushButton* loginButton = new QPushButton(tr("登录"));
    QPushButton* goToRegisterButton = new QPushButton(tr("没有账户？点击注册"));
    goToRegisterButton->setFlat(true);
    goToRegisterButton->setCursor(Qt::PointingHandCursor);
    loginPageLayout->addLayout(loginForm);
    loginPageLayout->addWidget(loginButton, 0, Qt::AlignRight);
    loginPageLayout->addSpacing(10);
    loginPageLayout->addWidget(goToRegisterButton, 0, Qt::AlignCenter);

    // --- 页面2: 注册页 (Register Page) ---
    QWidget* registerPage = new QWidget();
    QVBoxLayout* registerPageLayout = new QVBoxLayout(registerPage);
    QFormLayout* registerForm = new QFormLayout();
    m_registerUsernameEdit = new QLineEdit();
    m_registerEmailEdit = new QLineEdit();
    m_registerPasswordEdit = new QLineEdit();
    m_registerPasswordEdit->setEchoMode(QLineEdit::Password);
    m_registerConfirmPasswordEdit = new QLineEdit();
    m_registerConfirmPasswordEdit->setEchoMode(QLineEdit::Password);
    registerForm->addRow(tr("用户名:"), m_registerUsernameEdit);
    registerForm->addRow(tr("邮箱:"), m_registerEmailEdit);
    registerForm->addRow(tr("密码:"), m_registerPasswordEdit);
    registerForm->addRow(tr("确认密码:"), m_registerConfirmPasswordEdit);
    QPushButton* registerButton = new QPushButton(tr("注册"));
    QPushButton* goToLoginButton = new QPushButton(tr("已有账户？返回登录"));
    goToLoginButton->setFlat(true);
    goToLoginButton->setCursor(Qt::PointingHandCursor);
    registerPageLayout->addLayout(registerForm);
    registerPageLayout->addWidget(registerButton, 0, Qt::AlignRight);
    registerPageLayout->addSpacing(10);
    registerPageLayout->addWidget(goToLoginButton, 0, Qt::AlignCenter);

    // --- 页面3: 账户信息页 (Account Info Page) ---
    QWidget* accountPage = new QWidget();
    QVBoxLayout* accountPageLayout = new QVBoxLayout(accountPage);
    QGroupBox* accountInfoGroupBox = new QGroupBox(tr("当前账户信息"));
    QFormLayout* accountInfoForm = new QFormLayout(accountInfoGroupBox);
    m_accountUsernameLabel = new QLabel();
    m_accountEmailLabel = new QLabel();
    m_accountLevelLabel = new QLabel();
    m_accountStorageLabel = new QLabel();
    accountInfoForm->addRow(tr("用户名:"), m_accountUsernameLabel);
    accountInfoForm->addRow(tr("邮箱:"), m_accountEmailLabel);
    accountInfoForm->addRow(tr("用户级别:"), m_accountLevelLabel);
    accountInfoForm->addRow(tr("存储空间:"), m_accountStorageLabel);
    QPushButton* logoutButton = new QPushButton(tr("退出登录"));
    accountPageLayout->addWidget(accountInfoGroupBox);
    accountPageLayout->addStretch();
    accountPageLayout->addWidget(logoutButton, 0, Qt::AlignRight);


    m_mainStack->addWidget(loginPage);      // Index 0
    m_mainStack->addWidget(registerPage);   // Index 1
    m_mainStack->addWidget(accountPage);    // Index 2

    // --- 连接信号和槽 ---
    connect(goToRegisterButton, &QPushButton::clicked, this, &SettingsDialog::showRegisterPage);
    connect(goToLoginButton, &QPushButton::clicked, this, &SettingsDialog::showLoginPage);
    connect(loginButton, &QPushButton::clicked, this, &SettingsDialog::onLoginClicked);
    connect(registerButton, &QPushButton::clicked, this, &SettingsDialog::onRegisterClicked);
    connect(logoutButton, &QPushButton::clicked, this, &SettingsDialog::onLogoutClicked);

    // --- 最终布局 ---
    mainLayout->addWidget(serverGroupBox);
    mainLayout->addWidget(m_mainStack);
    mainLayout->addStretch();
}

void SettingsDialog::showRegisterPage() { m_mainStack->setCurrentIndex(1); }
void SettingsDialog::showLoginPage() { m_mainStack->setCurrentIndex(0); }

// 【占位符】未来这里将是真正的网络请求逻辑
void SettingsDialog::onLoginClicked()
{
    // 未来在这里调用 NetworkService::instance()->login(...)
    // 并根据返回结果决定是否调用 showAccountPage

    NetworkService::instance()->setServerUrl(m_serverComboBox->currentText());
    NetworkService::instance()->login(m_loginUsernameEdit->text(), m_loginPasswordEdit->text());
}

void SettingsDialog::onLogoutClicked()
{
    // 未来在这里调用 NetworkService::instance()->logout()
    m_loginPasswordEdit->clear();
    showLoginPage();
}

// 注册后显示“待审核”信息
void SettingsDialog::onRegisterClicked()
{
    // 【核心修改】替换占位符
    NetworkService::instance()->setServerUrl(m_serverComboBox->currentText());
    NetworkService::instance()->registerUser(m_registerUsernameEdit->text(), m_registerEmailEdit->text(), m_registerPasswordEdit->text());
}

// 显示账户信息的辅助函数
void SettingsDialog::showAccountPage(const QString& username, const QString& email, const QString& userLevel, const QString& storage)
{
    m_accountUsernameLabel->setText(username);
    m_accountEmailLabel->setText(email);
    m_accountLevelLabel->setText(userLevel);
    m_accountStorageLabel->setText(storage);
    m_mainStack->setCurrentIndex(2);
}
