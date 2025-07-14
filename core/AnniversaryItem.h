#pragma once

#include <QUuid>
#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>

#include "core/Moment.h"


// ... ö�ٶ��屣�ֲ��� ...
enum class AnniversaryEventType { Countdown, Anniversary };
enum class AnniversaryRecurrence { None, Yearly, Monthly };

//=============================================================================
//  ��������� AnniversaryItem �ࡿ
//=============================================================================
class AnniversaryItem {
public:
    explicit AnniversaryItem(const QString& title = "")
        : m_id(QUuid::createUuid()),
        m_title(title),
        m_isAddedToTodo(false)
    {
    }

    AnniversaryItem(const AnniversaryItem& other) = default;
    AnniversaryItem& operator=(const AnniversaryItem& other) = default;
    AnniversaryItem(AnniversaryItem&& other) noexcept = default;
    AnniversaryItem& operator=(AnniversaryItem&& other) noexcept = default;


    // --- ��������������ȫ���е� Getter �� Setter ���� ---
    QUuid id() const { return m_id; }
    QString title() const { return m_title; }
    void setTitle(const QString& title) { m_title = title; }

    AnniversaryEventType eventType() const { return m_eventType; }
    void setEventType(AnniversaryEventType type) { m_eventType = type; }

    QDateTime targetDateTime() const { return m_targetDateTime; }
    void setTargetDateTime(const QDateTime& dt) { m_targetDateTime = dt; }

    QDate originalDate() const { return m_originalDate; }
    void setOriginalDate(const QDate& date) { m_originalDate = date; }

    AnniversaryRecurrence recurrence() const { return m_recurrence; }
    void setRecurrence(AnniversaryRecurrence recurrence) { m_recurrence = recurrence; }

    QDateTime reminderDateTime() const { return m_reminderDateTime; }
    void setReminderDateTime(const QDateTime& dt) { m_reminderDateTime = dt; }

    bool isAddedToTodo() const { return m_isAddedToTodo; }
    void setAddedToTodo(bool added) { m_isAddedToTodo = added; }

    const QList<Moment>& moments() const { return m_moments; }
    QList<Moment>& moments() { return m_moments; }           // �����޸ķ���
    void addMoment(const Moment& moment) { m_moments.append(moment); }
    // --- -------------------------------------------- ---

    QString category() const { return m_category; }
    void setCategory(const QString& category) { m_category = category; }


    // --- ���ݳ־û����� (Ϊδ����׼��) ---
    QJsonObject toJson() const {
        QJsonObject json;
        json["id"] = m_id.toString(QUuid::WithoutBraces);
        json["title"] = m_title;
        json["category"] = m_category;
        json["eventType"] = static_cast<int>(m_eventType);
        json["targetDateTime"] = m_targetDateTime.toString(Qt::ISODate);
        json["originalDate"] = m_originalDate.toString(Qt::ISODate);
        json["recurrence"] = static_cast<int>(m_recurrence);
        json["reminderDateTime"] = m_reminderDateTime.toString(Qt::ISODate);
        json["isAddedToTodo"] = m_isAddedToTodo;

        // ���������� moments �б����л�Ϊһ��JSON����
        QJsonArray momentsArray;
        for (const auto& moment : m_moments) {
            momentsArray.append(moment.toJson());
        }
        json["moments"] = momentsArray;
        return json;
    }

    static AnniversaryItem fromJson(const QJsonObject& json) {
        AnniversaryItem item;
        item.m_id = QUuid(json.value("id").toString());
        if (item.m_id.isNull()) {
            item.m_id = QUuid::createUuid();
        }
        item.m_title = json.value("title").toString();
        item.m_category = json.value("category").toString();
        item.m_eventType = static_cast<AnniversaryEventType>(json.value("eventType").toInt());
        item.m_targetDateTime = QDateTime::fromString(json.value("targetDateTime").toString(), Qt::ISODate);
        item.m_originalDate = QDate::fromString(json.value("originalDate").toString(), Qt::ISODate);
        item.m_recurrence = static_cast<AnniversaryRecurrence>(json.value("recurrence").toInt());
        item.m_reminderDateTime = QDateTime::fromString(json.value("reminderDateTime").toString(), Qt::ISODate);
        item.m_isAddedToTodo = json.value("isAddedToTodo").toBool();

        // ���������߼���
        if (json.contains("moments") && json["moments"].isArray()) {
            QJsonArray momentsArray = json["moments"].toArray();
            for (const auto& momentValue : momentsArray) {
                // �������е�ÿһ��JSON����ת���� Moment ���󣬲���ӵ��б���
                item.m_moments.append(Moment::fromJson(momentValue.toObject()));
            }
        }
        return item;
    }

private:
    QUuid m_id;
    QString m_title;
    AnniversaryEventType m_eventType;
    QDateTime m_targetDateTime;
    QDate m_originalDate;
    AnniversaryRecurrence m_recurrence;
    QDateTime m_reminderDateTime;
    bool m_isAddedToTodo;
    QList<Moment> m_moments;
    QString m_category;
};
