#include "services/TodoService.h"
#include "gui/NotificationWidget.h"
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
#include <QApplication>
#include <QScreen>

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

    m_activeNotifications = QSet<QUuid>();
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

    // 【核心修正】如果文件不存在，我们只打印一条信息，然后直接返回，不再加载初始数据
    if (!file.exists()) {
        qWarning("Todo data file not found. Starting with an empty list.");
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Couldn't open Todo save file:" << m_savePath;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    if (data.isEmpty()) {
        qWarning("Todo data file is empty. Starting with an empty list.");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        qWarning("Todo data file is not a valid JSON array. Starting with an empty list.");
        return;
    }

    m_lists.clear();
    QJsonArray listsArray = doc.array();
    for (const auto& value : listsArray) {
        m_lists.append(TodoList::fromJson(value.toObject()));
    }
}

void TodoService::setTrayIcon(QSystemTrayIcon* trayIcon) {
    m_trayIcon = trayIcon;
}






// services/TodoService.cpp

// services/TodoService.cpp

void TodoService::checkReminders() {
    if (!m_trayIcon) return; // 安全检查

    for (auto& list : m_lists) {
        for (auto& task : list.items) {
            // 只读取 reminder 数据，绝不修改
            const Reminder& reminder = task.reminder();

            if (reminder.isActive() && reminder.nextReminderTime() <= QDateTime::currentDateTime() && !task.isCompleted()) {

                // 使用我们之前实现的“激活锁”，防止为同一个提醒创建多个通知
                if (m_activeNotifications.contains(task.id())) {
                    continue;
                }
                m_activeNotifications.insert(task.id());
                qDebug() << "Lock acquired for task:" << task.id();

                auto* notification = new NotificationWidget(task.id(), "ChronoVault 待办提醒", task.title());

                // 【重要】连接 closed 信号，用于在通知关闭时“解锁”
                connect(notification, &NotificationWidget::closed, this, [this, taskId = task.id()]() {
                    onNotificationClosed(taskId);
                });
                connect(notification, &NotificationWidget::snoozeRequested, this, &TodoService::onSnoozeRequested);
                connect(notification, &NotificationWidget::dismissRequested, this, &TodoService::onDismissRequested);

                // 计算位置并显示
                QScreen *screen = QGuiApplication::primaryScreen();
                if (!screen) {
                    notification->show(); // Fallback
                    continue;
                }

                QRect availableGeometry = screen->availableGeometry();
                QSize notificationSize = notification->sizeHint(); // 获取窗口的理想尺寸

                // 使用理想尺寸来计算右下角的位置
                notification->move(availableGeometry.right() - notificationSize.width() - 15,
                                   availableGeometry.bottom() - notificationSize.height() - 15);

                notification->show();

                // 【核心修正】删除下面这两行导致数据竞争的致命代码
                // task.setReminder(reminder);
                // saveData();
            }
        }
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

// 实现“稍后提醒”的逻辑
void TodoService::onSnoozeRequested(const QUuid& taskId, int minutes)
{
    qDebug() << "Snooze requested for task:" << taskId << "for" << minutes << "minutes.";
    for (auto& list : m_lists) {
        for (auto& task : list.items) {
            if (task.id() == taskId) {
                Reminder reminder = task.reminder();
                // 将提醒时间设置为 N 分钟后
                reminder.setNextReminderTime(QDateTime::currentDateTime().addSecs(minutes * 60));
                task.setReminder(reminder);
                saveData();
                return;
            }
        }
    }
}

// 实现“不再提醒”的逻辑
void TodoService::onDismissRequested(const QUuid& taskId)
{
    qDebug() << "Dismiss requested for task:" << taskId;
    for (auto& list : m_lists) {
        for (auto& task : list.items) {
            if (task.id() == taskId) {
                Reminder reminder = task.reminder();
                // 将提醒设置为非激活状态
                reminder.setActive(false);
                task.setReminder(reminder);
                saveData();
                return;
            }
        }
    }
}

void TodoService::onNotificationClosed(const QUuid& taskId)
{
    // 1. 首先，立即释放锁，这是最重要的
    m_activeNotifications.remove(taskId);
    qDebug() << "Lock released for task:" << taskId;

    // 2. 查找对应的任务，这是我们新增的核心逻辑
    for (auto& list : m_lists) {
        for (auto& task : list.items) {
            if (task.id() == taskId) {
                Reminder reminder = task.reminder();

                // 3. 检查这个提醒是否仍然处于“激活”状态。
                // 如果用户点击了“不再提醒”，onDismissRequested 会把它设为 false。
                // 如果用户点击了“稍后提醒”，onSnoozeRequested 会更新它的时间。
                // 只有当用户什么都没做，它才会保持原样。
                if (reminder.isActive() && reminder.nextReminderTime() <= QDateTime::currentDateTime()) {
                    qDebug() << "Notification for task" << taskId << "closed without action. Snoozing for default 5 minutes.";

                    // 4. 【默认行为】将提醒设置为5分钟后再次触发
                    reminder.setNextReminderTime(QDateTime::currentDateTime().addSecs(5 * 60));
                    task.setReminder(reminder);
                    saveData();
                }
                return; // 找到任务并处理完毕，即可退出
            }
        }
    }
}
