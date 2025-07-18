#pragma once

#include <QWidget>
#include "core/AnniversaryItem.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QScrollArea;
class QToolButton;
class QTimer;
class QVBoxLayout;
class QHBoxLayout;
QT_END_NAMESPACE

class AnniversaryDetailView : public QWidget
{
    Q_OBJECT

public:
    explicit AnniversaryDetailView(QWidget* parent = nullptr);
    void displayAnniversary(const AnniversaryItem& item);
    QUuid currentItemId() const;

signals:
    void backRequested();
    void momentUpdated(const QUuid& anniversaryId, const Moment& updatedMoment);
    void momentDeleteRequested(const QUuid& anniversaryId, const QUuid& momentId);
    void momentAdded(const QUuid& anniversaryId, const Moment& newMoment);
    void refreshRequested(const QUuid& anniversaryId);

protected:
    // 重写事件过滤器，用于监听鼠标进入/离开滚动区域
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void updateCountdown();
    void onMomentCardClicked(const Moment& moment);
    void autoScrollMoments(); // 【新增】用于自动滚动的槽函数
    void onMomentDeleteRequested(const QUuid& momentId, const QString& momentText);
    void onAddMomentClicked();


private:
    void setupUi();
    QString formatRemainingTime(qint64 seconds) const;
    void performAutoSave();

    QLabel* m_momentsHeader;
    AnniversaryItem m_currentItem;
    QTimer* m_countdownTimer;
    QTimer* m_autoScrollTimer; // 【新增】自动滚动的定时器
    int m_scrollSpeed = 1;     // 滚动速度

    // UI 控件
    QToolButton* m_backButton;
    QLabel* m_titleLabel;
    QLabel* m_countdownLabel;
    QScrollArea* m_momentsScrollArea;
    QWidget* m_momentsContainer;
    QHBoxLayout* m_momentsLayout;
    QToolButton* m_addMomentButton;

};
