#pragma once
#include <QObject>
#include <QList>

#include "core/TodoList.h"

class TodoService : public QObject {
    Q_OBJECT

public:
    // 获取单例实例
    static TodoService* instance();

    // 接口
    const QList<TodoList>& getAllLists() const;

    bool addList(const QString& name);
    QUuid addList(const TodoList& list); // 重载，方便未来测试
    bool deleteList(const QUuid& listId);
    bool updateListName(const QUuid& listId, const QString& newName);

    bool addTodoToList(const QUuid& listId, const TodoItem& todo);
    bool deleteTodoFromList(const QUuid& listId, const QUuid& todoId);
    bool updateTodoInList(const QUuid& listId, const TodoItem& updatedTodo);


    bool updateSubTask(const QUuid& listId, const QUuid& todoId, const SubTask& updatedSubTask);
    bool deleteSubTask(const QUuid& listId, const QUuid& todoId, const QUuid& subTaskId);
    bool promoteSubTaskToTodo(const QUuid& listId, const QUuid& todoId, const QUuid& subTaskId);


    TodoItem* findTodoById(const QUuid& listId, const QUuid& todoId);
    TodoList* findListById(const QUuid& listId);



signals:
    // 当数据模型发生重大变化时，发射这些信号
    void listsChanged(); // 列表本身（增、删、改名）发生变化
    void tasksChanged(const QUuid& listId); // 某个列表中的任务发生变化

private:
    explicit TodoService(QObject* parent = nullptr);
    ~TodoService();

    // 禁用拷贝和赋值
    TodoService(const TodoService&) = delete;
    TodoService& operator=(const TodoService&) = delete;


    // 数据存储
    QList<TodoList> m_lists;



    void saveData() const;
    void loadData();
    QString m_savePath;


    void loadInitialData(); // 用于加载一些测试数据
};
