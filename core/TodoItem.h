#pragma once
#include <QString>
#include <QDateTime>
#include <QUuid>
#include <QList> // ���� QList
#include "core/SubTask.h" // ���� SubTask

class TodoItem {
public:
    // ���캯�����ڰ�����������
    TodoItem(QString title, QString description = "", QDateTime dueDate = QDateTime())
        : m_id(QUuid::createUuid()),
        m_title(std::move(title)),
        m_description(std::move(description)),
        m_creationDate(QDateTime::currentDateTime()), // ��¼����ʱ��
        m_dueDate(std::move(dueDate)),
        m_isCompleted(false) {
    }

    // Getters
    QUuid id() const { return m_id; }
    const QString& title() const { return m_title; }
    const QString& description() const { return m_description; }
    const QDateTime& creationDate() const { return m_creationDate; }
    const QDateTime& dueDate() const { return m_dueDate; }
    const QDateTime& reminderDate() const { return m_reminderDate; }
    bool isCompleted() const { return m_isCompleted; }
    const QList<SubTask>& subTasks() const { return m_subTasks; }
    const QDateTime& completionDate() const { return m_completionDate; }

    // Setters
    void setTitle(const QString& title) { m_title = title; }
    void setDescription(const QString& description) { m_description = description; }
    void setDueDate(const QDateTime& dueDate) { m_dueDate = dueDate; }
    void setReminderDate(const QDateTime& reminderDate) { m_reminderDate = reminderDate; }
    void setCompleted(bool completed) { m_isCompleted = completed; }
    QList<SubTask>& subTasks() { return m_subTasks; } // �ṩһ����const���ð汾�Թ��޸�
    void setCompletionDate(const QDateTime& completionDate) { m_completionDate = completionDate; }



    QJsonObject toJson() const;
    static TodoItem fromJson(const QJsonObject& json);

private:
    QUuid m_id;
    QString m_title;
    QString m_description; // �������������ע��

    QDateTime m_creationDate;
    QDateTime m_dueDate;
    QDateTime m_reminderDate;

    bool m_isCompleted;
    QList<SubTask> m_subTasks;
    QDateTime m_completionDate;
};
