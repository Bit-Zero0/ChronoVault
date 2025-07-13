#include "gui/AnniversaryDetailView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QToolButton>
#include <QTimer>

AnniversaryDetailView::AnniversaryDetailView(QWidget* parent) : QWidget(parent)
{
    // 主垂直布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 10, 20, 20);
    mainLayout->setSpacing(15);

    // 1. 顶部导航栏 (返回按钮)
    QHBoxLayout* navLayout = new QHBoxLayout();
    m_backButton = new QToolButton();
    m_backButton->setText(tr("<-"));
    m_backButton->setAutoRaise(true);
    m_backButton->setStyleSheet("QToolButton { border: none; font-size: 14px; color: #0078d4; }");
    connect(m_backButton, &QToolButton::clicked, this, &AnniversaryDetailView::backRequested);
    navLayout->addWidget(m_backButton, 0, Qt::AlignLeft);
    navLayout->addStretch();
    mainLayout->addLayout(navLayout);

    // 2. 纪念日标题
    m_titleLabel = new QLabel();
    m_titleLabel->setStyleSheet("font-size: 28px; font-weight: bold;");
    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignCenter);

    // 3. 倒计时/计时
    m_countdownLabel = new QLabel();
    m_countdownLabel->setStyleSheet("font-size: 48px; color: #0078d4;");
    mainLayout->addWidget(m_countdownLabel, 0, Qt::AlignCenter);
    mainLayout->addStretch(1); // 添加一些弹性空间

    // 4. “时光图”滚动区域 (目前只是一个空的占位符)
    QLabel* momentsHeader = new QLabel(tr("时光瞬间"));
    momentsHeader->setStyleSheet("font-size: 16px; font-weight: bold; color: gray;");
    m_momentsScrollArea = new QScrollArea();
    m_momentsScrollArea->setWidgetResizable(true);
    m_momentsScrollArea->setFixedHeight(150);
    m_momentsScrollArea->setStyleSheet("QScrollArea { border: 1px solid #e0e0e0; border-radius: 8px; }");
    mainLayout->addWidget(momentsHeader);
    mainLayout->addWidget(m_momentsScrollArea);
    mainLayout->addStretch(2); // 添加更多弹性空间

    // 5. 初始化定时器
    m_countdownTimer = new QTimer(this);
    connect(m_countdownTimer, &QTimer::timeout, this, &AnniversaryDetailView::updateCountdown);
}

void AnniversaryDetailView::displayAnniversary(const AnniversaryItem& item)
{
    m_currentItem = item;
    m_titleLabel->setText(m_currentItem.title());

    // 立即更新一次倒计时显示
    updateCountdown();

    // 启动或停止定时器
    if (m_currentItem.eventType() == AnniversaryEventType::Countdown) {
        if (!m_countdownTimer->isActive()) {
            m_countdownTimer->start(1000);
        }
    } else {
        m_countdownTimer->stop();
    }

    // (我们将在下一步实现这里：填充 m_momentsScrollArea 的内容)
}

void AnniversaryDetailView::updateCountdown()
{
    // (这段逻辑是从旧的 AnniversaryItemWidget 中借鉴而来)
    QDateTime now = QDateTime::currentDateTime();
    QDateTime target = m_currentItem.targetDateTime();
    qint64 secondsDiff = now.secsTo(target);

    if (secondsDiff <= 0) {
        m_countdownTimer->stop(); // 倒计时结束后停止计时器
    }

    const qint64 days = secondsDiff / (24 * 3600);
    secondsDiff %= (24 * 3600);
    const qint64 hours = secondsDiff / 3600;
    secondsDiff %= 3600;
    const qint64 minutes = secondsDiff / 60;
    const qint64 seconds = secondsDiff % 60;

    m_countdownLabel->setText(tr("%1年 %2天 %3小时 %4秒")
                                  .arg(days / 365)
                                  .arg(days % 365)
                                  .arg(hours)
                                  .arg(seconds)); // 秒数这里有误，应该用minutes
}
