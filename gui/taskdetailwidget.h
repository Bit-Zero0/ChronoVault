#pragma once
#include <QWidget>
#include <QListWidgetItem>
#include "core/TodoItem.h"
#include "gui/SubTaskItemWidget.h"

// 前向声明
QT_BEGIN_NAMESPACE
class QCheckBox;
class QLineEdit;
class QTextEdit;
class QPushButton;
class QListWidget;
class QLabel;
class QToolButton;
QT_END_NAMESPACE

class TaskDetailWidget : public QWidget {
    Q_OBJECT

public:

    explicit TaskDetailWidget(QWidget *parent = nullptr);

    // 用任务数据填充UI
    void displayTask(const TodoItem& task);
    QUuid getCurrentTaskId() const;




signals:
    // 当UI控件被用户修改时，发射信号通知 MainWindow
    void taskUpdated(const TodoItem& updatedTask);
    void taskDeleted(const QUuid& taskId);
    void addSubTask(const QUuid& taskId, const QString& subTaskTitle);
    void subTaskStateChanged(const QUuid& taskId, const QUuid& subTaskId, bool isCompleted);
    void closeRequested();


    void subTaskUpdated(const QUuid& taskId, const SubTask& updatedSubTask);
    void subTaskDeleted(const QUuid& taskId, const QUuid& subTaskId);
    void subTaskPromoted(const QUuid& taskId, const QUuid& subTaskId);
    void dueDateChanged(const QUuid& taskId, const QDateTime& dueDate);



private slots:
    void onTitleEditingFinished();
    void onNotesChanged();
    void onCompletedStateChanged(int state);
    void onDeleteButtonClicked();
    void onAddSubTaskLineEditReturnPressed();

    void onSubTaskDoubleClicked(QListWidgetItem* item);
    void showSubTaskContextMenu(const QPoint& pos);
    //void handleModifySelectedSubTask();
    void showSubTaskMenu(const QPoint& pos, const SubTask& subTask); // 新增：通用的菜单显示函数
    void handleSubTaskDataUpdate(const SubTask& updatedSubTask);
    void handleSubTaskOptionsClicked(const QPoint& pos, const SubTask& subTask); // New slot

    void onDueDateButtonClicked();


private:
    void setupUi();
    void updateSubTasksListHeight();

    TodoItem m_currentTask; // 持有当前正在显示的Task的副本
    // UI 控件
    QCheckBox* m_completedCheckBox;
    QLineEdit* m_titleLineEdit;
    QListWidget* m_subTasksListWidget;
    QLineEdit* m_addSubTaskLineEdit;
    QPushButton* m_remindButton;
    QPushButton* m_dueDateButton;
    QTextEdit* m_notesTextEdit; // 用于“备注”
    QLabel* m_creationDateLabel;
    QPushButton* m_deleteButton;
    QToolButton* m_closeButton;

protected:
    // --- 新增：重写 eventFilter 来捕获子控件事件 ---
    bool eventFilter(QObject* watched, QEvent* event) override;
};


