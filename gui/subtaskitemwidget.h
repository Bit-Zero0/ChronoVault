#pragma once
#include <QWidget>
#include "core/SubTask.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QToolButton;
class QLineEdit;
class QStackedWidget;
class QMouseEvent; // <-- 引入 QMouseEvent
QT_END_NAMESPACE

class SubTaskItemWidget : public QWidget {
    Q_OBJECT

public:
    explicit SubTaskItemWidget(const SubTask& subTask, QWidget* parent = nullptr);
    const SubTask& getSubTask() const;

    void enterEditMode();

signals:
    void subTaskStateChanged(const QUuid& subTaskId, bool isCompleted);
    void optionsMenuRequested(const QPoint& globalPos, const SubTask& subTask);
    // 【重要】确保 subTaskUpdated 信号被声明
    void subTaskUpdated(const SubTask& updatedSubTask);

protected:
    // 【重要】确保 mouseDoubleClickEvent 函数被声明
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private slots:
    void onCheckBoxStateChanged(int state);
    void onOptionsMenuClicked();
    void exitEditMode();

private:
    void setCompletedStyle(bool completed);

    SubTask m_subTask;

    // UI 控件
    QStackedWidget* m_stackedWidget;
    QCheckBox* m_checkBox;
    QLineEdit* m_editLineEdit;
    QToolButton* m_optionsButton;
};
