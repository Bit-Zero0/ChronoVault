#pragma once

#include <QWidget>
#include "core/AnniversaryItem.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QToolButton;
class QTimer;
QT_END_NAMESPACE

class AnniversaryItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnniversaryItemWidget(const AnniversaryItem& item, QWidget *parent = nullptr);
    const AnniversaryItem& item() const;

signals:
    // 当用户点击删除按钮时发射此信号
    void itemDeleted(const QUuid& id);
    void addToTodoRequested(const QUuid& id);
    void addMomentRequested(const QUuid& id);
    void deleteRequested(const QUuid& id, const QString& title);

protected:
    // 实现悬停时显示/隐藏删除按钮
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;



private slots:
    // 每秒更新一次倒计时显示
    void updateCountdown();
    void onDeleteClicked();
    void onAddToTodoClicked();
    void onAddMomentClicked();
    void requestDelete();


private:
    void setupUi();
    QString formatRemainingTime(qint64 seconds) const;

    AnniversaryItem m_item;
    QTimer* m_countdownTimer; // 每个卡片自带一个定时器，用于刷新倒计时

    // UI 控件
    QLabel* m_iconLabel;
    QLabel* m_titleLabel;
    QLabel* m_countdownLabel; // "还剩 xx天xx小时 / 已过去 xx天"
    QLabel* m_targetDateLabel;
    QToolButton* m_deleteButton;
    QToolButton* m_addToTodoButton;
    QToolButton* m_addMomentButton;
};
