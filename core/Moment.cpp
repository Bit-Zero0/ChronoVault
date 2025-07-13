#include "core/Moment.h"
#include <QJsonArray>

Moment::Moment()
    : m_id(QUuid::createUuid()),
    m_timestamp(QDateTime::currentDateTime()) {}

Moment::Moment(const QString& text)
    : m_id(QUuid::createUuid()),
    m_timestamp(QDateTime::currentDateTime()),
    m_text(text) {}

QUuid Moment::id() const { return m_id; }
QDateTime Moment::timestamp() const { return m_timestamp; }
QString Moment::text() const { return m_text; }
void Moment::setText(const QString& text) { m_text = text; }
const QStringList& Moment::imagePaths() const { return m_imagePaths; }
void Moment::addImagePath(const QString& path) { m_imagePaths.append(path); }
void Moment::clearImagePaths() { m_imagePaths.clear(); }

QJsonObject Moment::toJson() const {
    QJsonObject json;
    json["id"] = m_id.toString(QUuid::WithoutBraces);
    json["timestamp"] = m_timestamp.toString(Qt::ISODate);
    json["text"] = m_text;
    json["imagePaths"] = QJsonArray::fromStringList(m_imagePaths);
    return json;
}

Moment Moment::fromJson(const QJsonObject& json) {
    Moment moment;
    moment.m_id = QUuid(json["id"].toString());
    moment.m_timestamp = QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate);
    moment.m_text = json["text"].toString();
    if (json.contains("imagePaths") && json["imagePaths"].isArray()) {
        QJsonArray pathsArray = json["imagePaths"].toArray();
        for (const auto& pathValue : pathsArray) {
            moment.m_imagePaths.append(pathValue.toString());
        }
    }
    return moment;
}
