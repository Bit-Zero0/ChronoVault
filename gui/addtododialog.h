#pragma once
#include <QDialog>
#include "core/TodoItem.h" // 引入我们的数据模型

// 前向声明 UI 控件
QT_BEGIN_NAMESPACE
class QLineEdit;
class QTextEdit;
class QDateTimeEdit;
class QDialogButtonBox;
QT_END_NAMESPACE

class AddTodoDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddTodoDialog(QWidget *parent = nullptr);

    // 公共接口，用于从对话框外部获取用户输入的数据
    TodoItem getNewTodoItemData() const;

private:
    void setupUi();

    QLineEdit *m_titleLineEdit;
    QTextEdit *m_descriptionTextEdit;
    QDateTimeEdit *m_dueDateEdit;
    QDialogButtonBox *m_buttonBox;
};
