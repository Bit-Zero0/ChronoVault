#include "gui/MainWindow.h"
#include "gui/TodoListItemWidget.h"
#include "gui/TaskDetailWidget.h"
#include "services/TodoService.h"
#include "gui/TodoListNameWidget.h"
#include "gui/CompletedHeaderWidget.h"

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
    m_isCompletedSectionExpanded = false;
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
    connect(m_listSelectionWidget, &QListWidget::currentItemChanged, this, &MainWindow::onCurrentListChanged);
    connect(m_listSelectionWidget, &QListWidget::customContextMenuRequested, this, &MainWindow::showListContextMenu);
    connect(m_addNewListButton, &QToolButton::clicked, this, &MainWindow::onAddNewList);
    connect(m_taskItemsWidget, &QListWidget::itemSelectionChanged, this, &MainWindow::onTaskSelectionChanged);
    connect(m_taskItemsWidget, &QListWidget::customContextMenuRequested, this, &MainWindow::showTaskContextMenu);
    connect(m_taskItemsWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::handleTaskDoubleClick);
    connect(m_addTodoLineEdit, &QLineEdit::returnPressed, this, &MainWindow::onAddNewTodo);

    // 连接所有来自详情页的信号
    connect(m_taskDetailWidget, &TaskDetailWidget::taskUpdated, this, &MainWindow::handleTaskUpdate);
    connect(m_taskDetailWidget, &TaskDetailWidget::taskDeleted, this, &MainWindow::handleTaskDelete);
    connect(m_taskDetailWidget, &TaskDetailWidget::addSubTask, this, &MainWindow::handleAddSubTask);
    connect(m_taskDetailWidget, &TaskDetailWidget::subTaskStateChanged, this, &MainWindow::handleSubTaskStateChange);
    connect(m_taskDetailWidget, &TaskDetailWidget::subTaskUpdated, this, &MainWindow::handleSubTaskUpdate);
    connect(m_taskDetailWidget, &TaskDetailWidget::subTaskDeleted, this, &MainWindow::handleSubTaskDelete);
    connect(m_taskDetailWidget, &TaskDetailWidget::subTaskPromoted, this, &MainWindow::handleSubTaskPromote);
    connect(m_taskDetailWidget, &TaskDetailWidget::closeRequested, this, &MainWindow::onDetailCloseRequested);
    connect(m_taskDetailWidget, &TaskDetailWidget::dueDateChanged, this, &MainWindow::handleDueDateChange);
    connect(m_taskDetailWidget, &TaskDetailWidget::reminderDateChanged, this, &MainWindow::handleReminderDateChange);

    // 连接来自服务层的信号
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
    m_isCompletedSectionExpanded = false; // <-- 切换列表时总是折叠

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
    // 【优化】在进行大量更新前，禁用控件的自动重绘，这是性能提升的关键！
    m_taskItemsWidget->setUpdatesEnabled(false);

    m_taskItemsWidget->clear();

    auto* currentList = m_todoService->findListById(listId);
    if (!currentList) {
        m_taskItemsWidget->setUpdatesEnabled(true); // 【优化】别忘了在函数退出前恢复重绘
        return;
    }

    // 1. 分割任务
    QList<TodoItem> activeTasks;
    QList<TodoItem> completedTasks;
    for (const auto& task : currentList->items) {
        if (task.isCompleted()) {
            completedTasks.append(task);
        } else {
            activeTasks.append(task);
        }
    }

    // 2. 添加未完成的任务
    for (const auto& task : activeTasks) {
        TodoListItemWidget *itemWidget = new TodoListItemWidget(task, this);
        QListWidgetItem *listItem = new QListWidgetItem(m_taskItemsWidget);
        listItem->setSizeHint(itemWidget->sizeHint());
        m_taskItemsWidget->addItem(listItem);
        m_taskItemsWidget->setItemWidget(listItem, itemWidget);
        connect(itemWidget, &TodoListItemWidget::taskUpdated, this, &MainWindow::onTaskUpdated);
        connect(itemWidget, &TodoListItemWidget::taskTitleChanged, this, &MainWindow::handleTaskTitleChange);
    }

    // 3. 如果有已完成的任务，则添加分组头和任务项
    if (!completedTasks.isEmpty()) {
        std::sort(completedTasks.begin(), completedTasks.end(), [](const TodoItem& a, const TodoItem& b){
            return a.completionDate() > b.completionDate();
        });

        QListWidgetItem* headerItem = new QListWidgetItem(m_taskItemsWidget);
        CompletedHeaderWidget* headerWidget = new CompletedHeaderWidget(completedTasks.count(), m_isCompletedSectionExpanded, this);
        headerItem->setSizeHint(headerWidget->sizeHint());
        headerItem->setFlags(headerItem->flags() & ~Qt::ItemIsSelectable);
        m_taskItemsWidget->addItem(headerItem);
        m_taskItemsWidget->setItemWidget(headerItem, headerWidget);

        connect(headerWidget, &CompletedHeaderWidget::toggleExpansion, this, [this, listId](){
            m_isCompletedSectionExpanded = !m_isCompletedSectionExpanded;
            displayTasksForList(listId);
        });

        if (m_isCompletedSectionExpanded) {
            for (const auto& task : completedTasks) {
                TodoListItemWidget *itemWidget = new TodoListItemWidget(task, this);
                QListWidgetItem *listItem = new QListWidgetItem(m_taskItemsWidget);
                listItem->setSizeHint(itemWidget->sizeHint());
                m_taskItemsWidget->addItem(listItem);
                m_taskItemsWidget->setItemWidget(listItem, itemWidget);
                connect(itemWidget, &TodoListItemWidget::taskUpdated, this, &MainWindow::onTaskUpdated);
                connect(itemWidget, &TodoListItemWidget::taskTitleChanged, this, &MainWindow::handleTaskTitleChange);
            }
        }
    }

    // 【优化】在所有项都添加完毕后，重新启用控件更新，并强制进行一次刷新
    m_taskItemsWidget->setUpdatesEnabled(true);
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
    // 1. 获取当前选中的列表项和它的ID
    QListWidgetItem* currentItem = m_listSelectionWidget->currentItem();
    if (!currentItem) return;

    QUuid listId = currentItem->data(Qt::UserRole).toUuid();
    if (listId.isNull()) return;

    // 2. 弹出确认对话框
    auto reply = QMessageBox::warning(this, "确认删除", "确定要删除此列表吗？\n此操作将永久删除列表及其包含的所有任务。", QMessageBox::Yes | QMessageBox::Cancel);

    if (reply == QMessageBox::Yes) {
        // 3. 【重要】在删除前，记录下当前项的行号
        int deletedIndex = m_listSelectionWidget->row(currentItem);

        // 4. 调用服务层删除数据 (这将自动触发 refreshListView)
        if (m_todoService->deleteList(listId)) {

            // 5. 【重要】在列表刷新后，计算并设置新的选中项
            int count = m_listSelectionWidget->count();
            if (count > 0) {
                // 确定新的选中行号
                int newIndex = (deletedIndex > 0) ? (deletedIndex - 1) : 0;
                m_listSelectionWidget->setCurrentRow(newIndex);
            } else {
                // 如果列表被删空了，就清理右侧UI
                m_taskItemsWidget->clear();
                m_currentListTitleLabel->setText("请选择一个列表");
                m_rightSideSplitter->setSizes({1, 0});
            }
        }
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
    if (!currentListItem || currentListItem->data(Qt::UserRole).toUuid() != listId) {
        return;
    }

    // 1. 获取刷新前，详情页正在显示的任务ID
    QUuid detailTaskId = m_taskDetailWidget->getCurrentTaskId();

    // 2. 正常重绘任务列表
    displayTasksForList(listId);

    // 3. 检查详情页是否需要同步刷新
    if (!detailTaskId.isNull()) {
        // 从服务层获取该任务的最新数据
        if (auto* freshTask = m_todoService->findTodoById(listId, detailTaskId)) {
            // 如果任务仍然存在，用最新数据强制刷新详情页
            m_taskDetailWidget->displayTask(*freshTask);
        } else {
            // 如果任务已经被删除，隐藏详情页
            m_rightSideSplitter->setSizes({1, 0});
        }
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

void MainWindow::handleDueDateChange(const QUuid& taskId, const QDateTime& dueDate) {
    QUuid listId = getCurrentListId();
    if (auto* task = m_todoService->findTodoById(listId, taskId)) {
        TodoItem updatedTask = *task;
        updatedTask.setDueDate(dueDate);
        m_todoService->updateTodoInList(listId, updatedTask);
    }
}


void MainWindow::handleReminderDateChange(const QUuid& taskId, const QDateTime& reminderDate) {
    QUuid listId = getCurrentListId();
    if (auto* task = m_todoService->findTodoById(listId, taskId)) {
        TodoItem updatedTask = *task;
        updatedTask.setReminderDate(reminderDate);

        // 调用通用的更新服务
        m_todoService->updateTodoInList(listId, updatedTask);
    }
}
