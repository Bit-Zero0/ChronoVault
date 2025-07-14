#include "gui/AnniversaryDetailView.h"
#include "gui/MomentCardWidget.h"
#include "gui/MomentDetailDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QToolButton>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>
#include <QEvent>
#include <QScrollBar>
#include <QTextBrowser>

AnniversaryDetailView::AnniversaryDetailView(QWidget* parent) : QWidget(parent)
{
    setupUi();

    // 初始化倒计时定时器
    m_countdownTimer = new QTimer(this);
    connect(m_countdownTimer, &QTimer::timeout, this, &AnniversaryDetailView::updateCountdown);

    // 初始化自动滚动定时器
    m_autoScrollTimer = new QTimer(this);
    connect(m_autoScrollTimer, &QTimer::timeout, this, &AnniversaryDetailView::autoScrollMoments);
    m_momentsScrollArea->viewport()->installEventFilter(this);
}

void AnniversaryDetailView::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 10, 20, 20);
    mainLayout->setSpacing(15);

    QHBoxLayout* navLayout = new QHBoxLayout();
    m_backButton = new QToolButton();
    m_backButton->setText(tr("⬅️ 返回概览"));
    m_backButton->setAutoRaise(true);
    m_backButton->setStyleSheet("QToolButton { border: none; font-size: 14px; color: #0078d4; }");
    connect(m_backButton, &QToolButton::clicked, this, &AnniversaryDetailView::backRequested);
    navLayout->addWidget(m_backButton, 0, Qt::AlignLeft);
    navLayout->addStretch();

    m_titleLabel = new QLabel();
    m_titleLabel->setStyleSheet("font-size: 28px; font-weight: bold;");

    m_countdownLabel = new QLabel();
    m_countdownLabel->setStyleSheet("font-size: 48px; color: #0078d4; font-weight: 300;");

    QLabel* momentsHeader = new QLabel(tr("时光瞬间"));
    momentsHeader->setStyleSheet("font-size: 16px; font-weight: bold; color: gray;");

    m_momentsScrollArea = new QScrollArea();
    m_momentsScrollArea->setWidgetResizable(true);
    m_momentsScrollArea->setFixedHeight(150);
    m_momentsScrollArea->setStyleSheet("QScrollArea { border: 1px solid #e0e0e0; border-radius: 8px; background-color: white; }");

    // 【核心修正】确保内部容器和布局被正确创建和设置
    QWidget* momentsContainer = new QWidget();
    m_momentsLayout = new QHBoxLayout(momentsContainer);
    m_momentsLayout->setSpacing(10);
    m_momentsLayout->setAlignment(Qt::AlignLeft);
    m_momentsScrollArea->setWidget(momentsContainer); // 将容器设置为滚动区域的控件

    mainLayout->addLayout(navLayout);
    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignCenter);
    mainLayout->addStretch(1);
    mainLayout->addWidget(m_countdownLabel, 0, Qt::AlignCenter);
    mainLayout->addStretch(2);
    mainLayout->addWidget(momentsHeader);
    mainLayout->addWidget(m_momentsScrollArea);
}

void AnniversaryDetailView::displayAnniversary(const AnniversaryItem& item)
{
    qDebug() << "[DIAGNOSTIC 3] In AnniversaryDetailView::displayAnniversary, received item has"
             << item.moments().count() << "moments.";

    m_currentItem = item;
    m_titleLabel->setText(m_currentItem.title());
    updateCountdown();

    bool isFutureCountdown = (m_currentItem.eventType() == AnniversaryEventType::Countdown && m_currentItem.targetDateTime() > QDateTime::currentDateTime());
    if (isFutureCountdown) {
        if (!m_countdownTimer->isActive()) {
            m_countdownTimer->start(1000);
        }
    } else {
        m_countdownTimer->stop();
    }

    // 清空旧的Moments
    QLayoutItem* child;
    while ((child = m_momentsLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }

    // 添加新的Moments卡片
    for (const Moment& moment : item.moments()) {
        MomentCardWidget* card = new MomentCardWidget(moment, this);
        connect(card, &MomentCardWidget::clicked, this, &AnniversaryDetailView::onMomentCardClicked);
        m_momentsLayout->addWidget(card);
    }
    m_momentsLayout->addStretch();

    // 【新增】启动或停止时光图的自动滚动
    if(m_momentsLayout->count() > 3) { // 只有卡片多的时候才滚动
        m_autoScrollTimer->start(30); // 数字越小滚动越快
    } else {
        m_autoScrollTimer->stop();
    }
}

void AnniversaryDetailView::updateCountdown() {
    QDateTime target = m_currentItem.targetDateTime();
    if (!target.isValid()) return;

    QDateTime now = QDateTime::currentDateTime();
    qint64 secondsDiff = now.secsTo(target);

    if (secondsDiff <= 0) {
        m_countdownTimer->stop();
    }

    m_countdownLabel->setText(formatRemainingTime(qAbs(secondsDiff)));
}

QString AnniversaryDetailView::formatRemainingTime(qint64 totalSeconds) const {
    const qint64 days = totalSeconds / (24 * 3600);
    totalSeconds %= (24 * 3600);
    const qint64 hours = totalSeconds / 3600;
    totalSeconds %= 3600;
    const qint64 minutes = totalSeconds / 60;
    const qint64 seconds = totalSeconds % 60;

    QString result;
    if (days > 365) {
        result += tr("%1年 ").arg(days / 365);
    }
    result += tr("%1天 %2小时 %3分 %4秒")
              .arg(days % 365)
              .arg(hours)
              .arg(minutes)
              .arg(seconds);
    return result;
}

//void AnniversaryDetailView::onMomentCardClicked(const Moment& moment)
//{
//    // 【修改】创建对话框时，传入当前纪念日的ID
//    MomentDetailDialog dialog(moment, m_currentItem.id(), this);

//    // 【新增】连接对话框的 momentUpdated 信号到服务层
//    // 我们使用 lambda 表达式来简化这个连接
//    connect(&dialog, &MomentDetailDialog::momentUpdated, this, [this](const Moment& updatedMoment){
//        // 命令服务层去更新这个 Moment
//        m_anniversaryService->updateMoment(m_currentItem.id(), updatedMoment);
//    });

//    dialog.exec();
//}

bool AnniversaryDetailView::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_momentsScrollArea->viewport()) {
        if (event->type() == QEvent::Enter) {
            // 鼠标进入，停止滚动
            m_autoScrollTimer->stop();
            return true;
        } else if (event->type() == QEvent::Leave) {
            // 鼠标离开，继续滚动
            if(m_momentsLayout->count() > 3) {
                m_autoScrollTimer->start(30);
            }
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

// 自动滚动的具体实现
void AnniversaryDetailView::autoScrollMoments()
{
    QScrollBar* scrollBar = m_momentsScrollArea->horizontalScrollBar();
    int nextValue = scrollBar->value() + m_scrollSpeed;
    if (nextValue > scrollBar->maximum()) {
        nextValue = scrollBar->minimum(); // 滚动到尽头后，从头开始
    }
    scrollBar->setValue(nextValue);
}

void AnniversaryDetailView::onMomentCardClicked(const Moment& moment)
{
    MomentDetailDialog dialog(moment, m_currentItem.id(), this);

    // 【核心修正】使用一个直接的、类型安全的信号-信号连接
    // 将 dialog 发出的 momentUpdated 信号，直接转发给 AnniversaryDetailView 自己发出的同名信号
    connect(&dialog, &MomentDetailDialog::momentUpdated,
            this, &AnniversaryDetailView::momentUpdated);

    dialog.exec();
}

//void MomentDetailDialog::performAutoSave()
//{
//    // 1. 更新 Moment 对象的内容
//    m_currentMoment.setText(m_textBrowser->toMarkdown());

//    // 2. 【核心修正】发射信号时，同时传递 anniversaryId 和 moment 对象
//    emit momentUpdated(m_anniversaryId, m_currentMoment);

//    qDebug() << "Auto-saved moment:" << m_currentMoment.id();
//}
