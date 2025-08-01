#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QString>

QT_BEGIN_NAMESPACE
class QNetworkReply;
QT_END_NAMESPACE

class NetworkService : public QObject
{
    Q_OBJECT

public:
    static NetworkService* instance();

    // --- 公共接口 ---
    void setServerUrl(const QString& url);
    void login(const QString& username, const QString& password);
    void registerUser(const QString& username, const QString& email, const QString& password);
    // 未来还会有: void syncData(const QString& token);

signals:
    // --- 信号，用于向UI层报告网络操作的结果 ---
    void loginSuccess(const QString& username, const QString& email, const QString& userLevel, const QString& storageInfo);
    void loginFailed(const QString& error);
    void registrationSuccess();
    void registrationFailed(const QString& error);

private slots:
    // --- 私有槽，用于处理网络请求的返回结果 ---
    void onLoginReply(QNetworkReply* reply);
    void onRegisterReply(QNetworkReply* reply);

private:
    explicit NetworkService(QObject* parent = nullptr);
    ~NetworkService();
    NetworkService(const NetworkService&) = delete;
    NetworkService& operator=(const NetworkService&) = delete;

    // Qt网络编程的核心类
    QNetworkAccessManager* m_manager;
    // 存储当前要连接的服务器地址
    QString m_serverUrl;
};
