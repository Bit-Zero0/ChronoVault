#include "gui/NotificationWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QTimer>
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QComboBox>

NotificationWidget::NotificationWidget(const QUuid& id, const QString& title, const QString& message, QWidget *parent)
    : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint), m_id(id)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    setAttribute(Qt::WA_DeleteOnClose);
    setupUi(title, message);

    // 10秒后自动淡出
    QTimer::singleShot(10000, this, &NotificationWidget::fadeOutAndClose);

    QPropertyAnimation *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(300);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

NotificationWidget::~NotificationWidget()
{
    qDebug() << "NotificationWidget destroyed for ID:" << m_id;
}

void NotificationWidget::setupUi(const QString& title, const QString& message)
{
    // 使用样式表美化外观
    this->setStyleSheet(R"(
        QWidget {
            background-color: #333333;
            color: white;
            border-radius: 8px;
        }
        QLabel#titleLabel {
            font-size: 14px;
            font-weight: bold;
            padding: 5px;
        }
        QLabel#messageLabel {
            font-size: 12px;
            padding: 5px;
        }
        QPushButton {
            background-color: #555555;
            border: none;
            border-radius: 4px;
            padding: 8px;
            min-width: 80px;
        }
        QPushButton:hover {
            background-color: #0078d4;
        }
    )");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 5, 10, 10);

    QLabel* titleLabel = new QLabel(title);
    titleLabel->setObjectName("titleLabel");
    QLabel* messageLabel = new QLabel(message);
    messageLabel->setObjectName("messageLabel");
    messageLabel->setWordWrap(true);


    m_snoozeComboBox = new QComboBox();
    m_snoozeComboBox->addItem(tr("5 分钟后"), 5);
    m_snoozeComboBox->addItem(tr("10 分钟后"), 10);
    m_snoozeComboBox->addItem(tr("20 分钟后"), 20);
    m_snoozeComboBox->addItem(tr("30 分钟后"), 30);
    m_snoozeComboBox->addItem(tr("1 小时后"), 60);

    QPushButton* snoozeButton = new QPushButton(tr("稍后提醒"));
    QPushButton* dismissButton = new QPushButton(tr("不再提醒"));
    connect(snoozeButton, &QPushButton::clicked, this, &NotificationWidget::onSnoozeClicked);
    connect(dismissButton, &QPushButton::clicked, this, &NotificationWidget::onDismissClicked);


    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_snoozeComboBox);
    buttonLayout->addWidget(snoozeButton);
    buttonLayout->addWidget(dismissButton);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(messageLabel);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonLayout);
}

void NotificationWidget::onSnoozeClicked()
{
    // 从下拉列表中获取用户选择的分钟数（我们之前存在了 userData 中）
    int minutes = m_snoozeComboBox->currentData().toInt();
    emit snoozeRequested(m_id, minutes);

    emit closed();
    // 【重要修正】立即关闭，不再等待动画
    this->close();
}

void NotificationWidget::onDismissClicked()
{
    emit dismissRequested(m_id);

    emit closed();
    // 【重要修正】立即关闭，不再等待动画
    this->close();
}

void NotificationWidget::fadeOutAndClose()
{
    // 这个函数现在只给自动关闭时使用，保留淡出动画
    QPropertyAnimation* animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(300);
    animation->setStartValue(windowOpacity());
    animation->setEndValue(0.0);
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    // 【核心修正】在动画结束、窗口关闭前，确保发射 closed() 信号
    // 我们使用 Lambda 表达式来同时完成两个操作
    connect(animation, &QPropertyAnimation::finished, this, [this]() {
        emit closed();  // 1. 发射信号，通知服务层“解锁”
        this->close();  // 2. 关闭并销毁窗口
        });
}

// 鼠标悬停时保持不透明
void NotificationWidget::enterEvent(QEnterEvent* event)
{
    setWindowOpacity(1.0);
    QWidget::enterEvent(event);
}

void NotificationWidget::leaveEvent(QEvent* event)
{
    // 这里可以添加逻辑，比如鼠标离开后重新开始计时
    QWidget::leaveEvent(event);
}
