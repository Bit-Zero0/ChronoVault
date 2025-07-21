#pragma once

#include <QDateTime>
#include <QJsonObject>
#include <QTime>

enum class ReminderIntervalType {
    None,
    Minutes,
    Hours,
    Days
};

class Reminder {
public:
    Reminder()
        : m_isActive(false),
        m_intervalType(ReminderIntervalType::None),
        m_intervalValue(0),
        m_soundPath("default"){}

    // 【新增】明确定义拷贝赋值运算符
    Reminder& operator=(const Reminder& other) = default;

    bool isActive() const { return m_isActive; }
    void setActive(bool active) { m_isActive = active; }

    QDateTime nextReminderTime() const { return m_nextReminderTime; }
    void setNextReminderTime(const QDateTime& dt) { m_nextReminderTime = dt; }

    ReminderIntervalType intervalType() const { return m_intervalType; }
    void setIntervalType(ReminderIntervalType type) { m_intervalType = type; }

    int intervalValue() const { return m_intervalValue; }
    void setIntervalValue(int value) { m_intervalValue = value; }

    QTime baseTime() const { return m_baseTime; }
    void setBaseTime(const QTime& time) { m_baseTime = time; }
    QString soundPath() const { return m_soundPath; }
    void setSoundPath(const QString& path) { m_soundPath = path; }

    void calculateNext(); // 将实现移至cpp，保持头文件清洁

    QJsonObject toJson() const;
    static Reminder fromJson(const QJsonObject& json);

private:
    bool m_isActive;
    QDateTime m_nextReminderTime;
    QTime m_baseTime;
    ReminderIntervalType m_intervalType;
    int m_intervalValue;
    QString m_soundPath;
};
