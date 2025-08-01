#include "services/NetworkService.h"
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

NetworkService* NetworkService::instance()
{
    static NetworkService service;
    return &service;
}

NetworkService::NetworkService(QObject *parent) : QObject(parent)
{
    // 创建网络访问管理器
    m_manager = new QNetworkAccessManager(this);
}

NetworkService::~NetworkService() = default;

void NetworkService::setServerUrl(const QString& url)
{
    // 移除末尾的斜杠（如果有），以规范化URL
    QString cleanUrl = url;
    if (cleanUrl.endsWith('/')) {
        cleanUrl.chop(1);
    }

    // 如果用户输入的地址不以 http 开头，则为其补上
    if (!cleanUrl.startsWith("http://") && !cleanUrl.startsWith("https://")) {
        m_serverUrl = "http://" + cleanUrl;
    } else {
        m_serverUrl = cleanUrl;
    }
    qDebug() << "NetworkService: Server URL set to" << m_serverUrl;
}

// --- 真实的登录请求 ---
void NetworkService::login(const QString& username, const QString& password)
{
    qDebug() << "Attempting to log in user:" << username;

    // 1. 将用户名和密码打包成JSON
    QJsonObject json;
    json["username"] = username;
    json["password"] = password;
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    // 2. 创建网络请求，并设置好API端点和请求头
    QNetworkRequest request(QUrl(m_serverUrl + "/api/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 3. 发送POST请求
    QNetworkReply* reply = m_manager->post(request, data);

    // 4. 将请求的 finished 信号连接到我们的处理槽
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        onLoginReply(reply);
    });
}

// --- 真实的注册请求 ---
void NetworkService::registerUser(const QString& username, const QString& email, const QString& password)
{
    qDebug() << "Attempting to register user:" << username;

    QJsonObject json;
    json["username"] = username;
    json["email"] = email;
    json["password"] = password;
    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    QNetworkRequest request(QUrl(m_serverUrl + "/api/register")); // 假设注册API的地址是 /api/register
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = m_manager->post(request, data);
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        onRegisterReply(reply);
    });
}


// --- 处理服务器的真实响应 ---
void NetworkService::onLoginReply(QNetworkReply* reply)
{
    // 1. 检查网络层面是否有错误 (如无法连接服务器)
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Login failed (Network Error):" << reply->errorString();
        emit loginFailed(tr("网络错误: %1").arg(reply->errorString()));
        reply->deleteLater();
        return;
    }

    // 2. 读取服务器返回的所有数据
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

    // 3. 检查返回的数据是否是有效的JSON
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        qDebug() << "Login failed: Invalid JSON response from server.";
        emit loginFailed(tr("服务器返回了无效的数据格式。"));
        reply->deleteLater();
        return;
    }

    // 4. 解析JSON对象
    QJsonObject jsonObj = jsonDoc.object();
    if (jsonObj.value("success").toBool(false)) { // 安全地转换为bool
        // --- 登录成功 ---
        QJsonObject userData = jsonObj.value("userData").toObject();
        QString username = userData.value("username").toString();
        QString email = userData.value("email").toString();
        QString level = userData.value("level").toString();
        QString storage = userData.value("storage").toString();
        // (未来，您需要在这里获取并安全地保存 token)
        // QString token = jsonObj.value("token").toString();

        qDebug() << "Login successful for user:" << username;
        emit loginSuccess(username, email, level, storage);

    } else {
        // --- 登录失败 (后端返回的业务错误) ---
        QString error = jsonObj.value("error").toString(tr("发生了未知错误"));
        qDebug() << "Login failed (API Error):" << error;
        emit loginFailed(error);
    }

    reply->deleteLater(); // 非常重要：确保reply对象被正确销毁
}

void NetworkService::onRegisterReply(QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Registration failed (Network Error):" << reply->errorString();
        emit registrationFailed(tr("网络错误: %1").arg(reply->errorString()));
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        qDebug() << "Registration failed: Invalid JSON response.";
        emit registrationFailed(tr("服务器返回了无效的数据格式。"));
        reply->deleteLater();
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    if (jsonObj.value("success").toBool(false)) {
        qDebug() << "Registration request successful.";
        emit registrationSuccess();
    } else {
        QString error = jsonObj.value("error").toString(tr("发生了未知错误"));
        qDebug() << "Registration failed (API Error):" << error;
        emit registrationFailed(error);
    }

    reply->deleteLater();
}
