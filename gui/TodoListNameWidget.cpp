#include "gui/TodoListNameWidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QStackedWidget>
#include <QMouseEvent>

TodoListNameWidget::TodoListNameWidget(QUuid id, const QString& name, QWidget* parent)
    : QWidget(parent), m_id(id)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    // 【核心修正】为布局添加垂直边距，为文字提供足够的呼吸空间
    layout->setContentsMargins(5, 6, 5, 6);

    m_stackedWidget = new QStackedWidget();
    layout->addWidget(m_stackedWidget);

    m_nameLabel = new QLabel(name);
    m_nameEdit = new QLineEdit(name);
    m_nameEdit->setStyleSheet("background-color: white; border: 1px solid #3fc1c9;");

    m_stackedWidget->addWidget(m_nameLabel);
    m_stackedWidget->addWidget(m_nameEdit);

    connect(m_nameEdit, &QLineEdit::editingFinished, this, &TodoListNameWidget::exitEditMode);
}

void TodoListNameWidget::enterEditMode() {
    m_nameEdit->setText(m_nameLabel->text());
    m_stackedWidget->setCurrentWidget(m_nameEdit);
    m_nameEdit->setFocus();
    m_nameEdit->selectAll();
}

void TodoListNameWidget::exitEditMode() {
    QString newName = m_nameEdit->text().trimmed();
    QString oldName = m_nameLabel->text();

    if (!newName.isEmpty() && newName != oldName) {
        m_nameLabel->setText(newName);
        emit listNameChanged(m_id, newName);
    }

    m_stackedWidget->setCurrentWidget(m_nameLabel);
}

void TodoListNameWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        enterEditMode();
    }
    QWidget::mouseDoubleClickEvent(event);
}
