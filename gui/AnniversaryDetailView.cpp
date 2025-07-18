#include "gui/AnniversaryDetailView.h"
#include "gui/MomentCardWidget.h"
#include "gui/MomentDetailDialog.h"
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

// gui/AnniversaryDetailView.cpp

void AnniversaryDetailView::displayAnniversary(const AnniversaryItem& item)
{
    qDebug() << "[DIAGNOSTIC 3] In AnniversaryDetailView::displayAnniversary, received item has"
             << item.moments().count() << "moments.";

    // --- 【重要修正】智能刷新逻辑 ---
    // 检查传入的 item 是否是第一次加载 (m_currentItem 无效) 或 是一个全新的 item
    if (m_currentItem.id() != item.id()) {
        // 如果是全新的 item，执行完整的重建流程
        m_currentItem = item;
        m_titleLabel->setText(m_currentItem.title());

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

    } else {
        // 如果只是更新当前的 item，执行“就地更新”
        m_currentItem = item; // 先更新内部数据

        // 遍历布局中现有的卡片
        for (int i = 0; i < m_momentsLayout->count(); ++i) {
            QLayoutItem* layoutItem = m_momentsLayout->itemAt(i);
            if (auto* card = qobject_cast<MomentCardWidget*>(layoutItem->widget())) {
                // 找到每个卡片对应的新数据并更新它
                for (const Moment& newMomentData : m_currentItem.moments()) {
                    if (card->moment().id() == newMomentData.id()) {
                        card->updateData(newMomentData);
                        break; // 找到后就跳出内层循环
                    }
                }
            }
        }
    }

    // --- 倒计时和滚动条的逻辑保持不变 ---
    updateCountdown();
    if(m_momentsLayout->count() > 3) {
        m_autoScrollTimer->start(30);
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
//void MomentDetailDialog::performAutoSave()
//{
//    // 1. 更新 Moment 对象的内容
//    m_currentMoment.setText(m_textBrowser->toMarkdown());

//    // 2. 【核心修正】发射信号时，同时传递 anniversaryId 和 moment 对象
//    emit momentUpdated(m_anniversaryId, m_currentMoment);

//    qDebug() << "Auto-saved moment:" << m_currentMoment.id();
//}
