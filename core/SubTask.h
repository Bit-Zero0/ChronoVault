#pragma once
#include <QString>
#include <QUuid>

struct SubTask {
    QUuid id;
    QString title;
    bool isCompleted;

    SubTask(const QString& title = "")
        : id(QUuid::createUuid()), title(title), isCompleted(false) {
    }
};