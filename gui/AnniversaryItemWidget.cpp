#include "gui/AnniversaryItemWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QTimer>
#include <QEnterEvent>
#include <QMenu>
#include <QMessageBox>
#include <QContextMenuEvent>

AnniversaryItemWidget::AnniversaryItemWidget(const AnniversaryItem& item, QWidget *parent)
    : QWidget(parent), m_item(item)
{
    setupUi();
    updateCountdown(); // 立即计算并显示一次

    // 只有倒计时需要每秒刷新
    if (m_item.eventType() == AnniversaryEventType::Countdown) {
        m_countdownTimer = new QTimer(this);
        // 【核心修正】在这里明确指出 updateCountdown 是 AnniversaryItemWidget 的成员
        connect(m_countdownTimer, &QTimer::timeout, this, &AnniversaryItemWidget::updateCountdown);
        m_countdownTimer->start(1000); // 每秒刷新一次
    }

    // 其他 connect 语句保持不变
    //connect(m_deleteButton, &QToolButton::clicked, this, &AnniversaryItemWidget::onDeleteClicked);
    connect(m_deleteButton, &QToolButton::clicked, this, &AnniversaryItemWidget::requestDelete);
    connect(m_addToTodoButton, &QToolButton::clicked, this, &AnniversaryItemWidget::onAddToTodoClicked);
    connect(m_addMomentButton, &QToolButton::clicked, this, &AnniversaryItemWidget::onAddMomentClicked);
}

void AnniversaryItemWidget::setupUi() {
    // 设置卡片的基本样式
    this->setStyleSheet("QWidget { background-color: #ffffff; border-radius: 8px; }");
    this->setMinimumHeight(80);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 10, 15, 10);

    m_iconLabel = new QLabel();
    // 未来可以根据类型设置不同图标
    m_iconLabel->setPixmap(QPixmap(":/icons/icon_anniversary.png").scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation)); // 假设有一个图标资源

    QVBoxLayout* textLayout = new QVBoxLayout();
    m_titleLabel = new QLabel(m_item.title());
    m_titleLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    m_countdownLabel = new QLabel();
    m_countdownLabel->setStyleSheet("font-size: 20px; color: #0078d4;");
    m_targetDateLabel = new QLabel();
    m_targetDateLabel->setStyleSheet("font-size: 12px; color: gray;");
    textLayout->addWidget(m_titleLabel);
    textLayout->addStretch();
    textLayout->addWidget(m_targetDateLabel);

    m_deleteButton = new QToolButton();
    m_addToTodoButton = new QToolButton();
    m_deleteButton->setText(tr("✕"));
    m_deleteButton->setAutoRaise(true);
    m_deleteButton->setCursor(Qt::PointingHandCursor);
    m_deleteButton->hide(); // 默认隐藏


    m_addToTodoButton = new QToolButton();
    m_addToTodoButton->setText(tr("-> 添加到待办"));
    m_addToTodoButton->setCursor(Qt::PointingHandCursor);
    m_addToTodoButton->setStyleSheet("QToolButton { border: none; color: #0078d4; }");
    // 只有倒计时且未被添加过的项目才显示此按钮
    if (m_item.eventType() == AnniversaryEventType::Countdown && !m_item.isAddedToTodo()) {
        textLayout->addWidget(m_addToTodoButton);
    }

    m_addMomentButton = new QToolButton();
    m_addMomentButton->setText(tr("＋ 记录瞬间"));
    m_addMomentButton->setCursor(Qt::PointingHandCursor);
    m_addMomentButton->setStyleSheet("QToolButton { border: none; color: #5c5c5c; }");
    textLayout->addWidget(m_addMomentButton); // 将按钮添加到布局中



    mainLayout->addWidget(m_iconLabel);
    mainLayout->addSpacing(15);
    mainLayout->addLayout(textLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(m_countdownLabel, 0, Qt::AlignRight);
    mainLayout->addWidget(m_deleteButton, 0, Qt::AlignTop);


    // 【新增】使用一个垂直布局来把 "x" 按钮放在右上角
    QVBoxLayout* topRightLayout = new QVBoxLayout();
    topRightLayout->addWidget(m_deleteButton, 0, Qt::AlignTop | Qt::AlignRight);
    topRightLayout->addStretch();
    mainLayout->addLayout(topRightLayout);
}

void AnniversaryItemWidget::onAddToTodoClicked()
{
    emit addToTodoRequested(m_item.id());
}

void AnniversaryItemWidget::updateCountdown() {
    QDateTime target = m_item.targetDateTime();
    if (!target.isValid()) {
        m_countdownLabel->setText(tr("无效日期"));
        return;
    }

    QDateTime now = QDateTime::currentDateTime();
    qint64 secondsDiff = now.secsTo(target);

    // 根据事件类型来决定显示逻辑
    if (m_item.eventType() == AnniversaryEventType::Countdown) {
        // --- 这是倒计时事件 ---
        if (secondsDiff > 0) {
            // 倒计时还未结束
            m_countdownLabel->setText(tr("还剩 %1").arg(formatRemainingTime(secondsDiff)));
            m_targetDateLabel->setText(tr("目标于: ") + target.toString("yyyy-MM-dd HH:mm"));
        } else {
            // 倒计时已结束
            m_countdownLabel->setText(tr("已过 %1").arg(formatRemainingTime(-secondsDiff))); // 传入正数秒
            m_targetDateLabel->setText(tr("发生于: ") + target.toString("yyyy-MM-dd HH:mm"));
        }
    } else {
        // --- 这是纪念日事件 ---
        qint64 daysPassed = target.daysTo(now);
        m_countdownLabel->setText(tr("已 %1 天").arg(daysPassed));
        m_targetDateLabel->setText(tr("发生于: ") + target.toString("yyyy-MM-dd"));
    }
}

QString AnniversaryItemWidget::formatRemainingTime(qint64 totalSeconds) const {
    // 确保秒数不为负
    if (totalSeconds < 0) {
        totalSeconds = 0;
    }

    // 1. 计算出天、小时、分钟和秒
    const qint64 days = totalSeconds / (24 * 3600);
    totalSeconds %= (24 * 3600);
    const qint64 hours = totalSeconds / 3600;
    totalSeconds %= 3600;
    const qint64 minutes = totalSeconds / 60;
    const qint64 seconds = totalSeconds % 60;

    // 2. 使用一个列表来动态构建最终的字符串
    QStringList parts;

    // 3. 只有当单位不为0时，才将其添加到列表中
    if (days > 0) {
        parts << tr("%1天").arg(days);
    }
    if (hours > 0) {
        parts << tr("%1小时").arg(hours);
    }
    if (minutes > 0) {
        parts << tr("%1分钟").arg(minutes);
    }

    // 4. 【特殊情况】如果天、小时、分钟都为0，那么我们应该显示秒
    if (parts.isEmpty()) {
        parts << tr("%1秒").arg(seconds);
    }

    // 5. 将列表中的所有部分用空格连接成一个最终的字符串
    return parts.join(" ");
}

void AnniversaryItemWidget::enterEvent(QEnterEvent *event) {
    m_deleteButton->show();
    QWidget::enterEvent(event);
}

void AnniversaryItemWidget::leaveEvent(QEvent *event) {
    m_deleteButton->hide();
    QWidget::leaveEvent(event);
}

void AnniversaryItemWidget::onDeleteClicked() {
    emit itemDeleted(m_item.id());
}

void AnniversaryItemWidget::onAddMomentClicked()
{
    emit addMomentRequested(m_item.id());
}


const AnniversaryItem& AnniversaryItemWidget::item() const
{
    return m_item;
}



// 统一的删除请求处理函数
void AnniversaryItemWidget::requestDelete()
{
    // 弹出确认对话框
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("确认删除"),
                                  tr("您确定要删除“%1”吗？\n此操作不可撤销。").arg(m_item.title()),
                                  QMessageBox::Yes | QMessageBox::Cancel);

    // 如果用户点击了“是”，则发射信号
    if (reply == QMessageBox::Yes) {
        // 发射信号，将ID和标题传递给主窗口
        emit deleteRequested(m_item.id(), m_item.title());
    }
}
