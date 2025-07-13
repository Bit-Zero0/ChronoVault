#pragma once

#include <QDialog>
#include "core/Reminder.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QDateTimeEdit;
class QSpinBox;
class QComboBox;
class QGroupBox;
QT_END_NAMESPACE

class ReminderSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    // 构造函数接收一个初始的 Reminder 对象，用于设置对话框的默认状态
    explicit ReminderSettingsDialog(const Reminder& initialReminder, QWidget *parent = nullptr);

    // 获取用户最终配置好的 Reminder 对象
    Reminder getReminder() const;

private slots:
    // 根据复选框的状态，动态更新UI控件的可用性
    void updateControlsState();

private:
    void setupUi();
    void accept() override;

    // UI 控件
    QCheckBox* m_enabledCheckBox;
    QGroupBox* m_settingsGroupBox;
    QDateTimeEdit* m_firstReminderTimeEdit;
    QCheckBox* m_recurringCheckBox;
    QSpinBox* m_intervalValueSpinBox;
    QComboBox* m_intervalTypeComboBox;

    // 用于存储结果
    Reminder m_resultReminder;
};
