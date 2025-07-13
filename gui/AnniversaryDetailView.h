#pragma once

#include <QWidget>
#include "core/AnniversaryItem.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QScrollArea;
class QToolButton;
class QTimer;
QT_END_NAMESPACE

class AnniversaryDetailView : public QWidget
{
    Q_OBJECT

public:
    explicit AnniversaryDetailView(QWidget* parent = nullptr);
    void displayAnniversary(const AnniversaryItem& item);

signals:
    // 当用户点击“返回上一层”按钮时发射此信号
    void backRequested();

private slots:
    void updateCountdown();

private:
    AnniversaryItem m_currentItem;
    QTimer* m_countdownTimer;

    // UI 控件
    QToolButton* m_backButton;
    QLabel* m_titleLabel;
    QLabel* m_countdownLabel;
    QScrollArea* m_momentsScrollArea; // “时光图”的滚动区域
};
