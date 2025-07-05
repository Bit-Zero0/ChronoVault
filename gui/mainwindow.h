#pragma once
#include <QMainWindow>
#include "core/TodoItem.h"

class TodoService;
class TaskDetailWidget;

QT_BEGIN_NAMESPACE
class QListWidget;
class QListWidgetItem;
class QSplitter;
class QLineEdit;
class QLabel;
class QToolButton;
class QMenu;
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // -- 响应用户交互的槽函数 --
    void onCurrentListChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void onTaskSelectionChanged();
    void onAddNewList();
    void onAddNewTodo();
    void showListContextMenu(const QPoint& pos);
    void showTaskContextMenu(const QPoint& pos);
    void handleTaskDoubleClick(QListWidgetItem* item);
    void onDetailCloseRequested();

    // --- 【重要】添加缺失的槽函数声明 ---
    void onRenameList();
    void onDeleteList();
    void onEditTodo();
    void onDeleteTodo();
    // ------------------------------------

    // -- 响应 Service 数据更新的槽函数 --
    void refreshListView();
    void refreshTaskView(const QUuid& listId);

    // -- 响应 Widget 内部信号的槽函数 --
    void onTaskUpdated(const QUuid& taskId, bool isCompleted);
    void handleTaskUpdate(const TodoItem& updatedTask);
    void handleTaskDelete(const QUuid& taskId);
    void handleAddSubTask(const QUuid& taskId, const QString& subTaskTitle);
    void handleSubTaskStateChange(const QUuid& taskId, const QUuid& subTaskId, bool isCompleted);


    void handleSubTaskUpdate(const QUuid& taskId, const SubTask& updatedSubTask);
    void handleSubTaskDelete(const QUuid& taskId, const QUuid& subTaskId);
    void handleSubTaskPromote(const QUuid& taskId, const QUuid& subTaskId);

    void handleTaskTitleChange(const QUuid& taskId, const QString& newTitle);
    void handleListNameChange(const QUuid& listId, const QString& newName);

private:
    // -- 私有辅助函数 --
    void setupUi();
    void setupConnections();
    void displayTasksForList(const QUuid& listId);
    TodoItem* findTodoItemFromWidget(QWidget* widget) const;
    QUuid getCurrentListId() const;

    // -- 数据服务 --
    TodoService* m_todoService;

    // -- UI 控件 --
    // 根布局
    QSplitter* m_rootSplitter;

    // 左侧面板 (列表选择区)
    QWidget* m_listPanel;
    QListWidget* m_listSelectionWidget;
    QToolButton* m_addNewListButton;

    // 右侧布局 (由另一个分割器管理)
    QSplitter* m_rightSideSplitter;

    // 右侧 -> 左边 (任务列表区)
    QWidget* m_taskPanel;
    QLabel* m_currentListTitleLabel;
    QListWidget* m_taskItemsWidget;
    QLineEdit* m_addTodoLineEdit;

    // 右侧 -> 右边 (任务详情区)
    TaskDetailWidget* m_taskDetailWidget;
};
