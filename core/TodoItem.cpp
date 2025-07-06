
#include <QJsonArray>
#include <QJsonObject>

#include "core/TodoItem.h"

QJsonObject TodoItem::toJson() const {
    QJsonObject json;
    json["id"] = m_id.toString(QUuid::WithoutBraces);
    json["title"] = m_title;
    json["description"] = m_description;
    json["creationDate"] = m_creationDate.toString(Qt::ISODate);
    json["dueDate"] = m_dueDate.toString(Qt::ISODate);
    json["reminderDate"] = m_reminderDate.toString(Qt::ISODate);
    json["isCompleted"] = m_isCompleted;
    json["completionDate"] = m_completionDate.toString(Qt::ISODate);

    // 将步骤列表转换为 QJsonArray
    QJsonArray subTasksArray;
    for (const SubTask& subTask : m_subTasks) {
        subTasksArray.append(subTask.toJson());
    }
    json["subTasks"] = subTasksArray;

    return json;
}

TodoItem TodoItem::fromJson(const QJsonObject& json) {
    // 创建一个临时的 TodoItem 来填充数据
    TodoItem task(json["title"].toString());

    task.m_id = QUuid(json["id"].toString());
    task.m_description = json["description"].toString();
    task.m_creationDate = QDateTime::fromString(json["creationDate"].toString(), Qt::ISODate);
    task.m_dueDate = QDateTime::fromString(json["dueDate"].toString(), Qt::ISODate);
    task.m_reminderDate = QDateTime::fromString(json["reminderDate"].toString(), Qt::ISODate);
    task.m_isCompleted = json["isCompleted"].toBool();
    task.m_completionDate = QDateTime::fromString(json["completionDate"].toString(), Qt::ISODate);


    // 从 QJsonArray 恢复步骤列表
    if (json.contains("subTasks") && json["subTasks"].isArray()) {
        QJsonArray subTasksArray = json["subTasks"].toArray();
        for (const QJsonValue& value : subTasksArray) {
            task.m_subTasks.append(SubTask::fromJson(value.toObject()));
        }
    }

    return task;
}
