#pragma once
#include <QString>
#include <QUuid>
#include <QJsonObject>


struct SubTask {
    QUuid id;
    QString title;
    bool isCompleted;

    SubTask(const QString& title = "")
        : id(QUuid::createUuid()), title(title), isCompleted(false) {
    }

    // 将 SubTask 对象转换为 QJsonObject
    QJsonObject toJson() const {
        QJsonObject json;
        json["id"] = id.toString(QUuid::WithoutBraces);
        json["title"] = title;
        json["isCompleted"] = isCompleted;
        return json;
    }

    // 从 QJsonObject 创建一个 SubTask 对象
    static SubTask fromJson(const QJsonObject& json) {
        SubTask task;
        task.id = QUuid(json["id"].toString());
        task.title = json["title"].toString();
        task.isCompleted = json["isCompleted"].toBool();
        return task;
    }
};
