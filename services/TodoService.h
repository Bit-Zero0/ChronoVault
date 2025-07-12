#pragma once
#include <QObject>
#include <QList>
#include <QtConcurrent>

#include "core/TodoList.h"

QT_BEGIN_NAMESPACE
class QTimer; // ǰ������ QTimer
class QSystemTrayIcon; // ǰ������ QSystemTrayIcon
QT_END_NAMESPACE

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

    void setTrayIcon(QSystemTrayIcon* trayIcon);


signals:
    // ������ģ�ͷ����ش�仯ʱ��������Щ�ź�
    void listsChanged(); // �б�������ɾ�������������仯
    void tasksChanged(const QUuid& listId); // ĳ���б��е��������仯

public slots:
    void checkReminders();

private:
    explicit TodoService(QObject* parent = nullptr);
    ~TodoService();

    // ���ÿ����͸�ֵ
    TodoService(const TodoService&) = delete;
    TodoService& operator=(const TodoService&) = delete;


    // ���ݴ洢
    QList<TodoList> m_lists;

//    һ���̰߳�ȫ�ġ������ں�̨�������ݵ���˽�к���
    void saveDataInBackground(const QList<TodoList> listsToSave) const;


    void saveData() const;
    void loadData();
    QString m_savePath;


    void loadInitialData(); // ���ڼ���һЩ��������

    QTimer* m_reminderTimer; // <-- ������ʱ����Ա
    QSystemTrayIcon* m_trayIcon; // <-- ��������ͼ��ָ��
};
