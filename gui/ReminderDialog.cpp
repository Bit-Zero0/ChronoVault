#include "gui/ReminderDialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QTimeEdit>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QSoundEffect>
#include <QStackedWidget>
#include <QTimer>

ReminderDialog::ReminderDialog(const Reminder& initialReminder, QWidget *parent)
    : QDialog(parent), m_resultReminder(initialReminder)
{
    m_soundPlayer = new QSoundEffect(this);
    setupUi(initialReminder);

    // 如果这是一个已存在的提醒，则延时显示“取消提醒”按钮
    if (initialReminder.isActive()) {
        // 使用单次定时器，在2秒（2000毫秒）后显示按钮
        QTimer::singleShot(2000, this, [this]() {
            if (m_cancelReminderButton) { // 安全检查
                m_cancelReminderButton->setVisible(true);
            }
        });
    }
}

ReminderDialog::~ReminderDialog() = default;

void ReminderDialog::setupUi(const Reminder& initialReminder)
{
    setWindowTitle(tr("设置提醒"));
    setMinimumWidth(350);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* formLayout = new QFormLayout();

    // --- 重复选项 ---
    m_recurrenceComboBox = new QComboBox();
    m_recurrenceComboBox->addItem(tr("仅一次"), QVariant::fromValue(ReminderIntervalType::None));
    m_recurrenceComboBox->addItem(tr("每天"), QVariant::fromValue(ReminderIntervalType::Days));
    connect(m_recurrenceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReminderDialog::onRecurrenceChanged);

    // --- 创建用于切换的 QStackedWidget ---
    m_timeStackedWidget = new QStackedWidget();
    m_dateTimeEdit = new QDateTimeEdit(initialReminder.nextReminderTime());
    m_dateTimeEdit->setCalendarPopup(true);
    m_dateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_dateTimeEdit->setMinimumDateTime(QDateTime::currentDateTime());
    m_timeEdit = new QTimeEdit(initialReminder.baseTime());
    m_timeEdit->setDisplayFormat("HH:mm");
    m_timeStackedWidget->addWidget(m_dateTimeEdit);
    m_timeStackedWidget->addWidget(m_timeEdit);

    // --- 铃声选项 ---
    QHBoxLayout* soundLayout = new QHBoxLayout();
    m_soundComboBox = new QComboBox();
    m_soundComboBox->addItem(tr("默认铃声"), "qrc:/sounds/default.wav");
    m_soundComboBox->addItem(tr("清脆铃声"), "qrc:/sounds/crisp.wav");
    m_soundComboBox->addItem(tr("静音"), "none");
    m_soundComboBox->addItem(tr("自定义..."), "custom");
    connect(m_soundComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ReminderDialog::onSoundSelectionChanged);
    QPushButton* testSoundButton = new QPushButton(tr("试听"));
    connect(testSoundButton, &QPushButton::clicked, this, &ReminderDialog::onTestSoundClicked);
    soundLayout->addWidget(m_soundComboBox, 1);
    soundLayout->addWidget(testSoundButton);

    // --- 按钮 ---
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_cancelReminderButton = new QPushButton(tr("取消提醒"));
    m_cancelReminderButton->setStyleSheet("color: red;");
    m_cancelReminderButton->setVisible(false);
    buttonBox->addButton(m_cancelReminderButton, QDialogButtonBox::DestructiveRole);
    connect(m_cancelReminderButton, &QPushButton::clicked, this, &ReminderDialog::onCancelReminderClicked);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // --- 最终布局 ---
    formLayout->addRow(tr("重复:"), m_recurrenceComboBox);
    formLayout->addRow(tr("提醒时间:"), m_timeStackedWidget);
    formLayout->addRow(tr("铃声:"), soundLayout);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);

    // 根据初始状态设置正确的UI显示
    if (initialReminder.intervalType() == ReminderIntervalType::None) {
        m_recurrenceComboBox->setCurrentIndex(0);
        onRecurrenceChanged(0);
    } else {
        m_recurrenceComboBox->setCurrentIndex(m_recurrenceComboBox->findData(QVariant::fromValue(initialReminder.intervalType())));
        onRecurrenceChanged(m_recurrenceComboBox->currentIndex());
    }
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
    ReminderIntervalType type = m_recurrenceComboBox->itemData(index).value<ReminderIntervalType>();
    if (type == ReminderIntervalType::None) {
        m_timeStackedWidget->setCurrentWidget(m_dateTimeEdit); // 切换到“年月日时分”
    } else {
        m_timeStackedWidget->setCurrentWidget(m_timeEdit);     // 切换到“时分”
    }
}

Reminder ReminderDialog::getReminder() const
{
    return m_resultReminder;
}

// 实现“取消提醒”按钮的槽函数
void ReminderDialog::onCancelReminderClicked()
{
    // 1. 将内部的 m_resultReminder 设置为“非激活”状态
    m_resultReminder.setActive(false);

    // 2. 调用 accept() 来关闭对话框。
    // 这很重要，因为它表示用户做出了一个有效的选择（即“取消提醒”），
    // 我们需要将这个结果传递出去。
    accept();
}


void ReminderDialog::accept()
{
    // 仅在用户点击"OK"按钮时，我们才采集UI上的新设置
    // 如果是点击"取消提醒"，则 m_resultReminder 已经在槽函数中被设为 inactive
    if (sender() != m_cancelReminderButton) {
        m_resultReminder = getReminderFromUi(); // (我们需要在下一步创建一个辅助函数)
    }
    QDialog::accept();
}

Reminder ReminderDialog::getReminderFromUi() const
{
    Reminder newReminder;
    newReminder.setActive(true);

    ReminderIntervalType type = m_recurrenceComboBox->currentData().value<ReminderIntervalType>();
    newReminder.setIntervalType(type);

    if (type == ReminderIntervalType::None) {
        newReminder.setNextReminderTime(m_dateTimeEdit->dateTime());
    } else {
        QTime baseTime = m_timeEdit->time();
        newReminder.setBaseTime(baseTime);
        newReminder.setIntervalValue(1);

        QDateTime nextTime(QDate::currentDate(), baseTime);
        if (nextTime <= QDateTime::currentDateTime()) {
            nextTime = nextTime.addDays(1);
        }
        newReminder.setNextReminderTime(nextTime);
    }

    newReminder.setSoundPath(m_soundComboBox->currentData().toString());

    return newReminder;
}
