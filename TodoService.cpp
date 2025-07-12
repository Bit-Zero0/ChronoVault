#include "services/TodoService.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QDateTime>
#include <QStringLiteral>
#include <QDebug>

TodoService* TodoService::instance() {
    static TodoService service;
    return &service;
}

TodoService::TodoService(QObject *parent) : QObject(parent) {
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataDir.isEmpty()) {
        dataDir = "data";
    }
    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    m_savePath = dataDir + "/data.json";
    m_trayIcon = nullptr;

    loadData();

    m_reminderTimer = new QTimer(this);
    connect(m_reminderTimer, &QTimer::timeout, this, &TodoService::checkReminders);
    m_reminderTimer->start(1000);
}

TodoService::~TodoService() {
    saveData();
}

void TodoService::loadInitialData() {
    TodoList list1("人生清单");
    list1.items.append(TodoItem("完成 ChronoVault 开发"));
    list1.items.append(TodoItem("学习一门新乐器"));
    m_lists.append(list1);

    TodoList list2("读书计划");
    list2.items.append(TodoItem("《经济学原理 宏观》"));
    list2.items.append(TodoItem("《社会心理学》"));
    m_lists.append(list2);

    saveData(); // 初始数据也应该保存一次
}

TodoList* TodoService::findListById(const QUuid& listId) {
    for (auto& list : m_lists) {
        if (list.id == listId) {
            return &list;
        }
    }
    return nullptr;
}

TodoItem* TodoService::findTodoById(const QUuid& listId, const QUuid& todoId) {
    if (auto* list = findListById(listId)) {
        for (auto& item : list->items) {
            if (item.id() == todoId) {
                return &item;
            }
        }
    }
    return nullptr;
}

const QList<TodoList>& TodoService::getAllLists() const {
    return m_lists;
}

// --- 以下所有修改数据的函数，都在末尾调用 saveData() ---

bool TodoService::addList(const QString& name) {
    if (name.isEmpty()) return false;
    m_lists.append(TodoList(name));
    emit listsChanged();
    saveData();
    return true;
}

QUuid TodoService::addList(const TodoList& list) {
    m_lists.append(list);
    emit listsChanged();
    saveData();
    return list.id;
}

bool TodoService::deleteList(const QUuid& listId) {
    int initialCount = m_lists.count();
    m_lists.removeIf([listId](const TodoList& list) { return list.id == listId; });
    if (m_lists.count() < initialCount) {
        emit listsChanged();
        saveData();
        return true;
    }
    return false;
}

bool TodoService::updateListName(const QUuid& listId, const QString& newName) {
    if (auto* list = findListById(listId)) {
        list->name = newName;
        emit listsChanged();
        saveData();
        return true;
    }
    return false;
}

bool TodoService::addTodoToList(const QUuid& listId, const TodoItem& todo) {
    if (auto* list = findListById(listId)) {
        list->items.append(todo);
        emit tasksChanged(listId);
        saveData();
        return true;
    }
    return false;
}

bool TodoService::deleteTodoFromList(const QUuid& listId, const QUuid& todoId) {
    if (auto* list = findListById(listId)) {
        int initialCount = list->items.count();
        list->items.removeIf([todoId](const TodoItem& item){ return item.id() == todoId; });
        if (list->items.count() < initialCount) {
            emit tasksChanged(listId);
            saveData();
            return true;
        }
    }
    return false;
}

// --- 【重要】这是 updateTodoInList 唯一且正确的实现 ---
bool TodoService::updateTodoInList(const QUuid& listId, const TodoItem& updatedTodo) {
    if (auto* list = findListById(listId)) {
        for (auto& item : list->items) {
            if (item.id() == updatedTodo.id()) {
                TodoItem finalUpdate = updatedTodo;
                if (finalUpdate.isCompleted() && !item.isCompleted()) {
                    finalUpdate.setCompletionDate(QDateTime::currentDateTime());
                }
                item = finalUpdate;
                emit tasksChanged(listId);
                saveData();
                return true;
            }
        }
    }
    return false;
}

bool TodoService::updateSubTask(const QUuid& listId, const QUuid& todoId, const SubTask& updatedSubTask) {
    if (auto* todo = findTodoById(listId, todoId)) {
        for (auto& subtask : todo->subTasks()) {
            if (subtask.id == updatedSubTask.id) {
                subtask = updatedSubTask;
                emit tasksChanged(listId);
                saveData();
                return true;
            }
        }
    }
    return false;
}

bool TodoService::deleteSubTask(const QUuid& listId, const QUuid& todoId, const QUuid& subTaskId) {
    if (auto* todo = findTodoById(listId, todoId)) {
        int initialCount = todo->subTasks().count();
        todo->subTasks().removeIf([subTaskId](const SubTask& task){ return task.id == subTaskId; });
        if (todo->subTasks().count() < initialCount) {
            emit tasksChanged(listId);
            saveData();
            return true;
        }
    }
    return false;
}

bool TodoService::promoteSubTaskToTodo(const QUuid& listId, const QUuid& todoId, const QUuid& subTaskId) {
    if (auto* list = findListById(listId)) {
        if (auto* parentTodo = findTodoById(listId, todoId)) {
            SubTask subTaskToPromote;
            int initialCount = parentTodo->subTasks().count();
            parentTodo->subTasks().removeIf([subTaskId, &subTaskToPromote](const SubTask& task) {
                if (task.id == subTaskId) {
                    subTaskToPromote = task;
                    return true;
                }
                return false;
            });
            if (parentTodo->subTasks().count() < initialCount) {
                TodoItem newTodo(subTaskToPromote.title);
                list->items.append(newTodo);
                emit tasksChanged(listId);
                saveData();
                return true;
            }
        }
    }
    return false;
}

void TodoService::saveData() const {
    QJsonArray listsArray;
    for (const auto& list : m_lists) {
        listsArray.append(list.toJson());
    }
    QJsonDocument doc(listsArray);
    QFile file(m_savePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return;
    }
    file.write(doc.toJson());
    file.close();
}

void TodoService::loadData() {
    QFile file(m_savePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file, or it doesn't exist. Loading initial data.");
        loadInitialData();
        return;
    }
    QByteArray data = file.readAll();
    file.close();
    if (data.isEmpty()) {
        qWarning("Save file is empty. Loading initial data.");
        loadInitialData();
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        qWarning("Save file is not a valid JSON array. Loading initial data.");
        loadInitialData();
        return;
    }
    QJsonArray listsArray = doc.array();
    m_lists.clear();
    for (const auto& value : listsArray) {
        m_lists.append(TodoList::fromJson(value.toObject()));
    }
}

void TodoService::setTrayIcon(QSystemTrayIcon* trayIcon) {
    m_trayIcon = trayIcon;
}

void TodoService::checkReminders() {
    if (!m_trayIcon) return;

    if (!m_trayIcon || !m_trayIcon->isVisible()) {
        // 如果不可见，可以在日志中打印一个警告，然后直接返回
        qWarning() << "Tray icon is not visible, skipping reminder check.";
        return;
    }
    QDateTime now = QDateTime::currentDateTime();

    for (auto& list : m_lists) {
        for (auto& task : list.items) {
            if (task.reminderDate().isValid() && task.reminderDate() <= now && !task.isCompleted()) {
                qDebug() << ">>> TRIGGERING NOTIFICATION for task:" << task.title();
                m_trayIcon->showMessage(
                    tr(QStringLiteral("ChronoVault 任务提醒")),  // 使用tr()来正确处理中文字符串
                    task.title(),               // task.title() 是QString，本身是Unicode，没有问题
                    QSystemTrayIcon::Information,
                    5000
                    );
                task.setReminderDate(QDateTime());
                saveData();
            }
        }
    }
}
