#pragma once
#include <QString>
#include <QDateTime>
#include <QUuid>
#include <QList> // 引入 QList
#include "core/SubTask.h"
#include "core/Reminder.h"

class TodoItem {
public:
    // 构造函数现在包含创建日期
//    TodoItem(QString title, QString description = "", QDateTime dueDate = QDateTime())
//        : m_id(QUuid::createUuid()),
//        m_title(std::move(title)),
//        m_description(std::move(description)),
//        m_creationDate(QDateTime::currentDateTime()), // 记录创建时间
//        m_dueDate(std::move(dueDate)),
//        m_isCompleted(false) {
//    }

    explicit TodoItem(const QString& title = "");

    // Getters
    QUuid id() const;
    QString title() const;
    QString description() const;
    QDateTime creationDate() const;
    QDateTime dueDate() const;
    bool isCompleted() const;
    const QList<SubTask>& subTasks() const;
    QDateTime completionDate() const;
    Reminder reminder() const;
    QUuid sourceAnniversaryId() const;

    // Setters
    void setTitle(const QString& title);
    void setDescription(const QString& description);
    void setDueDate(const QDateTime& dueDate);
    void setReminder(const Reminder& reminder) { m_reminder = reminder; }
    void setCompleted(bool completed);
    QList<SubTask>& subTasks();
    void setCompletionDate(const QDateTime& completionDate);
    void setSourceAnniversaryId(const QUuid& id);

    QJsonObject toJson() const;
    static TodoItem fromJson(const QJsonObject& json);

private:
    QUuid m_id;
    QString m_title;
    QString m_description; // 这个将用作“备注”

    QDateTime m_creationDate;
    QDateTime m_dueDate;
    //QDateTime m_reminderDate;
    Reminder m_reminder;

    bool m_isCompleted;
    QList<SubTask> m_subTasks;
    QDateTime m_completionDate;
    QUuid m_sourceAnniversaryId;
};
