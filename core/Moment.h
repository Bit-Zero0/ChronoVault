#pragma once

#include <QDateTime>
#include <QString>
#include <QJsonObject>
#include <QUuid>
#include <QStringList>
#include <QJsonArray>

class Moment {
public:
    // 构造函数
    Moment();
    explicit Moment(const QString& text);

    // Getters & Setters
    QUuid id() const;
    QDateTime timestamp() const;
    QString text() const;
    void setText(const QString& text);
    const QStringList& imagePaths() const;
    void addImagePath(const QString& path);
    void clearImagePaths();

    // 序列化
    QJsonObject toJson() const;
    static Moment fromJson(const QJsonObject& json);

private:
    QUuid m_id;
    QDateTime m_timestamp;      // 此瞬间的记录时间
    QString m_text;             // 文字描述 (未来支持Markdown)
    QStringList m_imagePaths;   // 存储关联的图片本地路径
};
