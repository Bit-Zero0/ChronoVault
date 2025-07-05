#include "gui/AddTodoDialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QTextEdit>
#include <QDateTimeEdit>
#include <QDialogButtonBox>

AddTodoDialog::AddTodoDialog(QWidget *parent) : QDialog(parent) {
    setupUi();
    // 连接 "OK" 按钮的 accepted 信号到对话框的 accept 槽
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    // 连接 "Cancel" 按钮的 rejected 信号到对话框的 reject 槽
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void AddTodoDialog::setupUi() {
    setWindowTitle("Add New Todo");
    setMinimumWidth(350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();

    m_titleLineEdit = new QLineEdit();
    m_descriptionTextEdit = new QTextEdit();
    m_descriptionTextEdit->setPlaceholderText("Optional details...");
    m_descriptionTextEdit->setMaximumHeight(100); // 避免描述框过大

    m_dueDateEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(1));
    m_dueDateEdit->setCalendarPopup(true);
    m_dueDateEdit->setDisplayFormat("yyyy-MM-dd hh:mm");

    formLayout->addRow("Title:", m_titleLineEdit);
    formLayout->addRow("Description:", m_descriptionTextEdit);
    formLayout->addRow("Due Date:", m_dueDateEdit);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(m_buttonBox);
}

// 这个函数是关键，它将UI控件中的数据打包成一个 TodoItem 对象
TodoItem AddTodoDialog::getNewTodoItemData() const {
    QString title = m_titleLineEdit->text();
    QString description = m_descriptionTextEdit->toPlainText();
    QDateTime dueDate = m_dueDateEdit->dateTime();

    return TodoItem(title, description, dueDate);
}
