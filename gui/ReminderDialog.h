#pragma once

#include <QDialog>
#include "core/Reminder.h"

QT_BEGIN_NAMESPACE
class QTimeEdit;
class QComboBox;
class QSoundEffect; // 【新增】
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

private:
    void setupUi(const Reminder& initialReminder);

    QTimeEdit* m_timeEdit;
    QComboBox* m_recurrenceComboBox;
    QComboBox* m_soundComboBox;
    QSoundEffect* m_soundPlayer; // 用于试听声音

    Reminder m_resultReminder;
};
