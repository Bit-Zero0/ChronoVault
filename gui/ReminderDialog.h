#pragma once

#include <QDialog>
#include "core/Reminder.h"

QT_BEGIN_NAMESPACE
class QTimeEdit;
class QComboBox;
class QSoundEffect;
class QDateTimeEdit;
class QStackedWidget;
QT_END_NAMESPACE

class ReminderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReminderDialog(const Reminder& initialReminder, QWidget *parent = nullptr);
    ~ReminderDialog();

    Reminder getReminder() const;

private slots:
    void onSoundSelectionChanged(int index);
    void onTestSoundClicked();
    void onRecurrenceChanged(int index);
    void onCancelReminderClicked();

private:
    void setupUi(const Reminder& initialReminder);
    Reminder getReminderFromUi() const;

    // 【新增】声明我们将要重写的 accept() 函数
    void accept() override;


    QComboBox* m_recurrenceComboBox;
    QStackedWidget* m_timeStackedWidget; // 用于切换的堆叠窗口
    QDateTimeEdit* m_dateTimeEdit;       // “仅一次”模式的控件
    QTimeEdit* m_timeEdit;               // “重复”模式的控件

    QComboBox* m_soundComboBox;
    QSoundEffect* m_soundPlayer;

    Reminder m_resultReminder;
    QPushButton* m_cancelReminderButton;
};
