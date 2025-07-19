#include "gui/ReminderDialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QTimeEdit>
#include <QComboBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QSoundEffect>

ReminderDialog::ReminderDialog(const Reminder& initialReminder, QWidget *parent)
    : QDialog(parent), m_resultReminder(initialReminder)
{
    m_soundPlayer = new QSoundEffect(this);
    setupUi(initialReminder);
}

ReminderDialog::~ReminderDialog() = default;

void ReminderDialog::setupUi(const Reminder& initialReminder)
{
    setWindowTitle(tr("设置提醒"));
    setMinimumWidth(350);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* formLayout = new QFormLayout();

    m_timeEdit = new QTimeEdit(initialReminder.nextReminderTime().time());
    m_timeEdit->setDisplayFormat("HH:mm");

    // --- 重复选项 ---
    m_recurrenceComboBox = new QComboBox();
    m_recurrenceComboBox->addItem(tr("仅一次"), QVariant::fromValue(ReminderIntervalType::None));
    m_recurrenceComboBox->addItem(tr("每天"), QVariant::fromValue(ReminderIntervalType::Days));
    // 您可以添加更多选项，如每周、每月等
    connect(m_recurrenceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReminderDialog::onRecurrenceChanged);

    // --- 铃声选项 ---
    QHBoxLayout* soundLayout = new QHBoxLayout();
    m_soundComboBox = new QComboBox();
    // 预设铃声，假设您在资源文件中有这些声音
    m_soundComboBox->addItem(tr("默认铃声"), "qrc:/sounds/default.wav");
    m_soundComboBox->addItem(tr("清脆铃声"), "qrc:/sounds/crisp.wav");
    m_soundComboBox->addItem(tr("静音"), "none");
    m_soundComboBox->addItem(tr("自定义..."), "custom");
    connect(m_soundComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReminderDialog::onSoundSelectionChanged);

    QPushButton* testSoundButton = new QPushButton(tr("试听"));
    connect(testSoundButton, &QPushButton::clicked, this, &ReminderDialog::onTestSoundClicked);
    soundLayout->addWidget(m_soundComboBox, 1);
    soundLayout->addWidget(testSoundButton);

    formLayout->addRow(tr("提醒时间:"), m_timeEdit);
    formLayout->addRow(tr("重复:"), m_recurrenceComboBox);
    formLayout->addRow(tr("铃声:"), soundLayout);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
}

void ReminderDialog::onSoundSelectionChanged(int index)
{
    if (m_soundComboBox->itemData(index).toString() == "custom") {
        QString filePath = QFileDialog::getOpenFileName(this, tr("选择铃声文件"), "", tr("音频文件 (*.wav *.mp3)"));
        if (!filePath.isEmpty()) {
            // 移除旧的自定义选项（如果有），并添加新的
            for (int i = 0; i < m_soundComboBox->count(); ++i) {
                if (m_soundComboBox->itemData(i).toString().startsWith("file://")) {
                    m_soundComboBox->removeItem(i);
                    break;
                }
            }
            m_soundComboBox->insertItem(index, QFileInfo(filePath).fileName(), QUrl::fromLocalFile(filePath).toString());
            m_soundComboBox->setCurrentIndex(index);
        } else {
            m_soundComboBox->setCurrentIndex(0); // 用户取消选择，返回默认
        }
    }
}

void ReminderDialog::onTestSoundClicked()
{
    QString path = m_soundComboBox->currentData().toString();
    if (path != "none" && !path.isEmpty()) {
        m_soundPlayer->setSource(QUrl(path));
        m_soundPlayer->play();
    }
}

void ReminderDialog::onRecurrenceChanged(int index)
{
    // 未来可以根据选择的重复类型，动态改变UI，比如选择“每周”时，显示周一到周日的复选框
}

Reminder ReminderDialog::getReminder() const
{
    Reminder newReminder;
    newReminder.setActive(true);

    // 设置时间
    QDateTime nextTime(QDate::currentDate(), m_timeEdit->time());
    if (nextTime <= QDateTime::currentDateTime()) {
        nextTime = nextTime.addDays(1); // 如果设置的时间已过，则为明天
    }
    newReminder.setNextReminderTime(nextTime);

    // 设置重复规则
    ReminderIntervalType type = m_recurrenceComboBox->currentData().value<ReminderIntervalType>();
    newReminder.setIntervalType(type);
    if (type == ReminderIntervalType::Days) {
        newReminder.setIntervalValue(1); // “每天”就是间隔为1天
    }

    // 设置铃声
    newReminder.setSoundPath(m_soundComboBox->currentData().toString());

    return newReminder;
}
