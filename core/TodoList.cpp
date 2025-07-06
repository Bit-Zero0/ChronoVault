#include "core/TodoList.h"

#include <QJsonArray>


QJsonObject TodoList::toJson() const {
    QJsonObject json;
    json["id"] = id.toString(QUuid::WithoutBraces);
    json["name"] = name;

    QJsonArray itemsArray;
    for (const TodoItem& item : items) {
        itemsArray.append(item.toJson());
    }
    json["items"] = itemsArray;

    return json;
}

TodoList TodoList::fromJson(const QJsonObject& json) {
    TodoList list(json["name"].toString());
    list.id = QUuid(json["id"].toString());

    if (json.contains("items") && json["items"].isArray()) {
        QJsonArray itemsArray = json["items"].toArray();
        for (const QJsonValue& value : itemsArray) {
            list.items.append(TodoItem::fromJson(value.toObject()));
        }
    }

    return list;
}
