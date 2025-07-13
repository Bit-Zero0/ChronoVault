#include "AddAnniversaryDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QDateTimeEdit>
#include <QDateEdit>
#include <QComboBox>
#include <QDialogButtonBox>

AddAnniversaryDialog::AddAnniversaryDialog(const QString& initialCategory, QWidget *parent) : QDialog(parent)
{
    setupUi();
    onEventTypeChanged(0);
    connect(m_eventTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AddAnniversaryDialog::onEventTypeChanged);

    // 如果传入了初始分类，就设置它
    if (!initialCategory.isEmpty()) {
        m_categoryEdit->setText(initialCategory);
    }
}

void AddAnniversaryDialog::setupUi() {
    setWindowTitle(tr("添加新项目"));
    setMinimumWidth(400);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    m_formLayout = new QFormLayout();
    m_formLayout->setLabelAlignment(Qt::AlignRight);

    m_titleEdit = new QLineEdit();
    m_categoryEdit = new QLineEdit();
    m_categoryEdit->setPlaceholderText(tr("例如：生日、工作、生活"));
    m_eventTypeComboBox = new QComboBox();
    m_eventTypeComboBox->addItem(tr("倒计时"), static_cast<int>(AnniversaryEventType::Countdown));
    m_eventTypeComboBox->addItem(tr("纪念日"), static_cast<int>(AnniversaryEventType::Anniversary));

    m_targetDateTimeEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(1));
    m_targetDateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
    m_targetDateTimeEdit->setCalendarPopup(true);

    m_formLayout->addRow(tr("标题:"), m_titleEdit);
    m_formLayout->addRow(tr("分类:"), m_categoryEdit);
    m_formLayout->addRow(tr("类型:"), m_eventTypeComboBox);
    m_formLayout->addRow(tr("目标时间:"), m_targetDateTimeEdit);

    // --- 专属于纪念日的选项 ---
    m_anniversaryOptionsWidget = new QWidget();
    QFormLayout* anniversaryOptionsLayout = new QFormLayout(m_anniversaryOptionsWidget);
    anniversaryOptionsLayout->setContentsMargins(0,0,0,0);
    m_originalDateEdit = new QDateEdit(QDate::currentDate());
    m_originalDateEdit->setDisplayFormat("yyyy-MM-dd");
    m_originalDateEdit->setCalendarPopup(true);
    m_recurrenceComboBox = new QComboBox();
    m_recurrenceComboBox->addItem(tr("每年"), static_cast<int>(AnniversaryRecurrence::Yearly));
    m_recurrenceComboBox->addItem(tr("每月"), static_cast<int>(AnniversaryRecurrence::Monthly));
    anniversaryOptionsLayout->addRow(tr("首次日期:"), m_originalDateEdit);
    anniversaryOptionsLayout->addRow(tr("重复规则:"), m_recurrenceComboBox);
    m_formLayout->addRow(m_anniversaryOptionsWidget);


    // --- OK 和 Cancel 按钮 ---
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addLayout(m_formLayout);
    mainLayout->addWidget(buttonBox);
}

void AddAnniversaryDialog::onEventTypeChanged(int index) {
    AnniversaryEventType type = static_cast<AnniversaryEventType>(m_eventTypeComboBox->itemData(index).toInt());
    // 当选择“纪念日”时显示专属选项，否则隐藏
    m_anniversaryOptionsWidget->setVisible(type == AnniversaryEventType::Anniversary);
}

AnniversaryItem AddAnniversaryDialog::getAnniversaryItem() const {
    QString title = m_titleEdit->text();
    AnniversaryEventType type = static_cast<AnniversaryEventType>(m_eventTypeComboBox->currentData().toInt());

    // 步骤 1: 使用当前唯一可用的构造函数，先凭标题创建一个基础对象
    AnniversaryItem item(title);
    item.setCategory(m_categoryEdit->text().trimmed());

    // 步骤 2: 根据用户在对话框中的选择，通过 setter 函数设置其他属性
    if (type == AnniversaryEventType::Anniversary) {
        AnniversaryRecurrence recurrence = static_cast<AnniversaryRecurrence>(m_recurrenceComboBox->currentData().toInt());

        item.setEventType(AnniversaryEventType::Anniversary);
        item.setRecurrence(recurrence);
        item.setOriginalDate(m_originalDateEdit->date());

        // 对于纪念日，目标日期也基于首次日期来设定
        QDateTime targetDateTime(m_originalDateEdit->date(), m_targetDateTimeEdit->time());
        item.setTargetDateTime(targetDateTime);
        item.setReminderDateTime(targetDateTime); // 默认在当天提醒

    } else { // 默认是倒计时
        item.setEventType(AnniversaryEventType::Countdown);
        item.setRecurrence(AnniversaryRecurrence::None);
        item.setTargetDateTime(m_targetDateTimeEdit->dateTime());
        item.setReminderDateTime(m_targetDateTimeEdit->dateTime()); // 默认在倒计时结束时提醒
    }

    return item;
}
