#pragma once
#include <QObject>
#include <QList>

#include "core/TodoList.h"

class TodoService : public QObject {
    Q_OBJECT

public:
    // ��ȡ����ʵ��
    static TodoService* instance();

    // �ӿ�
    const QList<TodoList>& getAllLists() const;

    bool addList(const QString& name);
    QUuid addList(const TodoList& list); // ���أ�����δ������
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
    // ������ģ�ͷ����ش�仯ʱ��������Щ�ź�
    void listsChanged(); // �б�������ɾ�������������仯
    void tasksChanged(const QUuid& listId); // ĳ���б��е��������仯

private:
    explicit TodoService(QObject* parent = nullptr);
    ~TodoService();

    // ���ÿ����͸�ֵ
    TodoService(const TodoService&) = delete;
    TodoService& operator=(const TodoService&) = delete;


    // ���ݴ洢
    QList<TodoList> m_lists;



    void saveData() const;
    void loadData();
    QString m_savePath;


    void loadInitialData(); // ���ڼ���һЩ��������
};
