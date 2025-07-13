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
    m_reminderTimer->start(100);
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

QUuid TodoService::addList(const QString& name) {
    if (name.isEmpty()) {
        return QUuid(); // 如果名称为空，返回一个无效的ID
    }
    TodoList newList(name);
    m_lists.append(newList);
    emit listsChanged();
    saveData();
    return newList.id; // 返回新创建列表的ID
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

//void TodoService::saveData() const {
//    QJsonArray listsArray;
//    for (const auto& list : m_lists) {
//        listsArray.append(list.toJson());
//    }
//    QJsonDocument doc(listsArray);
//    QFile file(m_savePath);
//    if (!file.open(QIODevice::WriteOnly)) {
//        qWarning("Couldn't open save file.");
//        return;
//    }
//    file.write(doc.toJson());
//    file.close();
//}

void TodoService::saveData() const {
    // 【核心修改】同样使用Lambda表达式
    QtConcurrent::run([this, lists = this->m_lists] {
        this->saveDataInBackground(lists);
    });
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

    QDateTime now = QDateTime::currentDateTime();
    bool dataChanged = false; // 标记是否有任务的提醒状态需要保存

    for (auto& list : m_lists) {
        for (auto& task : list.items) {
            // 获取任务的提醒对象
            Reminder reminder = task.reminder();

            // 检查提醒是否激活，并且时间已到
            if (reminder.isActive() && reminder.nextReminderTime() <= now && !task.isCompleted()) {
                qDebug() << "Todo Reminder Triggered:" << task.title();
                m_trayIcon->showMessage(
                    tr("ChronoVault 待办提醒"),
                    task.title(),
                    QSystemTrayIcon::Information,
                    5000
                    );

                // 【核心升级】让 Reminder 对象自己计算下一次提醒时间
                // 如果是单次提醒，此函数会自动将其设置为不再激活
                reminder.calculateNext();

                // 将更新后的 Reminder 对象设置回任务中
                task.setReminder(reminder);
                dataChanged = true;
            }
        }
    }

    // 如果有任何提醒状态被更新，则保存数据
    if (dataChanged) {
        saveData();
    }
}


void TodoService::saveDataInBackground(const QList<TodoList> listsToSave) const {
    qDebug() << "[BG Save] Starting to save data in background thread...";
    QJsonArray listsArray;
    for (const auto& list : listsToSave) {
        listsArray.append(list.toJson());
    }
    QJsonDocument doc(listsArray);
    QFile file(m_savePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "[BG Save] Couldn't open save file in background thread.";
        return;
    }
    file.write(doc.toJson());
    file.close();
    qDebug() << "[BG Save] Finished saving data in background thread.";
}


QUuid TodoService::findOrCreateInboxList()
{
    const QString inboxName = tr("收件箱");
    // 1. 尝试查找已存在的“收件箱”
    for (const auto& list : m_lists) {
        if (list.name == inboxName) {
            return list.id;
        }
    }
    // 2. 如果没找到，则创建一个新的
    qDebug() << "Inbox not found. Creating a new one.";
    TodoList inbox(inboxName);
    return addList(inbox); // addList 内部会自动保存和发射信号
}
