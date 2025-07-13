#include "gui/TodoListSelectionDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QToolButton>
#include <QDialogButtonBox>
#include <QInputDialog>

TodoListSelectionDialog::TodoListSelectionDialog(const QList<TodoList>& lists, QWidget *parent)
    : QDialog(parent)
{
    setupUi(lists);
}

void TodoListSelectionDialog::setupUi(const QList<TodoList>& lists) {
    setWindowTitle(tr("添加到待办列表"));
    setMinimumWidth(350);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);

    QLabel* promptLabel = new QLabel(tr("请选择要将此任务添加到的列表:"));

    // --- 下拉菜单和新建按钮的水平布局 ---
    QHBoxLayout* selectionLayout = new QHBoxLayout();
    m_listComboBox = new QComboBox();
    // 填充现有的列表到下拉菜单中
    for (const auto& list : lists) {
        // 我们将列表名称作为显示文本，将列表的UUID作为关联数据
        m_listComboBox->addItem(list.name, QVariant::fromValue(list.id));
    }

    QToolButton* newListButton = new QToolButton();
    newListButton->setText(tr("新建列表"));
    connect(newListButton, &QToolButton::clicked, this, &TodoListSelectionDialog::onNewListButtonClicked);

    selectionLayout->addWidget(m_listComboBox, 1); // 占据更多空间
    selectionLayout->addWidget(newListButton);

    // --- OK 和 Cancel 按钮 ---
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    mainLayout->addWidget(promptLabel);
    mainLayout->addLayout(selectionLayout);
    mainLayout->addWidget(buttonBox);
}

void TodoListSelectionDialog::onNewListButtonClicked() {
    bool ok;
    QString newName = QInputDialog::getText(this, tr("新建待办列表"), tr("请输入新列表的名称:"), QLineEdit::Normal, "", &ok);
    if (ok && !newName.isEmpty()) {
        // 在下拉菜单中添加一个代表“新列表”的临时选项
        m_listComboBox->addItem(QString(tr("【新】%1")).arg(newName));
        m_listComboBox->setCurrentIndex(m_listComboBox->count() - 1);
        m_newListName = newName;
        m_isNewList = true;
    }
}

// --- 实现结果获取函数 ---

QUuid TodoListSelectionDialog::getSelectedListId() const {
    return m_selectedListId;
}

QString TodoListSelectionDialog::getNewListName() const {
    return m_newListName;
}

bool TodoListSelectionDialog::isNewListCreated() const {
    // 重写 accept() 来在点击OK时处理最终选择
    // 这里我们直接在accept()中处理，所以这个函数可以简单一些
    return m_isNewList;
}

// 【重要】重写 accept() 函数，在用户点击"OK"时，解析最终的选择
void TodoListSelectionDialog::accept() {
    if (m_isNewList && m_listComboBox->currentText().startsWith(tr("【新】"))) {
        // 用户最终选择的是一个新创建的列表
        m_selectedListId = QUuid(); // 确保ID为空
    } else {
        // 用户选择了一个已存在的列表
        m_selectedListId = m_listComboBox->currentData().toUuid();
        m_newListName.clear();
        m_isNewList = false;
    }

    // 调用基类的 accept() 来关闭对话框并返回 QDialog::Accepted
    QDialog::accept();
}
