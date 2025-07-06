#include "gui/CompletedHeaderWidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QMouseEvent>

CompletedHeaderWidget::CompletedHeaderWidget(int completedCount, bool isExpanded, QWidget* parent)
    : QWidget(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 8, 5, 8);

    m_expandButton = new QToolButton();
    m_expandButton->setCursor(Qt::PointingHandCursor);
    m_expandButton->setAutoRaise(true);
    m_expandButton->setCheckable(true);
    m_expandButton->setChecked(isExpanded);
    m_expandButton->setArrowType(isExpanded ? Qt::DownArrow : Qt::RightArrow);

    m_titleLabel = new QLabel(QString("已完成 %1").arg(completedCount));
    m_titleLabel->setStyleSheet("font-weight: bold;");

    layout->addWidget(m_expandButton);
    layout->addWidget(m_titleLabel);
    layout->addStretch();

    // 连接按钮点击信号，以便外部可以监听
    connect(m_expandButton, &QToolButton::clicked, this, &CompletedHeaderWidget::toggleExpansion);
}

// 重写鼠标点击事件，使得点击整个控件区域都能触发折叠/展开
void CompletedHeaderWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_expandButton->click(); // 模拟点击箭头按钮
    }
    QWidget::mousePressEvent(event);
}
