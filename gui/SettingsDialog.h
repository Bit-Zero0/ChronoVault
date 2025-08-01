// gui/SettingsDialog.h

#pragma once
#include <QDialog>

QT_BEGIN_NAMESPACE
class QStackedWidget;
class QWidget; // 【新增】
class QLabel; // 【新增】
class QLineEdit;
class QComboBox;
QT_END_NAMESPACE

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

private slots:
    void showRegisterPage();
    void showLoginPage();
    void onLoginClicked();
    void onRegisterClicked();
    void onLogoutClicked(); // 【新增】登出槽函数

private:
    void setupUi();
    void showAccountPage(const QString& username, const QString& email, const QString& userLevel, const QString& storage); // 【新增】用于显示账户信息的辅助函数

    QStackedWidget* m_mainStack; // 【重命名】

    // 登录页面控件
    QLineEdit* m_loginUsernameEdit;
    QLineEdit* m_loginPasswordEdit;

    // 注册页面控件
    QLineEdit* m_registerUsernameEdit;
    QLineEdit* m_registerEmailEdit;
    QLineEdit* m_registerPasswordEdit;
    QLineEdit* m_registerConfirmPasswordEdit;

    // 【新增】账户信息页面控件
    QLabel* m_accountUsernameLabel;
    QLabel* m_accountEmailLabel;
    QLabel* m_accountLevelLabel;
    QLabel* m_accountStorageLabel;

    // 服务器设置控件
    QComboBox* m_serverComboBox;
};
