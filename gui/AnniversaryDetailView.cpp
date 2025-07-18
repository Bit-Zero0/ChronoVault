#include "gui/AnniversaryDetailView.h"
#include "gui/MomentCardWidget.h"
#include "gui/MomentDetailDialog.h"
#include "gui/AddMomentDialog.h"
#include "core/Moment.h"
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

// AnniversaryDetailView.cpp

void AnniversaryDetailView::setupUi() {
    // 创建主垂直布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 10, 20, 20);
    mainLayout->setSpacing(15);

    // --- 顶部导航栏 (返回按钮) ---
    QHBoxLayout* navLayout = new QHBoxLayout();
    m_backButton = new QToolButton();
    m_backButton->setText(tr("⬅️ 返回概览"));
    m_backButton->setAutoRaise(true);
    m_backButton->setStyleSheet("QToolButton { border: none; font-size: 14px; color: #0078d4; }");
    connect(m_backButton, &QToolButton::clicked, this, &AnniversaryDetailView::backRequested);
    navLayout->addWidget(m_backButton, 0, Qt::AlignLeft);
    navLayout->addStretch();

    // --- 标题和倒计时标签 ---
    m_titleLabel = new QLabel();
    m_titleLabel->setStyleSheet("font-size: 28px; font-weight: bold;");

    m_countdownLabel = new QLabel();
    m_countdownLabel->setStyleSheet("font-size: 48px; color: #0078d4; font-weight: 300;");

    // --- 【核心修正】创建包含“时光瞬间”标题和“+”按钮的水平布局 ---
    QHBoxLayout* momentsHeaderLayout = new QHBoxLayout();
    QLabel* momentsHeader = new QLabel(tr("时光瞬间"));
    momentsHeader->setStyleSheet("font-size: 16px; font-weight: bold; color: gray;");

    m_addMomentButton = new QToolButton();
    m_addMomentButton->setText("+");
    m_addMomentButton->setCursor(Qt::PointingHandCursor);
    m_addMomentButton->setStyleSheet("QToolButton { border: none; font-size: 20px; font-weight: bold; color: #0078d4; padding: 0px 5px; } QToolButton:hover { color: #005a9e; }");
    m_addMomentButton->setToolTip(tr("添加新的瞬间"));
    connect(m_addMomentButton, &QToolButton::clicked, this, &AnniversaryDetailView::onAddMomentClicked);

    // 将标题、弹簧（用于推挤）和按钮添加到这个新的水平布局中
    momentsHeaderLayout->addWidget(momentsHeader);
    momentsHeaderLayout->addStretch();
    momentsHeaderLayout->addWidget(m_addMomentButton);
    // ---------------------------------------------------

    // --- 时光图滚动区域 ---
    m_momentsScrollArea = new QScrollArea();
    m_momentsScrollArea->setWidgetResizable(true);
    m_momentsScrollArea->setFixedHeight(150);
    m_momentsScrollArea->setStyleSheet("QScrollArea { border: 1px solid #e0e0e0; border-radius: 8px; background-color: white; }");

    // --- 将所有部件按顺序添加到主布局 ---
    mainLayout->addLayout(navLayout);
    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignCenter);
    mainLayout->addStretch(1);
    mainLayout->addWidget(m_countdownLabel, 0, Qt::AlignCenter);
    mainLayout->addStretch(2);
    mainLayout->addLayout(momentsHeaderLayout); // **将包含标题和按钮的布局添加到主布局**
    mainLayout->addWidget(m_momentsScrollArea);
}

// gui/AnniversaryDetailView.cpp

void AnniversaryDetailView::displayAnniversary(const AnniversaryItem& item)
{
    qDebug() << "[DisplayAnniversary] Refreshing view for item:" << item.id() << "with" << item.moments().count() << "moments.";
    m_currentItem = item;
    m_titleLabel->setText(m_currentItem.title());

    // --- 【核心刷新逻辑】先销毁旧容器，再创建新容器 ---

    // 1. 获取并安全地删除旧的 widget
    QWidget* oldContainer = m_momentsScrollArea->takeWidget();
    if (oldContainer) {
        delete oldContainer;
    }

    // 2. 创建一个全新的容器 widget 和布局
    QWidget* momentsContainer = new QWidget();
    m_momentsLayout = new QHBoxLayout(momentsContainer);
    m_momentsLayout->setSpacing(10);
    m_momentsLayout->setAlignment(Qt::AlignLeft);
    m_momentsLayout->setContentsMargins(5, 5, 5, 5);

    // 3. 遍历最新的数据，用新数据填充新布局
    for (const Moment& moment : m_currentItem.moments()) {
        MomentCardWidget* card = new MomentCardWidget(moment, momentsContainer);
        connect(card, &MomentCardWidget::clicked, this, &AnniversaryDetailView::onMomentCardClicked);
        connect(card, &MomentCardWidget::deleteRequested, this, &AnniversaryDetailView::onMomentDeleteRequested);
        m_momentsLayout->addWidget(card);
    }
    m_momentsLayout->addStretch();

    // 4. 将全新的、填满内容的容器设置给 ScrollArea
    m_momentsScrollArea->setWidget(momentsContainer);

    // --- 【重要修正】在这里添加倒计时和滚动条的逻辑 ---

    // 5. 首先，立即更新一次倒计时标签的文本
    updateCountdown();

    // 6. 然后，根据事件是否在未来，决定是否启动/停止倒计时定时器
    bool isFutureCountdown = (m_currentItem.eventType() == AnniversaryEventType::Countdown && m_currentItem.targetDateTime() > QDateTime::currentDateTime());
    if (isFutureCountdown) {
        if (!m_countdownTimer->isActive()) {
            // 【新增日志】让我们确认定时器确实被启动了
            qDebug() << "[AnniversaryDetailView] Starting countdown timer for item:" << m_currentItem.title();
            m_countdownTimer->start(1000); // 每秒触发一次
        }
    } else {
        // 如果不是未来的倒计时，确保定时器是停止的
        if (m_countdownTimer->isActive()) {
            qDebug() << "[AnniversaryDetailView] Stopping countdown timer for non-future event:" << m_currentItem.title();
            m_countdownTimer->stop();
        }
    }

    // 7. 最后，处理自动滚动定时器
    if (m_momentsLayout->count() > 3) {
        m_autoScrollTimer->start(30);
    }
    else {
        m_autoScrollTimer->stop();
    }
}

void AnniversaryDetailView::updateCountdown() {

    qDebug() << "[AnniversaryDetailView] updateCountdown() slot called at:" << QDateTime::currentDateTime().toString("hh:mm:ss");

    QDateTime target = m_currentItem.targetDateTime();
    if (!target.isValid()) return;

    QDateTime now = QDateTime::currentDateTime();
    qint64 secondsDiff = now.secsTo(target);

    if (secondsDiff <= 0) {
        m_countdownTimer->stop();
    }

    m_countdownLabel->setText(formatRemainingTime(qAbs(secondsDiff)));

    // 【新增强制刷新】在某些情况下，简单的setText可能不足以触发重绘
    // 我们强制 m_countdownLabel 立即重绘自己
    //m_countdownLabel->repaint();
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
    // 1. 创建并以模态方式运行对话框
    MomentDetailDialog dialog(moment, this);
    dialog.exec();

    // 2. 安全地获取最终数据
    Moment finalMoment = dialog.getMoment();

    // 3. 检查数据是否真的发生了变化
    if (finalMoment.text() != moment.text() || finalMoment.imagePaths() != moment.imagePaths()) {
        qDebug() << "Moment data has changed, initiating update for item:" << m_currentItem.id();

        // 【重要修正】不再直接调用 displayAnniversary
        // 只发射信号，将更新的责任完全交给 MainWindow
        emit momentUpdated(m_currentItem.id(), finalMoment);
    }
}


void AnniversaryDetailView::onMomentDeleteRequested(const QUuid& momentId, const QString& momentText)
{
    auto reply = QMessageBox::question(this, tr("确认删除"),
                                       tr("您确定要删除这个瞬间吗？\n\n\"%1\"").arg(momentText),
                                       QMessageBox::Yes | QMessageBox::Cancel);

    if (reply == QMessageBox::Yes) {
        // 将 anniversaryId 和 momentId 一起发射出去
        emit momentDeleteRequested(m_currentItem.id(), momentId);
    }
}

QUuid AnniversaryDetailView::currentItemId() const
{
    return m_currentItem.id();
}


void AnniversaryDetailView::onAddMomentClicked()
{
    // 安全检查，确保当前有一个有效的纪念日项目
    if (m_currentItem.id().isNull()) {
        return;
    }

    AddMomentDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Moment newMoment = dialog.getMoment();
        // 确保用户确实输入了内容或添加了图片
        if (!newMoment.text().isEmpty() || !newMoment.imagePaths().isEmpty()) {
            // 发射信号，将新 Moment 和其所属的 Anniversary ID 传递出去
            emit momentAdded(m_currentItem.id(), newMoment);

            // 发射刷新请求信号，请求 MainWindow 刷新自己
            emit refreshRequested(m_currentItem.id());
        }
    }
}
