#pragma once
#include <QMainWindow>
#include "core/TodoItem.h"
#include "services/AnniversaryService.h"

class TodoService;
class TaskDetailWidget;
class AnniversaryItemWidget;
class AddAnniversaryDialog;
class ReminderSettingsDialog;
class AnniversaryDetailView;

QT_BEGIN_NAMESPACE
class QListWidget;
class QListWidgetItem;
class QSplitter;
class QLineEdit;
class QLabel;
class QToolButton;
class QMenu;
class QHBoxLayout;
class QStackedWidget;
class QButtonGroup;
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

    void onModuleChanged(int id);

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

    //void handleReminderDateChange(const QUuid& taskId, const QDateTime& reminderDate);



    void handleDueDateChange(const QUuid& taskId, const QDateTime& dueDate);

    void refreshAnniversaryView();
    void onAddNewAnniversary();
    void onAnniversaryItemDeleted(const QUuid& id);

    void onAddToTodoRequested(const QUuid& id);

    void onAnniversaryCategoryChanged();
    void showAnniversaryCategoryContextMenu(const QPoint& pos);
    void onDeleteAnniversaryCategory();
    void onAddNewAnniversaryCategory();
    void onRenameAnniversaryCategory();
    void onAddMomentRequested(const QUuid& anniversaryId);

    void onAnniversaryItemClicked(QListWidgetItem* item);
    void onBackFromAnniversaryDetail();

private:
    // -- 私有辅助函数 --
    void setupUi();
    void setupConnections();
    void displayTasksForList(const QUuid& listId);
    TodoItem* findTodoItemFromWidget(QWidget* widget) const;
    QUuid getCurrentListId() const;

    void refreshAnniversaryCategories();

    // -- 数据服务 --
    TodoService* m_todoService;
    AnniversaryService* m_anniversaryService;

    QSplitter* m_taskViewSplitter;


    // -- UI 控件 --
    // 根布局
    QSplitter* m_rootSplitter;

    // 左侧面板 (包含切换器和内容区)
    QWidget* m_leftPanel;
    QHBoxLayout* m_moduleSwitcherLayout; // 模块切换器布局
    QButtonGroup* m_moduleButtonGroup;   // 按钮组，用于管理切换
    QStackedWidget* m_leftContentStack;  // 左侧内容区 (待办列表/纪念日分类)
    QToolButton* m_settingsButton;       // 左下角设置按钮

    // 左侧 -> 待办模块
    QWidget* m_todoListPanel;            // 包含待办列表和新建按钮的面板
    QListWidget* m_listSelectionWidget;
    QToolButton* m_addNewListButton;

    // 左侧 -> 纪念日模块 (占位)
    QWidget* m_anniversaryPanel;         // 纪念日模块的面板
    QListWidget* m_anniversaryCategoryWidget; // 用于显示纪念日分类

    // 右侧布局 (与之前相同)
    QSplitter* m_rightSideSplitter;
    QWidget* m_taskPanel;
    QLabel* m_currentListTitleLabel;
    QListWidget* m_taskItemsWidget;
    QLineEdit* m_addTodoLineEdit;
    bool m_isCompletedSectionExpanded;
    TaskDetailWidget* m_taskDetailWidget;

    QStackedWidget* m_rightContentStack; // 【新增】用于管理右侧内容区

    // -- 纪念日模块的右侧UI --
    QWidget* m_anniversaryRightPanel;
    QListWidget* m_anniversaryItemsWidget;
    QToolButton* m_addAnniversaryButton;
    QToolButton* m_addAnniversaryCategoryButton;


    QStackedWidget* m_anniversaryContentStack; // 【新增】管理纪念日模块的右侧视图
    QWidget* m_anniversaryOverviewPanel;      // 【新增】概览页
    AnniversaryDetailView* m_anniversaryDetailView;



};
