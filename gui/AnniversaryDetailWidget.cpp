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

void AnniversaryDetailView::displayAnniversary(const AnniversaryItem& item)
{
    // 1. 更新当前显示的纪念日项目，并设置标题
    m_currentItem = item;
    m_titleLabel->setText(m_currentItem.title());

    // 2. 立即更新一次倒计时/计时器的显示
    updateCountdown();

    // 3. 根据项目类型，决定是否启动每秒刷新的定时器
    bool isFutureCountdown = (m_currentItem.eventType() == AnniversaryEventType::Countdown && m_currentItem.targetDateTime() > QDateTime::currentDateTime());
    if (isFutureCountdown) {
        if (!m_countdownTimer->isActive()) {
            m_countdownTimer->start(1000);
        }
    } else {
        m_countdownTimer->stop();
    }

    // 4. 动态填充“时光瞬间”滚动区域
    // 4a. 清空上一次显示的所有“瞬间”卡片，防止内容重复
    QLayoutItem* child;
    while ((child = m_momentsLayout->takeAt(0)) != nullptr) {
        // 安全地删除旧的控件和布局项，防止内存泄漏
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }

    // 4b. 遍历当前纪念日项目的所有“瞬间”，为每一个都创建一个卡片控件
    for (const Moment& moment : item.moments()) {
        MomentCardWidget* card = new MomentCardWidget(moment, this);
        // 连接卡片的点击信号到详情页的槽函数
        connect(card, &MomentCardWidget::clicked, this, &AnniversaryDetailView::onMomentCardClicked);
        m_momentsLayout->addWidget(card);
    }

    // 4c. 在末尾添加一个弹簧，让所有卡片都向左对齐
    m_momentsLayout->addStretch();
}


void AnniversaryDetailView::onMomentCardClicked(const Moment& moment)
{
    // 【核心修正】创建对话框时，传入当前纪念日的ID
    MomentDetailDialog dialog(moment, m_currentItem.id(), this);

    // 连接对话框的 momentUpdated 信号到详情页自身的 momentUpdated 信号
    connect(&dialog, &MomentDetailDialog::momentUpdated, this, &AnniversaryDetailView::momentUpdated);

    dialog.exec();
}
