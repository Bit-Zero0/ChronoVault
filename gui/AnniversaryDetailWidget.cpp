#include "gui/AnniversaryDetailWidget.h"
#include "gui/MomentDetailDialog.h"
#include "gui/MomentWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QToolButton>

AnniversaryDetailWidget::AnniversaryDetailWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 顶部标题和关闭按钮
    QHBoxLayout* headerLayout = new QHBoxLayout();
    m_titleLabel = new QLabel();
    m_titleLabel->setStyleSheet("font-size: 18px; font-weight: bold;");
    QToolButton* closeButton = new QToolButton();
    closeButton->setText("✕");
    closeButton->setAutoRaise(true);
    connect(closeButton, &QToolButton::clicked, this, &AnniversaryDetailWidget::closeRequested);
    headerLayout->addWidget(m_titleLabel, 1);
    headerLayout->addWidget(closeButton);
    mainLayout->addLayout(headerLayout);

    // 可滚动的区域，用于显示Moments列表
    m_momentsScrollArea = new QScrollArea();
    m_momentsScrollArea->setWidgetResizable(true);
    m_momentsScrollArea->setFixedHeight(150);
    m_momentsScrollArea->setStyleSheet("QScrollArea { border: 1px solid #e0e0e0; border-radius: 8px; background-color: white; }");
    m_momentsContainer = new QWidget();
    m_momentsLayout = new QHBoxLayout(m_momentsContainer); // <-- 改为 QHBoxLayout
    m_momentsLayout->setSpacing(10);
    m_momentsLayout->setAlignment(Qt::AlignLeft);
    m_scrollArea->setWidget(m_momentsContainer);
    mainLayout->addWidget(momentsHeader);
    mainLayout->addWidget(m_momentsScrollArea);
}

