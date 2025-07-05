#include "services/TodoService.h"
#include <QDebug> // Ϊ�˵��Է���

// --- ���� ---
TodoService* TodoService::instance() {
    static TodoService service;
    return &service;
}

// --- ����/���� ---
TodoService::TodoService(QObject* parent) : QObject(parent) {
    loadInitialData();
}

TodoService::~TodoService() {
    // δ�������ﱣ������
}

// --- ˽�к��� ---
void TodoService::loadInitialData() {
    TodoList list1("�����嵥");
    list1.items.append(TodoItem("��� ChronoVault ����"));
    list1.items.append(TodoItem("ѧϰһ��������"));
    this->addList(list1);

    TodoList list2("����ƻ�");
    list2.items.append(TodoItem("������ѧԭ�� ��ۡ�"));
    list2.items.append(TodoItem("���������ѧ��"));
    this->addList(list2);
}

TodoList* TodoService::findListById(const QUuid& listId) {
    for (auto& list : m_lists) {
        if (list.id == listId) {
            return &list;
        }
    }
    return nullptr;
}


// --- �����ӿ�ʵ�� ---
const QList<TodoList>& TodoService::getAllLists() const {
    return m_lists;
}

bool TodoService::addList(const QString& name) {
    if (name.isEmpty()) return false;
    m_lists.append(TodoList(name));
    emit listsChanged();
    return true;
}

QUuid TodoService::addList(const TodoList& list) {
    m_lists.append(list);
    emit listsChanged();
    return list.id;
}

bool TodoService::deleteList(const QUuid& listId) {
    int initialCount = m_lists.count();
    m_lists.removeIf([listId](const TodoList& list) {
        return list.id == listId;
        });

    if (m_lists.count() < initialCount) {
        emit listsChanged();
        return true;
    }
    return false;
}

bool TodoService::updateListName(const QUuid& listId, const QString& newName) {
    if (auto* list = findListById(listId)) {
        list->name = newName;
        emit listsChanged();
        return true;
    }
    return false;
}

bool TodoService::addTodoToList(const QUuid& listId, const TodoItem& todo) {
    if (auto* list = findListById(listId)) {
        list->items.append(todo);
        emit tasksChanged(listId);
        return true;
    }
    return false;
}

bool TodoService::deleteTodoFromList(const QUuid& listId, const QUuid& todoId) {
    if (auto* list = findListById(listId)) {
        int initialCount = list->items.count();
        list->items.removeIf([todoId](const TodoItem& item) {
            return item.id() == todoId;
            });
        if (list->items.count() < initialCount) {
            emit tasksChanged(listId);
            return true;
        }
    }
    return false;
}

bool TodoService::updateTodoInList(const QUuid& listId, const TodoItem& updatedTodo) {
    if (auto* list = findListById(listId)) {
        for (auto& item : list->items) {
            if (item.id() == updatedTodo.id()) {
                item = updatedTodo;
                emit tasksChanged(listId);
                return true;
            }
        }
    }
    return false;
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




bool TodoService::updateSubTask(const QUuid& listId, const QUuid& todoId, const SubTask& updatedSubTask) {
    if (auto* todo = findTodoById(listId, todoId)) {
        for (auto& subtask : todo->subTasks()) {
            if (subtask.id == updatedSubTask.id) {
                subtask = updatedSubTask;
                emit tasksChanged(listId);
                return true;
            }
        }
    }
    return false;
}

bool TodoService::deleteSubTask(const QUuid& listId, const QUuid& todoId, const QUuid& subTaskId) {
    if (auto* todo = findTodoById(listId, todoId)) {
        int initialCount = todo->subTasks().count();
        todo->subTasks().removeIf([subTaskId](const SubTask& task){
            return task.id == subTaskId;
        });

        if (todo->subTasks().count() < initialCount) {
            emit tasksChanged(listId);
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

            // �ҵ����Ƴ�������
            parentTodo->subTasks().removeIf([subTaskId, &subTaskToPromote](const SubTask& task) {
                if (task.id == subTaskId) {
                    subTaskToPromote = task;
                    return true;
                }
                return false;
            });

            if (parentTodo->subTasks().count() < initialCount) {
                // ����������µ� Todo
                TodoItem newTodo(subTaskToPromote.title);
                list->items.append(newTodo);

                // �����źţ���UI����ˢ��
                emit tasksChanged(listId);
                return true;
            }
        }
    }
    return false;
}
