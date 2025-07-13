#include "core/TodoItem.h"
#include <QJsonArray>
#include <QJsonObject>
#include <utility> // for std::move

TodoItem::TodoItem(const QString& title)
    : m_id(QUuid::createUuid()),
    m_title(title),
    m_creationDate(QDateTime::currentDateTime()),
    m_isCompleted(false)
{
}

// --- 实现所有的 Getters ---
QUuid TodoItem::id() const { return m_id; }
QString TodoItem::title() const { return m_title; }
QString TodoItem::description() const { return m_description; }
QDateTime TodoItem::creationDate() const { return m_creationDate; }
QDateTime TodoItem::dueDate() const { return m_dueDate; }
bool TodoItem::isCompleted() const { return m_isCompleted; }
const QList<SubTask>& TodoItem::subTasks() const { return m_subTasks; }
QDateTime TodoItem::completionDate() const { return m_completionDate; }
Reminder TodoItem::reminder() const { return m_reminder; }

// --- 实现所有的 Setters ---
void TodoItem::setTitle(const QString& title) { m_title = title; }
void TodoItem::setDescription(const QString& description) { m_description = description; }
void TodoItem::setDueDate(const QDateTime& dueDate) { m_dueDate = dueDate; }
void TodoItem::setCompleted(bool completed) { m_isCompleted = completed; }
QList<SubTask>& TodoItem::subTasks() { return m_subTasks; }
void TodoItem::setCompletionDate(const QDateTime& completionDate) { m_completionDate = completionDate; }
QUuid TodoItem::sourceAnniversaryId() const { return m_sourceAnniversaryId; }
void TodoItem::setSourceAnniversaryId(const QUuid& id) { m_sourceAnniversaryId = id; }


QJsonObject TodoItem::toJson() const {
    QJsonObject json;
    json["id"] = m_id.toString(QUuid::WithoutBraces);
    json["title"] = m_title;
    json["description"] = m_description;
    json["creationDate"] = m_creationDate.toString(Qt::ISODate);
    json["dueDate"] = m_dueDate.toString(Qt::ISODate);
    json["isCompleted"] = m_isCompleted;
    json["completionDate"] = m_completionDate.toString(Qt::ISODate);
    json["reminder"] = m_reminder.toJson();
    json["sourceAnniversaryId"] = m_sourceAnniversaryId.toString(QUuid::WithoutBraces);

    QJsonArray subTasksArray;
    for (const SubTask& subTask : m_subTasks) {
        subTasksArray.append(subTask.toJson());
    }
    json["subTasks"] = subTasksArray;
    return json;
}

TodoItem TodoItem::fromJson(const QJsonObject& json) {
    // 【核心修正】确保 fromJson 调用的是我们新定义的构造函数
    TodoItem task(json["title"].toString());
    task.m_id = QUuid(json["id"].toString());
    task.m_description = json["description"].toString();
    task.m_creationDate = QDateTime::fromString(json["creationDate"].toString(), Qt::ISODate);
    task.m_dueDate = QDateTime::fromString(json["dueDate"].toString(), Qt::ISODate);
    task.m_isCompleted = json["isCompleted"].toBool();
    task.m_completionDate = QDateTime::fromString(json["completionDate"].toString(), Qt::ISODate);

    if (json.contains("reminder") && json["reminder"].isObject()) {
        task.m_reminder = Reminder::fromJson(json["reminder"].toObject());
    }
    if (json.contains("sourceAnniversaryId")) {
        task.m_sourceAnniversaryId = QUuid(json["sourceAnniversaryId"].toString());
    }

    if (json.contains("subTasks") && json["subTasks"].isArray()) {
        QJsonArray subTasksArray = json["subTasks"].toArray();
        for (const QJsonValue& value : subTasksArray) {
            task.m_subTasks.append(SubTask::fromJson(value.toObject()));
        }
    }
    return task;
}
