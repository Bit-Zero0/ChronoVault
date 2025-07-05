#include "services/TodoService.h"
#include <QDebug> // 为了调试方便

// --- 单例 ---
TodoService* TodoService::instance() {
    static TodoService service;
    return &service;
}

// --- 构造/析构 ---
TodoService::TodoService(QObject* parent) : QObject(parent) {
    loadInitialData();
}

TodoService::~TodoService() {
    // 未来在这里保存数据
}

// --- 私有函数 ---
void TodoService::loadInitialData() {
    TodoList list1("人生清单");
    list1.items.append(TodoItem("完成 ChronoVault 开发"));
    list1.items.append(TodoItem("学习一门新乐器"));
    this->addList(list1);

    TodoList list2("读书计划");
    list2.items.append(TodoItem("《经济学原理 宏观》"));
    list2.items.append(TodoItem("《社会心理学》"));
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


// --- 公共接口实现 ---
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

            // 找到并移除子任务
            parentTodo->subTasks().removeIf([subTaskId, &subTaskToPromote](const SubTask& task) {
                if (task.id == subTaskId) {
                    subTaskToPromote = task;
                    return true;
                }
                return false;
            });

            if (parentTodo->subTasks().count() < initialCount) {
                // 创建并添加新的 Todo
                TodoItem newTodo(subTaskToPromote.title);
                list->items.append(newTodo);

                // 发射信号，让UI整体刷新
                emit tasksChanged(listId);
                return true;
            }
        }
    }
    return false;
}
