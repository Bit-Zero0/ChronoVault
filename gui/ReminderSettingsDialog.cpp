#include "gui/ReminderSettingsDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QDateTimeEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>

ReminderSettingsDialog::ReminderSettingsDialog(const Reminder& initialReminder, QWidget *parent)
    : QDialog(parent), m_resultReminder(initialReminder) // 将初始值存入成员变量
{
    setupUi();
    // 根据初始值设置UI状态
    m_enabledCheckBox->setChecked(m_resultReminder.isActive());
    m_firstReminderTimeEdit->setDateTime(m_resultReminder.nextReminderTime());
    bool isRecurring = (m_resultReminder.intervalType() != ReminderIntervalType::None);
    m_recurringCheckBox->setChecked(isRecurring);
    if (isRecurring) {
        m_intervalValueSpinBox->setValue(m_resultReminder.intervalValue());
        int comboIndex = m_intervalTypeComboBox->findData(static_cast<int>(m_resultReminder.intervalType()));
        m_intervalTypeComboBox->setCurrentIndex(comboIndex);
    }
    updateControlsState(); // 更新控件的可用状态
}

void ReminderSettingsDialog::setupUi() {
    setWindowTitle(tr("设置提醒"));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    m_enabledCheckBox = new QCheckBox(tr("启用提醒功能"));
    connect(m_enabledCheckBox, &QCheckBox::toggled, this, &ReminderSettingsDialog::updateControlsState);

    m_settingsGroupBox = new QGroupBox(tr("提醒设置"));
    QFormLayout* formLayout = new QFormLayout(m_settingsGroupBox);

    m_firstReminderTimeEdit = new QDateTimeEdit();
    m_firstReminderTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    m_firstReminderTimeEdit->setCalendarPopup(true);

    m_recurringCheckBox = new QCheckBox(tr("循环提醒"));
    connect(m_recurringCheckBox, &QCheckBox::toggled, this, &ReminderSettingsDialog::updateControlsState);

    // --- 循环提醒的子控件 ---
    QHBoxLayout* intervalLayout = new QHBoxLayout();
    m_intervalValueSpinBox = new QSpinBox();
    m_intervalValueSpinBox->setRange(1, 999);
    m_intervalTypeComboBox = new QComboBox();
    m_intervalTypeComboBox->addItem(tr("分钟"), static_cast<int>(ReminderIntervalType::Minutes));
    m_intervalTypeComboBox->addItem(tr("小时"), static_cast<int>(ReminderIntervalType::Hours));
    m_intervalTypeComboBox->addItem(tr("天"), static_cast<int>(ReminderIntervalType::Days));
    intervalLayout->addWidget(new QLabel(tr("每隔")));
    intervalLayout->addWidget(m_intervalValueSpinBox);
    intervalLayout->addWidget(m_intervalTypeComboBox);
    intervalLayout->addStretch();
    // -------------------------

    formLayout->addRow(tr("首次提醒时间:"), m_firstReminderTimeEdit);
    formLayout->addRow(m_recurringCheckBox);
    formLayout->addRow(intervalLayout);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(m_enabledCheckBox);
    mainLayout->addWidget(m_settingsGroupBox);
    mainLayout->addWidget(buttonBox);
}

void ReminderSettingsDialog::updateControlsState() {
    bool enabled = m_enabledCheckBox->isChecked();
    m_settingsGroupBox->setEnabled(enabled);

    if (enabled) {
        bool recurring = m_recurringCheckBox->isChecked();
        m_intervalValueSpinBox->setEnabled(recurring);
        m_intervalTypeComboBox->setEnabled(recurring);
    }
}

// 在用户点击OK时，将UI控件的当前状态保存到结果中
void ReminderSettingsDialog::accept() {
    m_resultReminder.setActive(m_enabledCheckBox->isChecked());
    if (m_enabledCheckBox->isChecked()) {
        m_resultReminder.setNextReminderTime(m_firstReminderTimeEdit->dateTime());
        if (m_recurringCheckBox->isChecked()) {
            m_resultReminder.setIntervalType(static_cast<ReminderIntervalType>(m_intervalTypeComboBox->currentData().toInt()));
            m_resultReminder.setIntervalValue(m_intervalValueSpinBox->value());
        } else {
            m_resultReminder.setIntervalType(ReminderIntervalType::None);
            m_resultReminder.setIntervalValue(0);
        }
    }
    QDialog::accept();
}

Reminder ReminderSettingsDialog::getReminder() const {
    return m_resultReminder;
}
