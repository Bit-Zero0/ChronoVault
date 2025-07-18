#pragma once

#include <QMainWindow>
#include "core/Moment.h"
#include "core/TodoItem.h"
#include "core/Reminder.h"
#include "core/SubTask.h"

// Forward declarations
class TodoService;
class AnniversaryService;
class TaskDetailWidget;
class AnniversaryDetailView;
class QListWidget;
class QListWidgetItem;
class QSplitter;
class QLineEdit;
class QLabel;
class QToolButton;
class QStackedWidget;
class QButtonGroup;
class QPoint;
class QUuid;
class QDateTime;
class TodoItem;
class SubTask;
class Reminder;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);


private slots:
    // Module Switching
    void onModuleChanged(int id);

    // Todo Module Slots
    void onCurrentListChanged(QListWidgetItem* current, QListWidgetItem* previous);
    void onTaskSelectionChanged();
    void onAddNewList();
    void onAddNewTodo();
    void onDetailCloseRequested();
    void refreshListView();
    void refreshTaskView(const QUuid& listId);
    void onTaskUpdated(const QUuid& taskId, bool isCompleted);
    void handleTaskUpdate(const TodoItem& updatedTask);
    void handleTaskDelete(const QUuid& taskId);
    void handleDueDateChange(const QUuid& taskId, const QDateTime& dueDate);
    void handleTaskTitleChange(const QUuid& taskId, const QString& newTitle);
    void handleListNameChange(const QUuid& listId, const QString& newName);
    void handleAddSubTask(const QUuid& taskId, const QString& subTaskTitle);
    void handleSubTaskStateChange(const QUuid& taskId, const QUuid& subTaskId, bool isCompleted);
    void handleSubTaskUpdate(const QUuid& taskId, const SubTask& updatedSubTask);
    void handleSubTaskDelete(const QUuid& taskId, const QUuid& subTaskId);
    void handleSubTaskPromote(const QUuid& taskId, const QUuid& subTaskId);
    void showListContextMenu(const QPoint& pos);
    void onRenameList();
    void onDeleteList();
    void showTaskContextMenu(const QPoint& pos);
    void handleTaskDoubleClick(QListWidgetItem* item);
    void onEditTodo();
    void onDeleteTodo();

    // Anniversary Module Slots
    void refreshAnniversaryView();
    void refreshAnniversaryCategories();

    void onAnniversaryCategoryChanged();
    void showAnniversaryCategoryContextMenu(const QPoint& pos);
    void onAddNewAnniversaryCategory();
    void onRenameAnniversaryCategory();
    void onDeleteAnniversaryCategory();
    void onAddNewAnniversary();
    void onAnniversaryItemClicked(QListWidgetItem* item);
    void onBackFromAnniversaryDetail();
    void onAnniversaryItemDeleted(const QUuid& id);
    void onAddToTodoRequested(const QUuid& id);
    void onAddMomentRequested(const QUuid& anniversaryId);
    void handleReminderChange(const QUuid& taskId, const Reminder& reminder);
    void onAnniversaryMomentUpdated(const QUuid& anniversaryId, const Moment& moment);
    void safeRefreshAnniversaryDetail(const QUuid& anniversaryId);

    void onMomentDeleteRequested(const QUuid& anniversaryId, const QUuid& momentId);
    void onAnniversaryDataChanged(); // 【新增】用于响应数据变化的槽



private:
    void setupUi();
    void setupConnections();
    void displayTasksForList(const QUuid& listId);
    QUuid getCurrentListId() const;
    TodoItem* findTodoItemFromWidget(QWidget* widget) const;

    // Services
    TodoService* m_todoService;
    AnniversaryService* m_anniversaryService;

    // UI Widgets
    QSplitter* m_rootSplitter;
    QWidget* m_leftPanel;
    QButtonGroup* m_moduleButtonGroup;
    QStackedWidget* m_leftContentStack;
    QWidget* m_todoListPanel;
    QListWidget* m_listSelectionWidget;
    QToolButton* m_addNewListButton;
    QWidget* m_anniversaryPanel;
    QListWidget* m_anniversaryCategoryWidget;
    QToolButton* m_addAnniversaryCategoryButton;
    QToolButton* m_settingsButton;

    QStackedWidget* m_rightContentStack;
    QSplitter* m_taskViewSplitter;
    QSplitter* m_rightSideSplitter;
    QWidget* m_taskPanel;
    QLabel* m_currentListTitleLabel;
    QListWidget* m_taskItemsWidget;
    QLineEdit* m_addTodoLineEdit;
    TaskDetailWidget* m_taskDetailWidget;

    QStackedWidget* m_anniversaryContentStack;
    QWidget* m_anniversaryOverviewPanel;
    QListWidget* m_anniversaryItemsWidget;
    QToolButton* m_addAnniversaryButton;
    AnniversaryDetailView* m_anniversaryDetailView;

    // State
    bool m_isCompletedSectionExpanded;
};
