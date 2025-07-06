#include "gui/TaskDetailWidget.h"
#include "gui/SubTaskItemWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QFrame>
#include <QMenu>
#include <QInputDialog>
#include <QToolButton>
#include <QCursor>
#include <QEvent>
#include <QCalendarWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QDateTimeEdit>

TaskDetailWidget::TaskDetailWidget(QWidget* parent)
    : QWidget(parent), m_currentTask("")
{
    setupUi();

    connect(m_titleLineEdit, &QLineEdit::editingFinished, this, &TaskDetailWidget::onTitleEditingFinished);
    //connect(m_notesTextEdit, &QTextEdit::textChanged, this, &TaskDetailWidget::onNotesChanged);
    m_notesTextEdit->installEventFilter(this);
    connect(m_completedCheckBox, &QCheckBox::stateChanged, this, &TaskDetailWidget::onCompletedStateChanged);
    connect(m_deleteButton, &QPushButton::clicked, this, &TaskDetailWidget::onDeleteButtonClicked);
    connect(m_addSubTaskLineEdit, &QLineEdit::returnPressed, this, &TaskDetailWidget::onAddSubTaskLineEditReturnPressed);
    connect(m_closeButton, &QToolButton::clicked, this, &TaskDetailWidget::closeRequested);
    connect(m_dueDateButton, &QPushButton::clicked, this, &TaskDetailWidget::onDueDateButtonClicked);
}

void TaskDetailWidget::setupUi() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    QHBoxLayout* topLayout = new QHBoxLayout();
    QHBoxLayout* titleLayout = new QHBoxLayout();
    m_completedCheckBox = new QCheckBox();
    m_titleLineEdit = new QLineEdit();
    m_titleLineEdit->setStyleSheet("border: none; background: transparent; font-size: 16px; font-weight: bold;");
    titleLayout->addWidget(m_completedCheckBox);
    titleLayout->addWidget(m_titleLineEdit);
    m_closeButton = new QToolButton();
    m_closeButton->setText("✕");
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setAutoRaise(true);
    topLayout->addLayout(titleLayout, 1);
    topLayout->addWidget(m_closeButton);
    mainLayout->addLayout(topLayout);

    m_subTasksListWidget = new QListWidget();
    m_subTasksListWidget->setStyleSheet("border: none;");
    m_subTasksListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_addSubTaskLineEdit = new QLineEdit();
    m_addSubTaskLineEdit->setPlaceholderText("＋ 添加步骤");
    mainLayout->addWidget(m_subTasksListWidget);
    mainLayout->addWidget(m_addSubTaskLineEdit);

    m_remindButton = new QPushButton("提醒我");
    m_dueDateButton = new QPushButton("添加截止日期");
    mainLayout->addWidget(m_remindButton);
    mainLayout->addWidget(m_dueDateButton);

    m_notesTextEdit = new QTextEdit();
    m_notesTextEdit->setPlaceholderText("添加备注");
    m_notesTextEdit->setFixedHeight(120);
    mainLayout->addWidget(m_notesTextEdit);

    mainLayout->addStretch();

    QFrame* footerFrame = new QFrame();
    QHBoxLayout* footerLayout = new QHBoxLayout(footerFrame);
    footerLayout->setContentsMargins(0, 0, 0, 0);
    m_creationDateLabel = new QLabel();
    m_deleteButton = new QPushButton("删除");
    m_deleteButton->setCursor(Qt::PointingHandCursor);
    m_deleteButton->setStyleSheet("border: none; color: red;");
    footerLayout->addWidget(m_creationDateLabel);
    footerLayout->addStretch();
    footerLayout->addWidget(m_deleteButton);
    mainLayout->addWidget(footerFrame);

    connect(m_subTasksListWidget, &QListWidget::itemDoubleClicked, this, &TaskDetailWidget::onSubTaskDoubleClicked);
    connect(m_subTasksListWidget, &QListWidget::customContextMenuRequested, this, &TaskDetailWidget::showSubTaskContextMenu);
}


bool TaskDetailWidget::eventFilter(QObject* watched, QEvent* event)
{
    // 检查事件是否发生在我们的备注输入框上
    if (watched == m_notesTextEdit) {
        // 检查事件类型是否是“失去焦点”
        if (event->type() == QEvent::FocusOut) {
            // 如果是，则调用我们之前写好的 onNotesChanged 函数来处理保存逻辑
            onNotesChanged();
        }
    }
    // 将事件交还给父类进行默认处理
    return QWidget::eventFilter(watched, event);
}



void TaskDetailWidget::displayTask(const TodoItem& task) {
    m_currentTask = task;
    bool oldSignalsState = signalsBlocked();
    blockSignals(true);

    m_titleLineEdit->setText(task.title());
    m_completedCheckBox->setChecked(task.isCompleted());
    m_notesTextEdit->setText(task.description());
    if (task.dueDate().isValid()) {
        m_dueDateButton->setText("截止于 " + task.dueDate().toString("yyyy-MM-dd HH:mm")); // 显示时间
            m_dueDateButton->setStyleSheet("color: blue;");
    } else {
        m_dueDateButton->setText("添加截止日期");
        m_dueDateButton->setStyleSheet("");
    }

    m_creationDateLabel->setText("创建于 " + task.creationDate().toString("yyyy年M月d日"));


    m_subTasksListWidget->clear();
    for(const auto& subtask : task.subTasks()) {
        QListWidgetItem* item = new QListWidgetItem(m_subTasksListWidget);
        SubTaskItemWidget* widget = new SubTaskItemWidget(subtask, m_subTasksListWidget);

        item->setSizeHint(widget->sizeHint());
        m_subTasksListWidget->addItem(item);
        m_subTasksListWidget->setItemWidget(item, widget);

        // 连接信号
        connect(widget, &SubTaskItemWidget::subTaskStateChanged, this, [this](const QUuid& subTaskId, bool isCompleted){
            emit subTaskStateChanged(m_currentTask.id(), subTaskId, isCompleted);
        });
        connect(widget, &SubTaskItemWidget::optionsMenuRequested, this, &TaskDetailWidget::showSubTaskMenu);
        // 【新增连接】处理来自步骤项的更新请求
        connect(widget, &SubTaskItemWidget::subTaskUpdated, this, &TaskDetailWidget::handleSubTaskDataUpdate);
    }

    blockSignals(oldSignalsState);
    updateSubTasksListHeight();
}

void TaskDetailWidget::handleSubTaskDataUpdate(const SubTask& updatedSubTask) {
    // 直接将更新请求向上传递给 MainWindow
    emit subTaskUpdated(m_currentTask.id(), updatedSubTask);
}


QUuid TaskDetailWidget::getCurrentTaskId() const {
    // 如果当前标题为空，可能代表没有有效任务，返回空UUID
    if (m_titleLineEdit->text().isEmpty() && !m_currentTask.id().isNull()){
        return m_currentTask.id();
    }
    return m_currentTask.id();
}

void TaskDetailWidget::onTitleEditingFinished() {
    if (m_currentTask.title() != m_titleLineEdit->text()) {
        m_currentTask.setTitle(m_titleLineEdit->text());
        emit taskUpdated(m_currentTask);
    }
}

void TaskDetailWidget::onNotesChanged() {
    if (m_currentTask.description() != m_notesTextEdit->toPlainText()) {
        m_currentTask.setDescription(m_notesTextEdit->toPlainText());
        emit taskUpdated(m_currentTask);
    }
}

void TaskDetailWidget::onCompletedStateChanged(int state) {
    bool isCompleted = (state == Qt::Checked);
    if (m_currentTask.isCompleted() != isCompleted) {
        m_currentTask.setCompleted(isCompleted);
        emit taskUpdated(m_currentTask);
    }
}

void TaskDetailWidget::onAddSubTaskLineEditReturnPressed() {
    QString title = m_addSubTaskLineEdit->text().trimmed();
    if (!title.isEmpty()) {
        emit addSubTask(m_currentTask.id(), title);
        m_addSubTaskLineEdit->clear();
    }
}

void TaskDetailWidget::onDeleteButtonClicked() {
    emit taskDeleted(m_currentTask.id());
}

void TaskDetailWidget::updateSubTasksListHeight() {
    int count = m_subTasksListWidget->count();
    if (count == 0) {
        m_subTasksListWidget->setVisible(false);
        return;
    }
    m_subTasksListWidget->setVisible(true);
    int itemHeight = m_subTasksListWidget->sizeHintForRow(0);
    int spacing = m_subTasksListWidget->spacing();
    int contentHeight = (itemHeight + spacing) * count - spacing;
    contentHeight += 2 * m_subTasksListWidget->frameWidth();
    int maxHeight = 200;
    int finalHeight = qMin(contentHeight, maxHeight);
    m_subTasksListWidget->setFixedHeight(finalHeight);
}

void TaskDetailWidget::onSubTaskDoubleClicked(QListWidgetItem* item) {
    if (!item) return;
    // 直接调用 item 对应 widget 的方法
    if (auto* widget = qobject_cast<SubTaskItemWidget*>(m_subTasksListWidget->itemWidget(item))) {
        widget->enterEditMode();
    }
}

void TaskDetailWidget::showSubTaskContextMenu(const QPoint& pos) {
    QListWidgetItem* item = m_subTasksListWidget->itemAt(pos);
    if (!item) return;
    auto* widget = qobject_cast<SubTaskItemWidget*>(m_subTasksListWidget->itemWidget(item));
    if (widget) {
        showSubTaskMenu(m_subTasksListWidget->mapToGlobal(pos), widget->getSubTask());
    }
}

// 【重要】添加缺失的函数实现
void TaskDetailWidget::handleSubTaskOptionsClicked(const QPoint& pos, const SubTask& subTask)
{
    showSubTaskMenu(pos, subTask);
}

void TaskDetailWidget::showSubTaskMenu(const QPoint& pos, const SubTask& subTask) {
    // 步骤1: 在步骤列表中找到与传入的 subTask 数据匹配的那个自定义 widget。
    //        这对于执行“修改”操作（调用 enterEditMode）至关重要。
    SubTaskItemWidget* widget = nullptr;
    for(int i = 0; i < m_subTasksListWidget->count(); ++i) {
        QListWidgetItem* item = m_subTasksListWidget->item(i);
        auto* currentWidget = qobject_cast<SubTaskItemWidget*>(m_subTasksListWidget->itemWidget(item));
        if (currentWidget && currentWidget->getSubTask().id == subTask.id) {
            widget = currentWidget;
            break;
        }
    }

    // 如果出于某种原因找不到对应的widget，则不显示菜单。
    if (!widget) {
        return;
    }

    // 步骤2: 创建并配置上下文菜单
    QMenu contextMenu(this);
    QAction* modifyAction = contextMenu.addAction("修改步骤");
    QAction* promoteAction = contextMenu.addAction("提升为任务");
    contextMenu.addSeparator(); // 添加一条分割线
    QAction* deleteAction = contextMenu.addAction("删除步骤");

    // 步骤3: 在指定位置显示菜单，并等待用户做出选择
    QAction* selectedAction = contextMenu.exec(pos);

    // 步骤4: 根据用户的选择，执行相应的操作
    if (selectedAction == modifyAction) {
        // 对于“修改”，我们调用 widget 自己的 enterEditMode() 函数来实现原地编辑
        widget->enterEditMode();
    }
    else if (selectedAction == deleteAction) {
        // 对于“删除”，我们发射一个信号，通知上层(MainWindow)来处理
        emit subTaskDeleted(m_currentTask.id(), subTask.id);
    }
    else if (selectedAction == promoteAction) {
        // 对于“提升”，我们也发射一个信号，通知上层(MainWindow)来处理
        emit subTaskPromoted(m_currentTask.id(), subTask.id);
    }
}

// --- 在 TaskDetailWidget.cpp 文件末尾，添加新槽函数的实现 ---
void TaskDetailWidget::onDueDateButtonClicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("选择截止日期和时间");

    // 1. 使用 QDateTimeEdit 代替 QCalendarWidget
    QDateTimeEdit* dateTimeEdit = new QDateTimeEdit();
    dateTimeEdit->setCalendarPopup(true); // 允许弹出日历选择日期
    dateTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm"); // 设置显示格式

    // 如果当前任务已有截止日期，则在控件上显示它
    if (m_currentTask.dueDate().isValid()) {
        dateTimeEdit->setDateTime(m_currentTask.dueDate());
    } else {
        // 默认显示当前日期时间的结束
        QDateTime now = QDateTime::currentDateTime();
        dateTimeEdit->setDateTime(QDateTime(now.date(), QTime(23, 59)));
    }

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Reset);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttonBox->button(QDialogButtonBox::Reset), &QPushButton::clicked, [&](){
        emit dueDateChanged(m_currentTask.id(), QDateTime());
        dialog.accept();
    });

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->addWidget(dateTimeEdit);
    layout->addWidget(buttonBox);

    if (dialog.exec() == QDialog::Accepted) {
        QDateTime selectedDateTime = dateTimeEdit->dateTime();
        emit dueDateChanged(m_currentTask.id(), selectedDateTime);
    }
}
