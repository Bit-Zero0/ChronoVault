#include "gui/SubTaskItemWidget.h"
#include <QHBoxLayout>
#include <QCheckBox>
#include <QToolButton>
#include <QLineEdit>
#include <QStackedWidget>
#include <QFont>
#include <QPalette>
#include <QMouseEvent>

SubTaskItemWidget::SubTaskItemWidget(const SubTask& subTask, QWidget* parent)
    : QWidget(parent), m_subTask(subTask)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    m_stackedWidget = new QStackedWidget();

    m_checkBox = new QCheckBox(m_subTask.title);
    m_checkBox->setChecked(m_subTask.isCompleted);
    m_stackedWidget->addWidget(m_checkBox);

    m_editLineEdit = new QLineEdit();
    m_editLineEdit->setStyleSheet("background-color: #f0f0f0; border: 1px solid #0078d4;");
    m_stackedWidget->addWidget(m_editLineEdit);

    m_optionsButton = new QToolButton();
    m_optionsButton->setText("⋮");
    m_optionsButton->setToolTip("更多选项");
    m_optionsButton->setCursor(Qt::PointingHandCursor);
    m_optionsButton->setAutoRaise(true);
    m_optionsButton->setStyleSheet("QToolButton { border: none; background: transparent; font-size: 16px; padding: 0px; }");

    layout->addWidget(m_stackedWidget, 1);
    layout->addWidget(m_optionsButton);

    m_checkBox->setChecked(m_subTask.isCompleted);
    setCompletedStyle(m_subTask.isCompleted);

    connect(m_checkBox, &QCheckBox::stateChanged, this, &SubTaskItemWidget::onCheckBoxStateChanged);
    connect(m_optionsButton, &QToolButton::clicked, this, &SubTaskItemWidget::onOptionsMenuClicked);
    connect(m_editLineEdit, &QLineEdit::editingFinished, this, &SubTaskItemWidget::exitEditMode);
}

const SubTask& SubTaskItemWidget::getSubTask() const {
    return m_subTask;
}

void SubTaskItemWidget::enterEditMode() {
    m_editLineEdit->setText(m_checkBox->text());
    m_stackedWidget->setCurrentWidget(m_editLineEdit);
    m_editLineEdit->setFocus();
    m_editLineEdit->selectAll();
}

void SubTaskItemWidget::exitEditMode() {
    QString newTitle = m_editLineEdit->text().trimmed();
    QString oldTitle = m_checkBox->text();

    if (!newTitle.isEmpty() && newTitle != oldTitle) {
        m_subTask.title = newTitle;
        m_checkBox->setText(newTitle);
        // 【重要】发射正确的 subTaskUpdated 信号
        emit subTaskUpdated(m_subTask);
    }

    m_stackedWidget->setCurrentWidget(m_checkBox);
}

void SubTaskItemWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        enterEditMode();
    }
    QWidget::mouseDoubleClickEvent(event);
}

// 【重要】onCheckBoxStateChanged 现在只负责发射信号
void SubTaskItemWidget::onCheckBoxStateChanged(int state) {
    bool isCompleted = (state == Qt::Checked);
    // 不再调用 setCompletedStyle(isCompleted); 来立即更新UI
    emit subTaskStateChanged(m_subTask.id, isCompleted);
}

void SubTaskItemWidget::onOptionsMenuClicked() {
    QPoint pos = m_optionsButton->mapToGlobal(QPoint(0, m_optionsButton->height()));
    emit optionsMenuRequested(pos, m_subTask);
}
// --- 【重要】修正 setCompletedStyle 函数的实现 ---
void SubTaskItemWidget::setCompletedStyle(bool completed) {
    QFont font = m_checkBox->font();
    font.setStrikeOut(completed); // 设置或取消删除线
    m_checkBox->setFont(font);

    // 使用样式表来改变颜色，而不是调色板，以避免交互问题
    if (completed) {
        m_checkBox->setStyleSheet("color: gray;");
    } else {
        m_checkBox->setStyleSheet("color: black;");
    }
}
