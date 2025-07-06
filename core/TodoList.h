#pragma once
#include <QString>
#include <QList>
#include <QUuid>
#include <QJsonObject>

#include "core/TodoItem.h"


class TodoList {
public:
    // 为了简单起见，我们暂时使用 public 成员变量
    // 在实际大型项目中，应使用 getters/setters
    QUuid id;
    QString name;
    QList<TodoItem> items;

    explicit TodoList(const QString& name = "New List")
        : id(QUuid::createUuid()), name(name) {}

    QJsonObject toJson() const;
    static TodoList fromJson(const QJsonObject& json);
};
