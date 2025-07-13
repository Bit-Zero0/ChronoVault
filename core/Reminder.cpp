#include "core/Reminder.h"

void Reminder::calculateNext() {
    if (!m_isActive || m_intervalType == ReminderIntervalType::None) {
        m_isActive = false; // 对于一次性提醒或手动禁用的，触发后就失效
        return;
    }
    switch (m_intervalType) {
    case ReminderIntervalType::Minutes:
        m_nextReminderTime = m_nextReminderTime.addSecs(m_intervalValue * 60);
        break;
    case ReminderIntervalType::Hours:
        m_nextReminderTime = m_nextReminderTime.addSecs(m_intervalValue * 3600);
        break;
    case ReminderIntervalType::Days:
        m_nextReminderTime = m_nextReminderTime.addDays(m_intervalValue);
        break;
    default: break;
    }
}

QJsonObject Reminder::toJson() const {
    QJsonObject json;
    json["isActive"] = m_isActive;
    json["nextReminderTime"] = m_nextReminderTime.toString(Qt::ISODate);
    json["intervalType"] = static_cast<int>(m_intervalType);
    json["intervalValue"] = m_intervalValue;
    return json;
}

Reminder Reminder::fromJson(const QJsonObject& json) {
    Reminder r;
    r.m_isActive = json["isActive"].toBool();
    r.m_nextReminderTime = QDateTime::fromString(json["nextReminderTime"].toString(), Qt::ISODate);
    r.m_intervalType = static_cast<ReminderIntervalType>(json["intervalType"].toInt());
    r.m_intervalValue = json["intervalValue"].toInt();
    return r;
}
