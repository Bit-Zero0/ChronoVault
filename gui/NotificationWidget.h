#pragma once

#include <QWidget>
#include <QUuid>
#include <QEnterEvent>

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QComboBox;
QT_END_NAMESPACE

class NotificationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationWidget(const QUuid& id, const QString& title, const QString& message, QWidget *parent = nullptr);
    ~NotificationWidget();

signals:
    // 当用户点击“稍后提醒”时发射此信号
     void snoozeRequested(const QUuid& id, int minutes);
    // 当用户点击“不再提醒”时发射此信号
    void dismissRequested(const QUuid& id);
    // 当通知关闭时（无论是用户点击还是自动关闭），发射此信号
    void closed();

protected:
    // 用于实现淡入淡出动画
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private slots:
    void onSnoozeClicked();
    void onDismissClicked();
    void fadeOutAndClose(); // 淡出并关闭

private:
    void setupUi(const QString& title, const QString& message);

    QUuid m_id; // 存储此通知关联的待办事项或纪念日的ID
    QComboBox* m_snoozeComboBox;
};
