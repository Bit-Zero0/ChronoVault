#pragma once
#include <QString>
#include <QDateTime>
#include <QUuid>

class Anniversary {
public:
    Anniversary(QString title, QDateTime date)
        : m_id(QUuid::createUuid()),
        m_title(std::move(title)),
        m_date(std::move(date)) {}

    QUuid id() const { return m_id; }
    const QString& title() const { return m_title; }
    const QDateTime& date() const { return m_date; }

private:
    QUuid m_id;
    QString m_title;
    QDateTime m_date;
};
