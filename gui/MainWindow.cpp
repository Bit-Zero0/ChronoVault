#include "gui/MainWindow.h"
#include "gui/TodoListItemWidget.h"
#include "gui/TaskDetailWidget.h"
#include "services/TodoService.h"
#include "gui/TodoListNameWidget.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QFont>
#include <QDebug>
#include <QToolButton>
#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    m_todoService = TodoService::instance();
    setupUi();
    setupConnections();

    refreshListView();
    if (m_listSelectionWidget->count() > 0) {
        m_listSelectionWidget->setCurrentRow(0);
    }
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi() {
    setWindowTitle("ChronoVault");
    resize(1024, 768);

    // --- 左侧列表面板 ---
    m_listPanel = new QWidget();
    QVBoxLayout* listLayout = new QVBoxLayout(m_listPanel);
    listLayout->setContentsMargins(0, 8, 0, 8);
    QLabel* listHeaderLabel = new QLabel("列表栏");
    listHeaderLabel->setStyleSheet("font-size: 16px; font-weight: bold; padding-left: 8px;");
    m_listSelectionWidget = new QListWidget();
    m_listSelectionWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_listSelectionWidget->setStyleSheet("QListWidget { border: none; font-size: 14px; } QListWidget::item { height: 32px; padding-left: 8px; }");
    m_addNewListButton = new QToolButton();
    m_addNewListButton->setText("＋ 新建列表");
    m_addNewListButton->setAutoRaise(true);
    m_addNewListButton->setStyleSheet("QToolButton { border: none; padding: 8px; text-align: left; }");
    listLayout->addWidget(listHeaderLabel);
    listLayout->addWidget(m_listSelectionWidget, 1);
    listLayout->addWidget(m_addNewListButton);

    // --- 右侧的任务列表面板 ---
    m_taskPanel = new QWidget();
    QVBoxLayout* taskLayout = new QVBoxLayout(m_taskPanel);
    taskLayout->setContentsMargins(15, 8, 15, 8);
    m_currentListTitleLabel = new QLabel("请选择一个列表");
    m_currentListTitleLabel->setStyleSheet("font-size: 24px; font-weight: bold;");
    m_taskItemsWidget = new QListWidget();
    m_taskItemsWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_taskItemsWidget->setSpacing(2);
    m_taskItemsWidget->setStyleSheet("QListWidget { border: none; }");
    m_addTodoLineEdit = new QLineEdit();
    m_addTodoLineEdit->setPlaceholderText("＋ 添加任务");
    m_addTodoLineEdit->setStyleSheet(
        "QLineEdit { border: 1px solid #dcdcdc; border-radius: 4px; padding: 10px; background-color: #ffffff; font-size: 14px;}"
        "QLineEdit:focus { border: 1px solid #0078d4;}"
        );
    taskLayout->addWidget(m_currentListTitleLabel);
    taskLayout->addWidget(m_taskItemsWidget, 1);
    taskLayout->addWidget(m_addTodoLineEdit);

    // --- 任务详情面板 ---
    m_taskDetailWidget = new TaskDetailWidget();

    // --- 创建嵌套分割器 ---
    m_rightSideSplitter = new QSplitter(Qt::Horizontal);
    m_rightSideSplitter->addWidget(m_taskPanel);
    m_rightSideSplitter->addWidget(m_taskDetailWidget);
    m_rightSideSplitter->setHandleWidth(1);
    m_rightSideSplitter->setStyleSheet("QSplitter::handle { background-color: #efefef; }");

    m_rootSplitter = new QSplitter(Qt::Horizontal, this);
    m_rootSplitter->addWidget(m_listPanel);
    m_rootSplitter->addWidget(m_rightSideSplitter);
    m_rootSplitter->setStretchFactor(1, 3);
    m_rootSplitter->setSizes({ 250, 750 });
    m_rootSplitter->setHandleWidth(1);

    m_rightSideSplitter->setSizes({ 1, 0 });

    setCentralWidget(m_rootSplitter);
}

void MainWindow::setupConnections() {
    // 列表选择
    connect(m_listSelectionWidget, &QListWidget::currentItemChanged, this, &MainWindow::onCurrentListChanged);
    connect(m_listSelectionWidget, &QListWidget::customContextMenuRequested, this, &MainWindow::showListContextMenu);
    connect(m_addNewListButton, &QToolButton::clicked, this, &MainWindow::onAddNewList);

    // 任务选择
    connect(m_taskItemsWidget, &QListWidget::itemSelectionChanged, this, &MainWindow::onTaskSelectionChanged);
    connect(m_taskItemsWidget, &QListWidget::customContextMenuRequested, this, &MainWindow::showTaskContextMenu);
    connect(m_taskItemsWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::handleTaskDoubleClick);
    connect(m_addTodoLineEdit, &QLineEdit::returnPressed, this, &MainWindow::onAddNewTodo);

    // 详情页信号
    connect(m_taskDetailWidget, &TaskDetailWidget::taskUpdated, this, &MainWindow::handleTaskUpdate);
    connect(m_taskDetailWidget, &TaskDetailWidget::addSubTask, this, &MainWindow::handleAddSubTask);
    connect(m_taskDetailWidget, &TaskDetailWidget::subTaskStateChanged, this, &MainWindow::handleSubTaskStateChange);
    connect(m_taskDetailWidget, &TaskDetailWidget::subTaskUpdated, this, &MainWindow::handleSubTaskUpdate);
    connect(m_taskDetailWidget, &TaskDetailWidget::closeRequested, this, &MainWindow::onDetailCloseRequested);
    // 【补上缺失的连接】
    connect(m_taskDetailWidget, &TaskDetailWidget::taskDeleted, this, &MainWindow::handleTaskDelete);
    connect(m_taskDetailWidget, &TaskDetailWidget::subTaskDeleted, this, &MainWindow::handleSubTaskDelete);
    connect(m_taskDetailWidget, &TaskDetailWidget::subTaskPromoted, this, &MainWindow::handleSubTaskPromote);

    // 服务层信号
    connect(m_todoService, &TodoService::listsChanged, this, &MainWindow::refreshListView);
    connect(m_todoService, &TodoService::tasksChanged, this, &MainWindow::refreshTaskView);
}

void MainWindow::onDetailCloseRequested() {
    // 清除任务列表的选中状态
    m_taskItemsWidget->clearSelection();
    // 隐藏详情面板
    m_rightSideSplitter->setSizes({ 1, 0 });
}


void MainWindow::onCurrentListChanged(QListWidgetItem* current, QListWidgetItem* previous) {
    Q_UNUSED(previous);
    m_rightSideSplitter->setSizes({1, 0}); // 切换主列表时总是隐藏详情页

    if (!current) {
        m_taskItemsWidget->clear();
        m_currentListTitleLabel->setText("请选择一个列表");
        return;
    }

    // 【重要】通过 item 中存储的 ID 来获取所有信息
    QUuid listId = current->data(Qt::UserRole).toUuid();
    if (listId.isNull()) return;

    // 从数据源获取列表名称，而不是从UI
    const auto& lists = m_todoService->getAllLists();
    QString listName;
    for (const auto& list : lists) {
        if (list.id == listId) {
            listName = list.name;
            break;
        }
    }

    if (!listName.isEmpty()) {
        m_currentListTitleLabel->setText(listName);
        displayTasksForList(listId);
    }
}

void MainWindow::onTaskSelectionChanged() {
    QListWidgetItem* selectedItem = m_taskItemsWidget->currentItem();
    if (!selectedItem) {
        m_rightSideSplitter->setSizes({ 1, 0 });
        return;
    }
    TodoListItemWidget* itemWidget = qobject_cast<TodoListItemWidget*>(m_taskItemsWidget->itemWidget(selectedItem));
    if (!itemWidget) return;
    TodoItem* task = m_todoService->findTodoById(getCurrentListId(), itemWidget->getTodoItem().id());
    if (task) {
        m_taskDetailWidget->displayTask(*task);
        int totalWidth = m_rightSideSplitter->width();
        m_rightSideSplitter->setSizes({ totalWidth * 2 / 3, totalWidth * 1 / 3 });
    }
}

void MainWindow::onAddNewTodo() {
    QString title = m_addTodoLineEdit->text().trimmed();
    if (title.isEmpty()) return;

    QUuid listId = getCurrentListId();
    if (listId.isNull()) return;

    m_todoService->addTodoToList(listId, TodoItem(title));
    m_addTodoLineEdit->clear();
}


void MainWindow::displayTasksForList(const QUuid& listId) {
    m_taskItemsWidget->clear();
    const auto& lists = m_todoService->getAllLists();
    const TodoList* currentList = nullptr;
    for (const auto& list : lists) {
        if (list.id == listId) {
            currentList = &list;
            break;
        }
    }

    if (!currentList) return;

    for (const auto& task : currentList->items) {
        TodoListItemWidget *itemWidget = new TodoListItemWidget(task, this);
        QListWidgetItem *listItem = new QListWidgetItem(m_taskItemsWidget);
        listItem->setSizeHint(itemWidget->sizeHint());
        m_taskItemsWidget->addItem(listItem);
        m_taskItemsWidget->setItemWidget(listItem, itemWidget);

        connect(itemWidget, &TodoListItemWidget::taskUpdated, this, &MainWindow::onTaskUpdated);
        connect(itemWidget, &TodoListItemWidget::taskTitleChanged, this, &MainWindow::handleTaskTitleChange);

    }
}

void MainWindow::handleTaskDelete(const QUuid& taskId) {
    m_todoService->deleteTodoFromList(getCurrentListId(), taskId);
    m_rightSideSplitter->setSizes({1, 0});
}

QUuid MainWindow::getCurrentListId() const {
    QListWidgetItem* currentItem = m_listSelectionWidget->currentItem();
    if (currentItem) {
        return currentItem->data(Qt::UserRole).toUuid();
    }
    return QUuid();
}

TodoItem* MainWindow::findTodoItemFromWidget(QWidget* widget) const {
    if (!widget) return nullptr;
    auto* itemWidget = qobject_cast<TodoListItemWidget*>(widget);
    if (!itemWidget) return nullptr;
    QUuid listId = getCurrentListId();
    QUuid todoId = itemWidget->getTodoItem().id();
    if(listId.isNull() || todoId.isNull()) return nullptr;
    return m_todoService->findTodoById(listId, todoId);
}

void MainWindow::onAddNewList() {
    bool ok;
    QString name = QInputDialog::getText(this, "新建列表", "请输入新列表的名称:", QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        m_todoService->addList(name);
    }
}

void MainWindow::showListContextMenu(const QPoint& pos) {
    QListWidgetItem* item = m_listSelectionWidget->itemAt(pos);
    if (!item) return;
    QMenu contextMenu(this);
    contextMenu.addAction("重命名", this, &MainWindow::onRenameList);
    contextMenu.addSeparator();
    contextMenu.addAction("删除列表", this, &MainWindow::onDeleteList);
    contextMenu.exec(m_listSelectionWidget->mapToGlobal(pos));
}

void MainWindow::onDeleteList() {
    QUuid listId = getCurrentListId();
    if (listId.isNull()) return;
    auto reply = QMessageBox::warning(this, "确认删除", "确定要删除此列表吗？\n此操作将永久删除列表及其包含的所有任务。", QMessageBox::Yes | QMessageBox::Cancel);
    if (reply == QMessageBox::Yes) {
        m_todoService->deleteList(listId);
    }
}

void MainWindow::onRenameList() {
    QListWidgetItem* currentItem = m_listSelectionWidget->currentItem();
    if (!currentItem) return;

    if (auto* widget = qobject_cast<TodoListNameWidget*>(m_listSelectionWidget->itemWidget(currentItem))) {
        widget->enterEditMode();
    }
}

void MainWindow::showTaskContextMenu(const QPoint& pos) {
    // 获取右键点击位置对应的列表项
    QListWidgetItem* item = m_taskItemsWidget->itemAt(pos);
    if (!item) return;

    // 将当前右键点击的项设置为“当前项”，这对于后续获取焦点很重要
    m_taskItemsWidget->setCurrentItem(item);

    QMenu contextMenu(this);
    QAction* modifyAction = contextMenu.addAction("编辑任务...");
    contextMenu.addSeparator();
    QAction* deleteAction = contextMenu.addAction("删除任务");

    QAction* selectedAction = contextMenu.exec(m_taskItemsWidget->mapToGlobal(pos));

    if (selectedAction == modifyAction) {
        // 不再调用 onEditTodo() 弹窗，而是直接触发原地编辑
        if (auto* widget = qobject_cast<TodoListItemWidget*>(m_taskItemsWidget->itemWidget(item))) {
            widget->enterEditMode();
        }
    } else if (selectedAction == deleteAction) {
        // 调用我们已有的删除逻辑
        onDeleteTodo();
    }
}

void MainWindow::handleTaskDoubleClick(QListWidgetItem* item) {
    if (!item) return;
    if (auto* widget = qobject_cast<TodoListItemWidget*>(m_taskItemsWidget->itemWidget(item))) {
        widget->enterEditMode();
    }
}

void MainWindow::onEditTodo() {
    QListWidgetItem* currentItem = m_taskItemsWidget->currentItem();
    if(!currentItem) return;
    TodoListItemWidget* itemWidget = qobject_cast<TodoListItemWidget*>(m_taskItemsWidget->itemWidget(currentItem));
    if(!itemWidget) return;
    TodoItem originalItem = itemWidget->getTodoItem();
    bool ok;
    QString newTitle = QInputDialog::getText(this, "编辑任务", "任务标题:", QLineEdit::Normal, originalItem.title(), &ok);
    if (ok && !newTitle.isEmpty()) {
        TodoItem updatedItem = originalItem;
        updatedItem.setTitle(newTitle);
        m_todoService->updateTodoInList(getCurrentListId(), updatedItem);
    }
}

void MainWindow::onDeleteTodo() {
    QListWidgetItem* currentItem = m_taskItemsWidget->currentItem();
    if(!currentItem) return;
    TodoListItemWidget* itemWidget = qobject_cast<TodoListItemWidget*>(m_taskItemsWidget->itemWidget(currentItem));
    if(!itemWidget) return;
    QUuid listId = getCurrentListId();
    QUuid todoId = itemWidget->getTodoItem().id();
    if (listId.isNull() || todoId.isNull()) return;
    m_todoService->deleteTodoFromList(listId, todoId);
}

void MainWindow::onTaskUpdated(const QUuid& taskId, bool isCompleted) {
    QUuid listId = getCurrentListId();
    if (listId.isNull()) {
        qWarning() << "Could not update task: No list selected.";
        return;
    }
    if (auto* todo = m_todoService->findTodoById(listId, taskId)) {
        TodoItem updatedTodo = *todo;
        updatedTodo.setCompleted(isCompleted);
        m_todoService->updateTodoInList(listId, updatedTodo);
        qDebug() << "Task status updated via service:" << taskId;
    } else {
        qWarning() << "Could not find task to update with ID:" << taskId;
    }
}

void MainWindow::handleTaskUpdate(const TodoItem& updatedTask) {
    m_todoService->updateTodoInList(getCurrentListId(), updatedTask);
}

void MainWindow::handleAddSubTask(const QUuid& taskId, const QString& subTaskTitle) {
    TodoItem* task = m_todoService->findTodoById(getCurrentListId(), taskId);
    if (task) {
        task->subTasks().append(SubTask(subTaskTitle));
        m_todoService->updateTodoInList(getCurrentListId(), *task);
    }
}

void MainWindow::handleSubTaskStateChange(const QUuid& taskId, const QUuid& subTaskId, bool isCompleted) {
    TodoItem* task = m_todoService->findTodoById(getCurrentListId(), taskId);
    if (task) {
        for (auto& subtask : task->subTasks()) {
            if (subtask.id == subTaskId) {
                subtask.isCompleted = isCompleted;
                break;
            }
        }
        m_todoService->updateTodoInList(getCurrentListId(), *task);
    }
}

void MainWindow::refreshListView() {
    disconnect(m_listSelectionWidget, &QListWidget::currentItemChanged, this, &MainWindow::onCurrentListChanged);
    m_listSelectionWidget->clear();
    const auto& lists = m_todoService->getAllLists();
    for (const auto& list : lists) {
        QListWidgetItem* item = new QListWidgetItem(m_listSelectionWidget);
        TodoListNameWidget* widget = new TodoListNameWidget(list.id, list.name, m_listSelectionWidget);

        // 【重要】将列表ID存回 item 中
        item->setData(Qt::UserRole, QVariant::fromValue(list.id));

        item->setSizeHint(widget->sizeHint());
        m_listSelectionWidget->addItem(item);
        m_listSelectionWidget->setItemWidget(item, widget);

        connect(widget, &TodoListNameWidget::listNameChanged, this, &MainWindow::handleListNameChange);
    }
    connect(m_listSelectionWidget, &QListWidget::currentItemChanged, this, &MainWindow::onCurrentListChanged);
}

void MainWindow::refreshTaskView(const QUuid& listId) {
    QListWidgetItem* currentListItem = m_listSelectionWidget->currentItem();
    if (!currentListItem) return;
    QUuid currentListId = currentListItem->data(Qt::UserRole).toUuid();
    if (currentListId == listId) {
        // 保存当前选中的任务ID，以便刷新后恢复
        QListWidgetItem* currentTaskItem = m_taskItemsWidget->currentItem();
        QUuid selectedTaskId;
        if(currentTaskItem) {
            if(auto* widget = qobject_cast<TodoListItemWidget*>(m_taskItemsWidget->itemWidget(currentTaskItem))) {
                selectedTaskId = widget->getTodoItem().id();
            }
        }

        displayTasksForList(listId);

        // 刷新后，尝试重新选中之前的任务
        if(!selectedTaskId.isNull()) {
            for(int i = 0; i < m_taskItemsWidget->count(); ++i) {
                QListWidgetItem* item = m_taskItemsWidget->item(i);
                if(auto* widget = qobject_cast<TodoListItemWidget*>(m_taskItemsWidget->itemWidget(item))) {
                    if(widget->getTodoItem().id() == selectedTaskId) {
                        m_taskItemsWidget->setCurrentItem(item);
                        break;
                    }
                }
            }
        }
        // onTaskSelectionChanged(); // 不再需要手动调用，因为setCurrentItem会触发信号
    }
}


void MainWindow::handleSubTaskUpdate(const QUuid& taskId, const SubTask& updatedSubTask) {
    m_todoService->updateSubTask(getCurrentListId(), taskId, updatedSubTask);
}

void MainWindow::handleSubTaskDelete(const QUuid& taskId, const QUuid& subTaskId) {
    m_todoService->deleteSubTask(getCurrentListId(), taskId, subTaskId);
}

void MainWindow::handleSubTaskPromote(const QUuid& taskId, const QUuid& subTaskId) {
    m_todoService->promoteSubTaskToTodo(getCurrentListId(), taskId, subTaskId);
}

void MainWindow::handleTaskTitleChange(const QUuid& taskId, const QString& newTitle) {
    QUuid listId = getCurrentListId();
    if (auto* task = m_todoService->findTodoById(listId, taskId)) {
        TodoItem updatedTask = *task;
        updatedTask.setTitle(newTitle);
        m_todoService->updateTodoInList(listId, updatedTask);
    }
}

void MainWindow::handleListNameChange(const QUuid& listId, const QString& newName) {
    m_todoService->updateListName(listId, newName);
}
