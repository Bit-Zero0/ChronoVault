#pragma once

#include <QDialog>
#include <QUuid>
#include <QString>
#include <QList>
#include "core/TodoList.h" // 包含 TodoList 定义

QT_BEGIN_NAMESPACE
class QComboBox;
QT_END_NAMESPACE

class TodoListSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    // 构造函数接收一个当前所有列表的引用，用于填充下拉菜单
    explicit TodoListSelectionDialog(const QList<TodoList>& lists, QWidget *parent = nullptr);

    // --- 公共接口，用于获取用户的最终选择 ---

    // 如果用户选择了一个已存在的列表，返回其ID
    QUuid getSelectedListId() const;
    // 如果用户创建了一个新列表，返回其名称
    QString getNewListName() const;
    // 返回用户是否创建了新列表
    bool isNewListCreated() const;

private slots:
    void onNewListButtonClicked();

private:
    void setupUi(const QList<TodoList>& lists);
    void accept() override;

    // UI 控件
    QComboBox* m_listComboBox;

    // 用于存储结果的成员变量
    QUuid m_selectedListId;
    QString m_newListName;
    bool m_isNewList = false;
};
