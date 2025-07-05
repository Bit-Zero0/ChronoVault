#include "gui/TodoListItemWidget.h"
// --- 【重要】添加所有缺失的头文件 ---
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QFont>
#include <QPalette>
#include <QMouseEvent>
#include <numeric>
// ------------------------------------

TodoListItemWidget::TodoListItemWidget(const TodoItem &item, QWidget *parent)
    : QWidget(parent), m_item(item) {
    setupUi();
    connect(m_completedCheckBox, &QCheckBox::stateChanged, this, &TodoListItemWidget::onStateChanged);
    connect(m_titleEdit, &QLineEdit::editingFinished, this, &TodoListItemWidget::exitEditMode);
}

// setupUi 是一个私有成员函数，需要在这里实现
void TodoListItemWidget::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(2);

    QHBoxLayout *topLayout = new QHBoxLayout();

    m_titleStackedWidget = new QStackedWidget();
    m_completedCheckBox = new QCheckBox(m_item.title());
    m_titleEdit = new QLineEdit(m_item.title());
    m_titleEdit->setStyleSheet("background-color: white; border: 1px solid #0078d4;");
    m_titleStackedWidget->addWidget(m_completedCheckBox);
    m_titleStackedWidget->addWidget(m_titleEdit);

    m_dueDateLabel = new QLabel();
    if (m_item.dueDate().isValid()) {
         m_dueDateLabel->setText(m_item.dueDate().toString("MM-dd hh:mm"));
    }
    m_dueDateLabel->setStyleSheet("color: gray;");

    topLayout->addWidget(m_titleStackedWidget);
    topLayout->addStretch();
    topLayout->addWidget(m_dueDateLabel);

    m_subTaskInfoLabel = new QLabel();
    m_subTaskInfoLabel->setStyleSheet("color: gray; padding-left: 25px;");

    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(m_subTaskInfoLabel);

    const int subTaskCount = m_item.subTasks().count();
    if (subTaskCount > 0) {
        int completedCount = std::accumulate(m_item.subTasks().begin(), m_item.subTasks().end(), 0,
            [](int sum, const SubTask& task){ return sum + (task.isCompleted ? 1 : 0); });
        m_subTaskInfoLabel->setText(QString("%1 / %2 已完成").arg(completedCount).arg(subTaskCount));
        m_subTaskInfoLabel->setVisible(true);
    } else {
        m_subTaskInfoLabel->setVisible(false);
    }

    m_completedCheckBox->setChecked(m_item.isCompleted());
    setCompleted(m_item.isCompleted());
}

void TodoListItemWidget::enterEditMode() {
    m_titleEdit->setText(m_completedCheckBox->text());
    m_titleStackedWidget->setCurrentWidget(m_titleEdit);
    m_titleEdit->setFocus();
    m_titleEdit->selectAll();
}

void TodoListItemWidget::exitEditMode() {
    QString newTitle = m_titleEdit->text().trimmed();
    QString oldTitle = m_completedCheckBox->text();

    if (!newTitle.isEmpty() && newTitle != oldTitle) {
        m_item.setTitle(newTitle);
        m_completedCheckBox->setText(newTitle);
        emit taskTitleChanged(m_item.id(), newTitle);
    }

    m_titleStackedWidget->setCurrentWidget(m_completedCheckBox);
}

void TodoListItemWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_titleStackedWidget->currentWidget() == m_completedCheckBox) {
        enterEditMode();
    }
    QWidget::mouseDoubleClickEvent(event);
}

void TodoListItemWidget::onStateChanged(int state) {
    bool isCompleted = (state == Qt::Checked);
    // 不再调用 setCompleted(isCompleted); 来立即更新UI
    emit taskUpdated(m_item.id(), isCompleted);
}

TodoItem TodoListItemWidget::getTodoItem() const {
    return m_item;
}

void TodoListItemWidget::setCompleted(bool completed) {
    QFont font = m_completedCheckBox->font();
    font.setStrikeOut(completed); // 设置或取消删除线
    m_completedCheckBox->setFont(font);

    // 我们不再修改复选框本身的调色板，以避免它在某些系统样式下变得不可交互。
    // 我们只改变它旁边次要标签的颜色。
    QPalette pal = m_subTaskInfoLabel->palette();
    pal.setColor(QPalette::WindowText, completed ? Qt::gray : Qt::black);
    m_subTaskInfoLabel->setPalette(pal);

    // 同时，为了让标题文本也变灰，我们可以使用样式表，这比调色板更安全
    if (completed) {
        m_completedCheckBox->setStyleSheet("color: gray;");
    } else {
        m_completedCheckBox->setStyleSheet("color: black;");
    }
}
