#pragma once
#include <QWidget>
#include "core/TodoItem.h"

// 前向声明所有需要用到的 Qt 类
QT_BEGIN_NAMESPACE
class QCheckBox;
class QLabel;
class QLineEdit;
class QStackedWidget;
class QMouseEvent;
QT_END_NAMESPACE

class TodoListItemWidget : public QWidget {
    Q_OBJECT

public:
    explicit TodoListItemWidget(const TodoItem& item, QWidget* parent = nullptr);
    TodoItem getTodoItem() const;

    void enterEditMode();

signals:
    void taskUpdated(const QUuid& taskId, bool isCompleted);
    void taskTitleChanged(const QUuid& taskId, const QString& newTitle);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private slots:
    void onStateChanged(int state);
    void exitEditMode();

private:
    void setupUi(); // <-- 【重要】添加这一行缺失的声明
    //void setCompleted(bool completed);

    TodoItem m_item;

    // UI 控件
    QStackedWidget* m_titleStackedWidget;
    QCheckBox* m_completedCheckBox;
    QLineEdit* m_titleEdit;
    QLabel* m_dueDateLabel;
    QLabel* m_subTaskInfoLabel;
};
