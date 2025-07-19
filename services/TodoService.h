#pragma once
#include <QObject>
#include <QList>
#include <QtConcurrent>

#include "core/TodoList.h"

QT_BEGIN_NAMESPACE
class QTimer; // 前向声明 QTimer
class QSystemTrayIcon; // 前向声明 QSystemTrayIcon
QT_END_NAMESPACE

class TodoService : public QObject {
    Q_OBJECT


public:
    // 获取单例实例
    static TodoService* instance();

    // 接口
    const QList<TodoList>& getAllLists() const;

    QUuid  addList(const QString& name);
    QUuid addList(const TodoList& list);
    bool deleteList(const QUuid& listId);
    bool updateListName(const QUuid& listId, const QString& newName);

    QUuid findOrCreateInboxList();
    bool addTodoToList(const QUuid& listId, const TodoItem& todo);
    bool deleteTodoFromList(const QUuid& listId, const QUuid& todoId);
    bool updateTodoInList(const QUuid& listId, const TodoItem& updatedTodo);


    bool updateSubTask(const QUuid& listId, const QUuid& todoId, const SubTask& updatedSubTask);
    bool deleteSubTask(const QUuid& listId, const QUuid& todoId, const QUuid& subTaskId);
    bool promoteSubTaskToTodo(const QUuid& listId, const QUuid& todoId, const QUuid& subTaskId);


    TodoItem* findTodoById(const QUuid& listId, const QUuid& todoId);
    TodoList* findListById(const QUuid& listId);

    void setTrayIcon(QSystemTrayIcon* trayIcon);


signals:
    // 当数据模型发生重大变化时，发射这些信号
    void listsChanged(); // 列表本身（增、删、改名）发生变化
    void tasksChanged(const QUuid& listId); // 某个列表中的任务发生变化

public slots:
    void checkReminders();
    void onSnoozeRequested(const QUuid& taskId, int minutes);
    void onDismissRequested(const QUuid& taskId);
    void onNotificationClosed(const QUuid& taskId);

private:
    explicit TodoService(QObject* parent = nullptr);
    ~TodoService();

    // 禁用拷贝和赋值
    TodoService(const TodoService&) = delete;
    TodoService& operator=(const TodoService&) = delete;


    // 数据存储
    QList<TodoList> m_lists;

//    一个线程安全的、用于在后台保存数据的新私有函数
    void saveDataInBackground(const QList<TodoList> listsToSave) const;


    void saveData() const;
    void loadData();
    QString m_savePath;


    void loadInitialData(); // 用于加载一些测试数据

    QTimer* m_reminderTimer; // 定时器成员
    QSystemTrayIcon* m_trayIcon; // 新增托盘图标指针
    QSet<QUuid> m_activeNotifications;// 激活锁，用于记录正在显示的通知
};
