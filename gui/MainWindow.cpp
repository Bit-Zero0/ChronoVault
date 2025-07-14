#include "gui/MainWindow.h"
#include "services/TodoService.h"
#include "services/AnniversaryService.h"
#include "gui/AnniversaryDetailView.h"
#include "gui/AnniversaryItemWidget.h"
#include "gui/TaskDetailWidget.h"
#include "gui/AddAnniversaryDialog.h"
#include "gui/TodoListSelectionDialog.h"
#include "gui/ReminderSettingsDialog.h"
#include "gui/AddMomentDialog.h"
#include "gui/TodoListItemWidget.h"
#include "gui/TodoListNameWidget.h"
#include "gui/CompletedHeaderWidget.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QToolButton>
#include <QButtonGroup>
#include <QStackedWidget>
#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>
#include <QDebug>
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    m_todoService = TodoService::instance();
    m_anniversaryService = AnniversaryService::instance();
    m_isCompletedSectionExpanded = false;
    setupUi();
    setupConnections();
    refreshListView();
    if (m_listSelectionWidget->count() > 0) {
        m_listSelectionWidget->setCurrentRow(0);
    }
    refreshAnniversaryCategories();
    refreshAnniversaryView();
}

MainWindow::~MainWindow() {}


//void MainWindow::setupUi() {
//    // 1. 初始化主窗口基本属性
//    setWindowTitle(tr("ChronoVault"));
//    resize(1080, 720); // 推荐一个稍大的默认尺寸

//    //=========================================================================
//    // 2. 构建左侧面板 (Left Panel)
//    //=========================================================================
//    m_leftPanel = new QWidget();
//    QVBoxLayout* leftPanelLayout = new QVBoxLayout(m_leftPanel);
//    leftPanelLayout->setContentsMargins(0, 0, 0, 5);
//    leftPanelLayout->setSpacing(0);

//    // 2a. 顶部模块切换器 (Module Switcher)
//    QWidget* moduleSwitcherWidget = new QWidget();
//    moduleSwitcherWidget->setStyleSheet("background-color: #f3f3f3; border-bottom: 1px solid #e0e0e0;");
//    m_moduleSwitcherLayout = new QHBoxLayout(moduleSwitcherWidget);
//    m_moduleSwitcherLayout->setContentsMargins(5, 5, 5, 5);
//    m_moduleSwitcherLayout->setSpacing(5);
//    m_moduleButtonGroup = new QButtonGroup(this);
//    m_moduleButtonGroup->setExclusive(true);

//    QToolButton* todoButton = new QToolButton();
//    todoButton->setText(tr("待办"));
//    todoButton->setCheckable(true);
//    todoButton->setChecked(true);
//    todoButton->setStyleSheet("QToolButton { border: 1px solid #ccc; padding: 5px 15px; background-color: white; border-radius: 4px; } QToolButton:checked { background-color: #cce5ff; border-color: #0078d4; }");
//    m_moduleSwitcherLayout->addWidget(todoButton);
//    m_moduleButtonGroup->addButton(todoButton, 0); // ID 0 for Todo module

//    QToolButton* anniversaryButton = new QToolButton();
//    anniversaryButton->setText(tr("纪念日"));
//    anniversaryButton->setCheckable(true);
//    anniversaryButton->setStyleSheet("QToolButton { border: 1px solid #ccc; padding: 5px 15px; background-color: white; border-radius: 4px; } QToolButton:checked { background-color: #cce5ff; border-color: #0078d4; }");
//    m_moduleSwitcherLayout->addWidget(anniversaryButton);
//    m_moduleButtonGroup->addButton(anniversaryButton, 1); // ID 1 for Anniversary module
//    m_moduleSwitcherLayout->addStretch();

//    // 2b. 左侧内容堆叠窗口 (for lists and categories)
//    m_leftContentStack = new QStackedWidget();

//    //    -- Page 0: 待办模块的左侧面板 --
//    m_todoListPanel = new QWidget();
//    QVBoxLayout* todoListLayout = new QVBoxLayout(m_todoListPanel);
//    todoListLayout->setContentsMargins(0, 8, 0, 0);
//    todoListLayout->setSpacing(0);
//    QLabel* listHeaderLabel = new QLabel(tr("列表栏"));
//    listHeaderLabel->setStyleSheet("font-size: 16px; font-weight: bold; padding: 0 8px 4px 8px;");
//    m_listSelectionWidget = new QListWidget();
//    m_listSelectionWidget->setContextMenuPolicy(Qt::CustomContextMenu);
//    m_listSelectionWidget->setStyleSheet("QListWidget { border: none; font-size: 14px; } QListWidget::item { height: 32px; padding-left: 8px; }");
//    m_addNewListButton = new QToolButton();
//    m_addNewListButton->setText(tr("＋ 新建列表"));
//    m_addNewListButton->setAutoRaise(true);
//    m_addNewListButton->setStyleSheet("QToolButton { border: none; padding: 8px; text-align: left; color: #0078d4; }");
//    todoListLayout->addWidget(listHeaderLabel);
//    todoListLayout->addWidget(m_listSelectionWidget, 1);
//    todoListLayout->addWidget(m_addNewListButton);
//    m_leftContentStack->addWidget(m_todoListPanel);

//    //    -- Page 1: 纪念日模块的左侧面板 --
//    m_anniversaryPanel = new QWidget();
//    QVBoxLayout* anniversaryLayout = new QVBoxLayout(m_anniversaryPanel);
//    anniversaryLayout->setContentsMargins(0, 8, 0, 0);

//    // 【核心修改】创建一个水平布局来放置标题和“+”按钮
//    QHBoxLayout* anniversaryHeaderLayout = new QHBoxLayout();
//    anniversaryHeaderLayout->setContentsMargins(8, 0, 8, 4);
//    QLabel* anniversaryHeader = new QLabel(tr("纪念日分类"));
//    anniversaryHeader->setStyleSheet("font-size: 16px; font-weight: bold;");
//    m_addAnniversaryCategoryButton = new QToolButton();
//    m_addAnniversaryCategoryButton->setText("+");
//    m_addAnniversaryCategoryButton->setCursor(Qt::PointingHandCursor);
//    m_addAnniversaryCategoryButton->setStyleSheet("QToolButton { border: none; font-size: 18px; font-weight: bold; color: #0078d4; }");
//    anniversaryHeaderLayout->addWidget(anniversaryHeader);
//    anniversaryHeaderLayout->addStretch();
//    anniversaryHeaderLayout->addWidget(m_addAnniversaryCategoryButton);

//    m_anniversaryCategoryWidget = new QListWidget();
//    m_anniversaryCategoryWidget->setContextMenuPolicy(Qt::CustomContextMenu);
//    m_anniversaryCategoryWidget->setStyleSheet("QListWidget { border: none; }");

//    // 将新的头部布局和列表添加到垂直布局中
//    anniversaryLayout->addLayout(anniversaryHeaderLayout);
//    anniversaryLayout->addWidget(m_anniversaryCategoryWidget, 1);
//    m_leftContentStack->addWidget(m_anniversaryPanel);


//    // 2c. 左下角设置按钮
//    m_settingsButton = new QToolButton();
//    m_settingsButton->setText(tr("⚙️ 设置与账户"));
//    m_settingsButton->setAutoRaise(true);
//    m_settingsButton->setStyleSheet("QToolButton { border-top: 1px solid #e0e0e0; padding: 8px; text-align: left; }");

//    // 2d. 组合左侧面板
//    leftPanelLayout->addWidget(moduleSwitcherWidget);
//    leftPanelLayout->addWidget(m_leftContentStack, 1);
//    leftPanelLayout->addWidget(m_settingsButton);

//    //=========================================================================
//    // 3. 构建右侧内容面板 (Right Panel)
//    //=========================================================================
//    m_rightContentStack = new QStackedWidget();

//    //    -- Page 0: 待办模块的右侧主视图 --
//    m_taskPanel = new QWidget();
//    QVBoxLayout* taskLayout = new QVBoxLayout(m_taskPanel);
//    taskLayout->setContentsMargins(15, 8, 15, 8);
//    m_currentListTitleLabel = new QLabel(tr("请选择一个列表"));
//    m_currentListTitleLabel->setStyleSheet("font-size: 24px; font-weight: bold;");
//    m_taskItemsWidget = new QListWidget();
//    m_taskItemsWidget->setContextMenuPolicy(Qt::CustomContextMenu);
//    m_taskItemsWidget->setSpacing(2);
//    m_taskItemsWidget->setStyleSheet("QListWidget { border: none; }");
//    m_addTodoLineEdit = new QLineEdit();
//    m_addTodoLineEdit->setPlaceholderText(tr("＋ 添加任务，回车即可保存"));
//    m_addTodoLineEdit->setStyleSheet(
//        "QLineEdit { border: 1px solid #dcdcdc; border-radius: 4px; padding: 10px; background-color: #ffffff; font-size: 14px;}"
//        "QLineEdit:focus { border: 1px solid #0078d4;}"
//        );
//    taskLayout->addWidget(m_currentListTitleLabel);
//    taskLayout->addWidget(m_taskItemsWidget, 1);
//    taskLayout->addWidget(m_addTodoLineEdit);
//    m_rightContentStack->addWidget(m_taskPanel);

//    //    -- Page 1: 纪念日模块的右侧主视图 --
//    m_anniversaryRightPanel = new QWidget();
//    m_anniversaryRightPanel->setStyleSheet("background-color: #f7f7f7;");
//    QVBoxLayout* anniversaryRightLayout = new QVBoxLayout(m_anniversaryRightPanel);
//    anniversaryRightLayout->setContentsMargins(15, 8, 15, 8);
//    QLabel* anniversaryListTitle = new QLabel(tr("所有纪念日与倒计时"));
//    anniversaryListTitle->setStyleSheet("font-size: 24px; font-weight: bold;");
//    m_anniversaryItemsWidget = new QListWidget();
//    m_anniversaryItemsWidget->setStyleSheet("QListWidget { border: none; background-color: transparent; }");
//    m_anniversaryItemsWidget->setSpacing(10);
//    m_addAnniversaryButton = new QToolButton();
//    m_addAnniversaryButton->setText(tr("＋ 添加纪念日或倒计时"));
//    connect(m_addAnniversaryButton, &QToolButton::clicked, this, &MainWindow::onAddNewAnniversary);
//    m_addAnniversaryButton->setStyleSheet("QToolButton { border: 1px dashed #ccc; padding: 15px; border-radius: 8px; } QToolButton:hover { background-color: #e9e9e9; }");
//    anniversaryRightLayout->addWidget(anniversaryListTitle);
//    anniversaryRightLayout->addWidget(m_anniversaryItemsWidget, 1);
//    anniversaryRightLayout->addWidget(m_addAnniversaryButton);
//    m_rightContentStack->addWidget(m_anniversaryRightPanel);

//    // --- 任务详情面板 ---
//    m_taskDetailWidget = new TaskDetailWidget();

//    //=========================================================================
//    // 4. 构建顶层分割器
//    //=========================================================================
//    m_rightSideSplitter = new QSplitter(Qt::Horizontal);
//    m_rightSideSplitter->addWidget(m_rightContentStack);
//    m_rightSideSplitter->addWidget(m_taskDetailWidget);
//    m_rightSideSplitter->setHandleWidth(1);
//    m_rightSideSplitter->setStyleSheet("QSplitter::handle { background-color: #efefef; }");
//    m_rightSideSplitter->setSizes({ 1, 0 }); // 默认隐藏详情页

//    m_rootSplitter = new QSplitter(Qt::Horizontal, this);
//    m_rootSplitter->addWidget(m_leftPanel);
//    m_rootSplitter->addWidget(m_rightSideSplitter);
//    m_rootSplitter->setStretchFactor(1, 3);
//    m_rootSplitter->setSizes({ 280, 720 }); // 调整左侧默认宽度
//    m_rootSplitter->setHandleWidth(1);

//    setCentralWidget(m_rootSplitter);
//}


void MainWindow::setupUi() {
    setWindowTitle(tr("ChronoVault"));
    resize(1080, 720);
    m_rootSplitter = new QSplitter(Qt::Horizontal, this);
    m_leftPanel = new QWidget();
    QVBoxLayout* leftPanelLayout = new QVBoxLayout(m_leftPanel);
    leftPanelLayout->setContentsMargins(0, 0, 0, 5);
    leftPanelLayout->setSpacing(0);
    QWidget* moduleSwitcherWidget = new QWidget();
    moduleSwitcherWidget->setStyleSheet("background-color: #f3f3f3; border-bottom: 1px solid #e0e0e0;");
    QHBoxLayout* moduleSwitcherLayout = new QHBoxLayout(moduleSwitcherWidget);
    moduleSwitcherLayout->setContentsMargins(5, 5, 5, 5);
    m_moduleButtonGroup = new QButtonGroup(this);
    m_moduleButtonGroup->setExclusive(true);
    QToolButton* todoButton = new QToolButton();
    todoButton->setText(tr("待办"));
    todoButton->setCheckable(true);
    todoButton->setChecked(true);
    todoButton->setStyleSheet("QToolButton { border: 1px solid #ccc; padding: 5px 15px; background-color: white; border-radius: 4px; } QToolButton:checked { background-color: #cce5ff; border-color: #0078d4; }");
    moduleSwitcherLayout->addWidget(todoButton);
    m_moduleButtonGroup->addButton(todoButton, 0);
    QToolButton* anniversaryButton = new QToolButton();
    anniversaryButton->setText(tr("纪念日"));
    anniversaryButton->setCheckable(true);
    anniversaryButton->setStyleSheet("QToolButton { border: 1px solid #ccc; padding: 5px 15px; background-color: white; border-radius: 4px; } QToolButton:checked { background-color: #cce5ff; border-color: #0078d4; }");
    moduleSwitcherLayout->addWidget(anniversaryButton);
    m_moduleButtonGroup->addButton(anniversaryButton, 1);
    moduleSwitcherLayout->addStretch();
    m_leftContentStack = new QStackedWidget();
    m_todoListPanel = new QWidget();
    QVBoxLayout* todoListLayout = new QVBoxLayout(m_todoListPanel);
    todoListLayout->setContentsMargins(0, 8, 0, 0);
    todoListLayout->setSpacing(0);
    m_listSelectionWidget = new QListWidget();
    m_listSelectionWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_addNewListButton = new QToolButton();
    m_addNewListButton->setText(tr("＋ 新建列表"));
    todoListLayout->addWidget(new QLabel(tr("列表栏")));
    todoListLayout->addWidget(m_listSelectionWidget, 1);
    todoListLayout->addWidget(m_addNewListButton);
    m_leftContentStack->addWidget(m_todoListPanel);
    m_anniversaryPanel = new QWidget();
    QVBoxLayout* anniversaryLayout = new QVBoxLayout(m_anniversaryPanel);
    anniversaryLayout->setContentsMargins(0, 8, 0, 0);
    QHBoxLayout* anniversaryHeaderLayout = new QHBoxLayout();
    anniversaryHeaderLayout->setContentsMargins(8, 0, 8, 4);
    m_addAnniversaryCategoryButton = new QToolButton();
    m_addAnniversaryCategoryButton->setText("+");
    anniversaryHeaderLayout->addWidget(new QLabel(tr("纪念日分类")));
    anniversaryHeaderLayout->addStretch();
    anniversaryHeaderLayout->addWidget(m_addAnniversaryCategoryButton);
    m_anniversaryCategoryWidget = new QListWidget();
    m_anniversaryCategoryWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    anniversaryLayout->addLayout(anniversaryHeaderLayout);
    anniversaryLayout->addWidget(m_anniversaryCategoryWidget, 1);
    m_leftContentStack->addWidget(m_anniversaryPanel);
    m_settingsButton = new QToolButton();
    m_settingsButton->setText(tr("⚙️ 设置与账户"));
    leftPanelLayout->addWidget(moduleSwitcherWidget);
    leftPanelLayout->addWidget(m_leftContentStack, 1);
    leftPanelLayout->addWidget(m_settingsButton);
    m_rightContentStack = new QStackedWidget();
    m_taskViewSplitter = new QSplitter(Qt::Horizontal);
    m_taskPanel = new QWidget();
    QVBoxLayout* taskLayout = new QVBoxLayout(m_taskPanel);
    m_currentListTitleLabel = new QLabel(tr("请选择一个列表"));
    m_taskItemsWidget = new QListWidget();
    m_taskItemsWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    m_addTodoLineEdit = new QLineEdit();
    m_addTodoLineEdit->setPlaceholderText(tr("＋ 添加任务"));
    taskLayout->addWidget(m_currentListTitleLabel);
    taskLayout->addWidget(m_taskItemsWidget, 1);
    taskLayout->addWidget(m_addTodoLineEdit);
    m_taskDetailWidget = new TaskDetailWidget();
    m_taskViewSplitter->addWidget(m_taskPanel);
    m_taskViewSplitter->addWidget(m_taskDetailWidget);
    m_taskViewSplitter->setSizes({1, 0});
    m_rightContentStack->addWidget(m_taskViewSplitter);
    m_anniversaryContentStack = new QStackedWidget();
    m_anniversaryOverviewPanel = new QWidget();
    QVBoxLayout* overviewLayout = new QVBoxLayout(m_anniversaryOverviewPanel);
    m_anniversaryItemsWidget = new QListWidget();
    m_anniversaryItemsWidget->setSpacing(10);
    m_addAnniversaryButton = new QToolButton();
    m_addAnniversaryButton->setText(tr("＋ 添加纪念日"));
    overviewLayout->addWidget(new QLabel(tr("所有纪念日与倒计时")));
    overviewLayout->addWidget(m_anniversaryItemsWidget, 1);
    overviewLayout->addWidget(m_addAnniversaryButton);
    m_anniversaryContentStack->addWidget(m_anniversaryOverviewPanel);
    m_anniversaryDetailView = new AnniversaryDetailView();
    m_anniversaryContentStack->addWidget(m_anniversaryDetailView);
    m_rightContentStack->addWidget(m_anniversaryContentStack);
    m_rootSplitter->addWidget(m_leftPanel);
    m_rootSplitter->addWidget(m_rightContentStack);
    m_rootSplitter->setStretchFactor(1, 3);
    m_rootSplitter->setSizes({280, 720});
    setCentralWidget(m_rootSplitter);
}


void MainWindow::setupConnections() {
    // Module switcher
    connect(m_moduleButtonGroup, &QButtonGroup::idClicked, this, &MainWindow::onModuleChanged);

    // --- Todo connections ---
    connect(m_listSelectionWidget, &QListWidget::currentItemChanged, this, &MainWindow::onCurrentListChanged);
    connect(m_listSelectionWidget, &QListWidget::customContextMenuRequested, this, &MainWindow::showListContextMenu);
    connect(m_addNewListButton, &QToolButton::clicked, this, &MainWindow::onAddNewList);

    connect(m_taskItemsWidget, &QListWidget::itemSelectionChanged, this, &MainWindow::onTaskSelectionChanged);
    connect(m_taskItemsWidget, &QListWidget::customContextMenuRequested, this, &MainWindow::showTaskContextMenu);
    connect(m_taskItemsWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::handleTaskDoubleClick);

    connect(m_addTodoLineEdit, &QLineEdit::returnPressed, this, &MainWindow::onAddNewTodo);

    // --- TaskDetailWidget connections ---
    connect(m_taskDetailWidget, &TaskDetailWidget::closeRequested, this, &MainWindow::onDetailCloseRequested);
    connect(m_taskDetailWidget, &TaskDetailWidget::taskUpdated, this, &MainWindow::handleTaskUpdate);
    connect(m_taskDetailWidget, &TaskDetailWidget::taskDeleted, this, &MainWindow::handleTaskDelete);
    connect(m_taskDetailWidget, &TaskDetailWidget::dueDateChanged, this, &MainWindow::handleDueDateChange);

    // 【核心修正】补全所有 SubTask相关的信号连接
    connect(m_taskDetailWidget, &TaskDetailWidget::addSubTask, this, &MainWindow::handleAddSubTask);
    connect(m_taskDetailWidget, &TaskDetailWidget::subTaskStateChanged, this, &MainWindow::handleSubTaskStateChange);
    connect(m_taskDetailWidget, &TaskDetailWidget::subTaskUpdated, this, &MainWindow::handleSubTaskUpdate);
    connect(m_taskDetailWidget, &TaskDetailWidget::subTaskDeleted, this, &MainWindow::handleSubTaskDelete);
    connect(m_taskDetailWidget, &TaskDetailWidget::subTaskPromoted, this, &MainWindow::handleSubTaskPromote);

    // --- Service connections ---
    connect(m_todoService, &TodoService::listsChanged, this, &MainWindow::refreshListView);
    connect(m_todoService, &TodoService::tasksChanged, this, &MainWindow::refreshTaskView);

    // --- Anniversary connections ---
    connect(m_addAnniversaryCategoryButton, &QToolButton::clicked, this, &MainWindow::onAddNewAnniversaryCategory);
    connect(m_anniversaryCategoryWidget, &QListWidget::currentItemChanged, this, &MainWindow::onAnniversaryCategoryChanged);
    connect(m_anniversaryCategoryWidget, &QListWidget::customContextMenuRequested, this, &MainWindow::showAnniversaryCategoryContextMenu);
    connect(m_addAnniversaryButton, &QToolButton::clicked, this, &MainWindow::onAddNewAnniversary);
    connect(m_anniversaryItemsWidget, &QListWidget::itemClicked, this, &MainWindow::onAnniversaryItemClicked);
    connect(m_anniversaryDetailView, &AnniversaryDetailView::backRequested, this, &MainWindow::onBackFromAnniversaryDetail);
    connect(m_anniversaryService, &AnniversaryService::itemsChanged, this, &MainWindow::refreshAnniversaryCategories);
    connect(m_anniversaryService, &AnniversaryService::itemsChanged, this, &MainWindow::refreshAnniversaryView);
}



void MainWindow::onDetailCloseRequested() {
    m_taskItemsWidget->clearSelection();
    m_taskViewSplitter->setSizes({ 1, 0 }); // 使用 m_taskViewSplitter
}

void MainWindow::onCurrentListChanged(QListWidgetItem* current, QListWidgetItem* previous) {
    Q_UNUSED(previous);
    m_taskViewSplitter->setSizes({1, 0}); // 使用 m_taskViewSplitter
    m_isCompletedSectionExpanded = false;

    if (!current) {
        m_taskItemsWidget->clear();
        m_currentListTitleLabel->setText(tr("请选择一个列表"));
        return;
    }

    QUuid listId = current->data(Qt::UserRole).toUuid();
    if (listId.isNull()) return;

    const auto& lists = m_todoService->getAllLists();
    QString listName;
    for (const auto& list : lists) {
        if (list.id == listId) {
            listName = list.name;
            break;
        }
    }

    if (!listName.isEmpty()) {
        m_currentListTitleLabel->setText(listName);
        displayTasksForList(listId);
    }
}

void MainWindow::onTaskSelectionChanged() {
    QListWidgetItem* selectedItem = m_taskItemsWidget->currentItem();
    if (!selectedItem) {
        m_taskViewSplitter->setSizes({ 1, 0 }); // 使用 m_taskViewSplitter
        return;
    }

    // 检查是否是任务项，而不是分组头
    if (auto* itemWidget = qobject_cast<TodoListItemWidget*>(m_taskItemsWidget->itemWidget(selectedItem))) {
        TodoItem* task = m_todoService->findTodoById(getCurrentListId(), itemWidget->getTodoItem().id());
        if (task) {
            m_taskDetailWidget->displayTask(*task);
            int totalWidth = m_taskViewSplitter->width();
            m_taskViewSplitter->setSizes({ totalWidth * 2 / 3, totalWidth * 1 / 3 }); // 使用 m_taskViewSplitter
        }
    } else {
        // 如果点击的是分组头或其他非任务项，则隐藏详情页
        m_taskViewSplitter->setSizes({ 1, 0 });
    }
}

void MainWindow::onAddNewTodo() {
    QString title = m_addTodoLineEdit->text().trimmed();
    if (title.isEmpty()) return;

    QUuid listId = getCurrentListId();
    if (listId.isNull()) return;

    m_todoService->addTodoToList(listId, TodoItem(title));
    m_addTodoLineEdit->clear();
}


void MainWindow::displayTasksForList(const QUuid& listId) {
    // 【优化】在进行大量更新前，禁用控件的自动重绘，这是性能提升的关键！
    m_taskItemsWidget->setUpdatesEnabled(false);

    m_taskItemsWidget->clear();

    auto* currentList = m_todoService->findListById(listId);
    if (!currentList) {
        m_taskItemsWidget->setUpdatesEnabled(true); // 【优化】别忘了在函数退出前恢复重绘
        return;
    }

    // 1. 分割任务
    QList<TodoItem> activeTasks;
    QList<TodoItem> completedTasks;
    for (const auto& task : currentList->items) {
        if (task.isCompleted()) {
            completedTasks.append(task);
        } else {
            activeTasks.append(task);
        }
    }

    // 2. 添加未完成的任务
    for (const auto& task : activeTasks) {
        TodoListItemWidget *itemWidget = new TodoListItemWidget(task, this);
        QListWidgetItem *listItem = new QListWidgetItem(m_taskItemsWidget);
        listItem->setSizeHint(itemWidget->sizeHint());
        m_taskItemsWidget->addItem(listItem);
        m_taskItemsWidget->setItemWidget(listItem, itemWidget);
        connect(itemWidget, &TodoListItemWidget::taskUpdated, this, &MainWindow::onTaskUpdated);
        connect(itemWidget, &TodoListItemWidget::taskTitleChanged, this, &MainWindow::handleTaskTitleChange);
    }

    // 3. 如果有已完成的任务，则添加分组头和任务项
    if (!completedTasks.isEmpty()) {
        std::sort(completedTasks.begin(), completedTasks.end(), [](const TodoItem& a, const TodoItem& b){
            return a.completionDate() > b.completionDate();
        });

        QListWidgetItem* headerItem = new QListWidgetItem(m_taskItemsWidget);
        CompletedHeaderWidget* headerWidget = new CompletedHeaderWidget(completedTasks.count(), m_isCompletedSectionExpanded, this);
        headerItem->setSizeHint(headerWidget->sizeHint());
        headerItem->setFlags(headerItem->flags() & ~Qt::ItemIsSelectable);
        m_taskItemsWidget->addItem(headerItem);
        m_taskItemsWidget->setItemWidget(headerItem, headerWidget);

        connect(headerWidget, &CompletedHeaderWidget::toggleExpansion, this, [this, listId](){
            m_isCompletedSectionExpanded = !m_isCompletedSectionExpanded;
            displayTasksForList(listId);
        });

        if (m_isCompletedSectionExpanded) {
            for (const auto& task : completedTasks) {
                TodoListItemWidget *itemWidget = new TodoListItemWidget(task, this);
                QListWidgetItem *listItem = new QListWidgetItem(m_taskItemsWidget);
                listItem->setSizeHint(itemWidget->sizeHint());
                m_taskItemsWidget->addItem(listItem);
                m_taskItemsWidget->setItemWidget(listItem, itemWidget);
                connect(itemWidget, &TodoListItemWidget::taskUpdated, this, &MainWindow::onTaskUpdated);
                connect(itemWidget, &TodoListItemWidget::taskTitleChanged, this, &MainWindow::handleTaskTitleChange);
            }
        }
    }

    // 【优化】在所有项都添加完毕后，重新启用控件更新，并强制进行一次刷新
    m_taskItemsWidget->setUpdatesEnabled(true);
}

void MainWindow::handleTaskDelete(const QUuid& taskId) {
    m_todoService->deleteTodoFromList(getCurrentListId(), taskId);
    m_taskViewSplitter->setSizes({1, 0}); // 使用 m_taskViewSplitter
}

QUuid MainWindow::getCurrentListId() const {
    QListWidgetItem* currentItem = m_listSelectionWidget->currentItem();
    if (currentItem) {
        return currentItem->data(Qt::UserRole).toUuid();
    }
    return QUuid();
}

TodoItem* MainWindow::findTodoItemFromWidget(QWidget* widget) const {
    if (!widget) return nullptr;
    auto* itemWidget = qobject_cast<TodoListItemWidget*>(widget);
    if (!itemWidget) return nullptr;
    QUuid listId = getCurrentListId();
    QUuid todoId = itemWidget->getTodoItem().id();
    if(listId.isNull() || todoId.isNull()) return nullptr;
    return m_todoService->findTodoById(listId, todoId);
}

void MainWindow::onAddNewList() {
    bool ok;
    QString name = QInputDialog::getText(this, tr("新建列表"), tr("请输入新列表的名称:"), QLineEdit::Normal, "", &ok);
    if (ok && !name.isEmpty()) {
        m_todoService->addList(name);
    }
}

void MainWindow::showListContextMenu(const QPoint& pos) {
    QListWidgetItem* item = m_listSelectionWidget->itemAt(pos);
    if (!item) return;
    QMenu contextMenu(this);
    contextMenu.addAction("重命名", this, &MainWindow::onRenameList);
    contextMenu.addSeparator();
    contextMenu.addAction("删除列表", this, &MainWindow::onDeleteList);
    contextMenu.exec(m_listSelectionWidget->mapToGlobal(pos));
}



void MainWindow::onDeleteList() {
    QListWidgetItem* currentItem = m_listSelectionWidget->currentItem();
    if (!currentItem) return;
    QUuid listId = currentItem->data(Qt::UserRole).toUuid();
    if (listId.isNull()) return;

    auto reply = QMessageBox::warning(this, tr("确认删除"), tr("确定要删除此列表吗？\n此操作将永久删除列表及其包含的所有任务。"), QMessageBox::Yes | QMessageBox::Cancel);
    if (reply == QMessageBox::Yes) {
        int deletedIndex = m_listSelectionWidget->row(currentItem);
        if (m_todoService->deleteList(listId)) {
            int count = m_listSelectionWidget->count();
            if (count > 0) {
                int newIndex = (deletedIndex > 0) ? (deletedIndex - 1) : 0;
                m_listSelectionWidget->setCurrentRow(newIndex);
            } else {
                m_taskItemsWidget->clear();
                m_currentListTitleLabel->setText(tr("请选择一个列表"));
                m_taskViewSplitter->setSizes({1, 0}); // 使用 m_taskViewSplitter
            }
        }
    }
}

void MainWindow::onRenameList() {
    QListWidgetItem* currentItem = m_listSelectionWidget->currentItem();
    if (!currentItem) return;

    if (auto* widget = qobject_cast<TodoListNameWidget*>(m_listSelectionWidget->itemWidget(currentItem))) {
        widget->enterEditMode();
    }
}

void MainWindow::showTaskContextMenu(const QPoint& pos) {
    // 获取右键点击位置对应的列表项
    QListWidgetItem* item = m_taskItemsWidget->itemAt(pos);
    if (!item) return;

    // 将当前右键点击的项设置为“当前项”，这对于后续获取焦点很重要
    m_taskItemsWidget->setCurrentItem(item);

    QMenu contextMenu(this);
    QAction* modifyAction = contextMenu.addAction("编辑任务...");
    contextMenu.addSeparator();
    QAction* deleteAction = contextMenu.addAction("删除任务");

    QAction* selectedAction = contextMenu.exec(m_taskItemsWidget->mapToGlobal(pos));

    if (selectedAction == modifyAction) {
        // 不再调用 onEditTodo() 弹窗，而是直接触发原地编辑
        if (auto* widget = qobject_cast<TodoListItemWidget*>(m_taskItemsWidget->itemWidget(item))) {
            widget->enterEditMode();
        }
    } else if (selectedAction == deleteAction) {
        // 调用我们已有的删除逻辑
        onDeleteTodo();
    }
}

void MainWindow::handleTaskDoubleClick(QListWidgetItem* item) {
    if (!item) return;
    if (auto* widget = qobject_cast<TodoListItemWidget*>(m_taskItemsWidget->itemWidget(item))) {
        widget->enterEditMode();
    }
}

void MainWindow::onEditTodo() {
    QListWidgetItem* currentItem = m_taskItemsWidget->currentItem();
    if(!currentItem) return;
    TodoListItemWidget* itemWidget = qobject_cast<TodoListItemWidget*>(m_taskItemsWidget->itemWidget(currentItem));
    if(!itemWidget) return;
    TodoItem originalItem = itemWidget->getTodoItem();
    bool ok;
    QString newTitle = QInputDialog::getText(this, "编辑任务", "任务标题:", QLineEdit::Normal, originalItem.title(), &ok);
    if (ok && !newTitle.isEmpty()) {
        TodoItem updatedItem = originalItem;
        updatedItem.setTitle(newTitle);
        m_todoService->updateTodoInList(getCurrentListId(), updatedItem);
    }
}

void MainWindow::onDeleteTodo() {
    QListWidgetItem* currentItem = m_taskItemsWidget->currentItem();
    if(!currentItem) return;
    TodoListItemWidget* itemWidget = qobject_cast<TodoListItemWidget*>(m_taskItemsWidget->itemWidget(currentItem));
    if(!itemWidget) return;
    QUuid listId = getCurrentListId();
    QUuid todoId = itemWidget->getTodoItem().id();
    if (listId.isNull() || todoId.isNull()) return;
    m_todoService->deleteTodoFromList(listId, todoId);
}

void MainWindow::onTaskUpdated(const QUuid& taskId, bool isCompleted) {
    QUuid listId = getCurrentListId();
    if (listId.isNull()) {
        qWarning() << "Could not update task: No list selected.";
        return;
    }
    if (auto* todo = m_todoService->findTodoById(listId, taskId)) {
        TodoItem updatedTodo = *todo;
        updatedTodo.setCompleted(isCompleted);
        m_todoService->updateTodoInList(listId, updatedTodo);
        qDebug() << "Task status updated via service:" << taskId;
    } else {
        qWarning() << "Could not find task to update with ID:" << taskId;
    }
}

void MainWindow::handleTaskUpdate(const TodoItem& updatedTask) {
    m_todoService->updateTodoInList(getCurrentListId(), updatedTask);
}

void MainWindow::handleAddSubTask(const QUuid& taskId, const QString& subTaskTitle) {
    TodoItem* task = m_todoService->findTodoById(getCurrentListId(), taskId);
    if (task) {
        task->subTasks().append(SubTask(subTaskTitle));
        m_todoService->updateTodoInList(getCurrentListId(), *task);
    }
}

void MainWindow::handleSubTaskStateChange(const QUuid& taskId, const QUuid& subTaskId, bool isCompleted) {
    TodoItem* task = m_todoService->findTodoById(getCurrentListId(), taskId);
    if (task) {
        for (auto& subtask : task->subTasks()) {
            if (subtask.id == subTaskId) {
                subtask.isCompleted = isCompleted;
                break;
            }
        }
        m_todoService->updateTodoInList(getCurrentListId(), *task);
    }
}

void MainWindow::refreshListView() {
    disconnect(m_listSelectionWidget, &QListWidget::currentItemChanged, this, &MainWindow::onCurrentListChanged);
    m_listSelectionWidget->clear();
    const auto& lists = m_todoService->getAllLists();
    for (const auto& list : lists) {
        QListWidgetItem* item = new QListWidgetItem(m_listSelectionWidget);
        TodoListNameWidget* widget = new TodoListNameWidget(list.id, list.name, m_listSelectionWidget);

        // 【重要】将列表ID存回 item 中
        item->setData(Qt::UserRole, QVariant::fromValue(list.id));

        item->setSizeHint(widget->sizeHint());
        m_listSelectionWidget->addItem(item);
        m_listSelectionWidget->setItemWidget(item, widget);

        connect(widget, &TodoListNameWidget::listNameChanged, this, &MainWindow::handleListNameChange);
    }
    connect(m_listSelectionWidget, &QListWidget::currentItemChanged, this, &MainWindow::onCurrentListChanged);
}

void MainWindow::refreshTaskView(const QUuid& listId) {
    QListWidgetItem* currentListItem = m_listSelectionWidget->currentItem();
    if (!currentListItem || currentListItem->data(Qt::UserRole).toUuid() != listId) {
        return;
    }
    QUuid detailTaskId = m_taskDetailWidget->getCurrentTaskId();
    displayTasksForList(listId);
    if (!detailTaskId.isNull()) {
        if (auto* freshTask = m_todoService->findTodoById(listId, detailTaskId)) {
            m_taskDetailWidget->displayTask(*freshTask);
        } else {
            m_taskViewSplitter->setSizes({1, 0}); // 使用 m_taskViewSplitter
        }
    }
}


void MainWindow::handleSubTaskUpdate(const QUuid& taskId, const SubTask& updatedSubTask) {
    m_todoService->updateSubTask(getCurrentListId(), taskId, updatedSubTask);
}

void MainWindow::handleSubTaskDelete(const QUuid& taskId, const QUuid& subTaskId) {
    m_todoService->deleteSubTask(getCurrentListId(), taskId, subTaskId);
}

void MainWindow::handleSubTaskPromote(const QUuid& taskId, const QUuid& subTaskId) {
    m_todoService->promoteSubTaskToTodo(getCurrentListId(), taskId, subTaskId);
}

void MainWindow::handleTaskTitleChange(const QUuid& taskId, const QString& newTitle) {
    QUuid listId = getCurrentListId();
    if (auto* task = m_todoService->findTodoById(listId, taskId)) {
        TodoItem updatedTask = *task;
        updatedTask.setTitle(newTitle);
        m_todoService->updateTodoInList(listId, updatedTask);
    }
}

void MainWindow::handleListNameChange(const QUuid& listId, const QString& newName) {
    m_todoService->updateListName(listId, newName);
}

void MainWindow::handleDueDateChange(const QUuid& taskId, const QDateTime& dueDate)
{
    QUuid listId = getCurrentListId();
    if (auto* task = m_todoService->findTodoById(listId, taskId)) {
        // 1. 更新 TodoItem 自身
        TodoItem updatedTask = *task;
        updatedTask.setDueDate(dueDate);
        m_todoService->updateTodoInList(listId, updatedTask);

        // 2. 【核心联动逻辑】检查是否有源倒计时，并同步更新
        if (!task->sourceAnniversaryId().isNull()) {
            auto reply = QMessageBox::question(this,
                                               tr("同步更新"),
                                               tr("是否要将此新的截止日期同步更新到源头的“倒计时”项目中？"),
                                               QMessageBox::Yes | QMessageBox::No,
                                               QMessageBox::Yes);
            if (reply == QMessageBox::Yes) {
                m_anniversaryService->updateTargetDateTime(task->sourceAnniversaryId(), dueDate);
            }
        }
    }
}


//实现模块切换的槽函数
void MainWindow::onModuleChanged(int id) {
    m_leftContentStack->setCurrentIndex(id);
    // 在这里，我们未来还可以根据模块切换，更新右侧面板的内容
    // 例如，当 id 为 1 (纪念日) 时，隐藏 m_taskPanel，显示纪念日详情面板
    m_rightContentStack->setCurrentIndex(id); // 同步切换右侧内容区
    qDebug() << "Switched to module with ID:" << id;
}

//用于刷新纪念日列表
void MainWindow::refreshAnniversaryView() {
    m_anniversaryItemsWidget->setUpdatesEnabled(false);
    m_anniversaryItemsWidget->clear();

    // 1. 获取当前选中的分类
    QString selectedCategory;
    if (m_anniversaryCategoryWidget->currentItem()) {
        selectedCategory = m_anniversaryCategoryWidget->currentItem()->text();
    }
    // 如果选中的是“所有项目”，则将分类名置空以匹配所有
    if (selectedCategory == tr("所有项目")) {
        selectedCategory.clear();
    }

    const auto& allItems = m_anniversaryService->getAllItems();
    for (const auto& item : allItems) {
        // 2. 筛选逻辑：如果未选择分类(显示所有)，或者项目分类与所选分类匹配
        if (selectedCategory.isEmpty() || item.category() == selectedCategory) {
            AnniversaryItemWidget* itemWidget = new AnniversaryItemWidget(item, this);
            QListWidgetItem* listItem = new QListWidgetItem(m_anniversaryItemsWidget);
            listItem->setSizeHint(itemWidget->sizeHint());
            listItem->setFlags(listItem->flags() & ~Qt::ItemIsSelectable);
            m_anniversaryItemsWidget->addItem(listItem);

            m_anniversaryItemsWidget->setItemWidget(listItem, itemWidget);

            connect(itemWidget, &AnniversaryItemWidget::itemDeleted, this, &MainWindow::onAnniversaryItemDeleted);
            connect(itemWidget, &AnniversaryItemWidget::addToTodoRequested, this, &MainWindow::onAddToTodoRequested);
            connect(itemWidget, &AnniversaryItemWidget::addMomentRequested, this, &MainWindow::onAddMomentRequested);
        }
    }
    m_anniversaryItemsWidget->setUpdatesEnabled(true);
}


void MainWindow::onAddNewAnniversary() {
    QString currentCategory;
    QListWidgetItem* currentItem = m_anniversaryCategoryWidget->currentItem();
    // 如果当前选中的不是“所有项目”，则将其作为默认分类
    if (currentItem && currentItem->text() != tr("所有项目")) {
        currentCategory = currentItem->text();
    }

    AddAnniversaryDialog dialog(currentCategory, this);
    if (dialog.exec() == QDialog::Accepted) {
        AnniversaryItem item = dialog.getAnniversaryItem();
        if (!item.title().isEmpty()) {
            m_anniversaryService->addItem(item);
        }
    }
}

void MainWindow::onAnniversaryItemDeleted(const QUuid& id)
{
    qDebug() << "Request to delete anniversary item with ID:" << id;
    // 直接调用服务层的删除接口
    m_anniversaryService->deleteItem(id);
}


void MainWindow::onAddToTodoRequested(const QUuid& anniversaryId)
{
    // 1. 获取源项目信息
    AnniversaryItem* sourceItem = m_anniversaryService->findItemById(anniversaryId);
    if (!sourceItem || sourceItem->eventType() != AnniversaryEventType::Countdown) {
        return;
    }

    // 2. 弹出列表选择对话框
    TodoListSelectionDialog listDialog(m_todoService->getAllLists(), this);
    if (listDialog.exec() != QDialog::Accepted) {
        return; // 用户点击了取消
    }

    // 3. 处理对话框返回的结果
    QUuid targetListId;
    if (listDialog.isNewListCreated()) {
        // 如果用户在对话框中创建了一个新列表
        targetListId = m_todoService->addList(listDialog.getNewListName());
        if (targetListId.isNull()) {
            QMessageBox::warning(this, tr("错误"), tr("无法创建新的待办列表。"));
            return;
        }
    } else {
        // 如果用户选择了一个已存在的列表
        targetListId = listDialog.getSelectedListId();
        if (targetListId.isNull()) {
            QMessageBox::warning(this, tr("错误"), tr("没有选择任何有效的待办列表。"));
            return;
        }
    }

    // ------------------- 【已删除重复的代码块】 -------------------
    //  您原来的代码在这里重复了获取 sourceItem 和 targetListId 的逻辑
    // -----------------------------------------------------------

    // 4. 创建新的 TodoItem
    TodoItem newTodo(sourceItem->title());
    newTodo.setDueDate(sourceItem->targetDateTime());
    newTodo.setSourceAnniversaryId(sourceItem->id());

    // 5. 询问用户是否设置提醒
    auto reply = QMessageBox::question(this,
                                       tr("设置提醒"),
                                       tr("是否要为此新待办事项“%1”设置提醒？").arg(newTodo.title()),
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::Yes);

    if (reply == QMessageBox::Yes) {
        // 创建一个默认的 Reminder 对象 (在截止前15分钟提醒，不重复)
        Reminder initialReminder;
        initialReminder.setActive(true);
        initialReminder.setNextReminderTime(newTodo.dueDate().addSecs(-15 * 60));
        initialReminder.setIntervalType(ReminderIntervalType::None);

        // 弹出我们强大的新对话框，让用户进行配置
        ReminderSettingsDialog reminderDialog(initialReminder, this);
        if (reminderDialog.exec() == QDialog::Accepted) {
            newTodo.setReminder(reminderDialog.getReminder());
        }
    }

    // 6. 将配置好的 TodoItem 添加到目标列表
    m_todoService->addTodoToList(targetListId, newTodo);

    // 7. 标记原始的 AnniversaryItem 为“已添加”
    m_anniversaryService->markAsAddedToTodo(anniversaryId);

    // 8. 最终的成功提示
    QMessageBox::information(this, tr("操作成功"), tr("已将倒计时“%1”添加到待办列表中。").arg(sourceItem->title()));
}

void MainWindow::refreshAnniversaryCategories() {
    disconnect(m_anniversaryCategoryWidget, &QListWidget::currentItemChanged, this, &MainWindow::onAnniversaryCategoryChanged);

    QString currentCategoryText;
    if (m_anniversaryCategoryWidget->currentItem()) {
        currentCategoryText = m_anniversaryCategoryWidget->currentItem()->text();
    }

    m_anniversaryCategoryWidget->clear();
    m_anniversaryCategoryWidget->addItem(tr("所有项目"));

    // 【核心修正】只从服务层这一个“唯一数据源”获取并添加分类
    m_anniversaryCategoryWidget->addItems(m_anniversaryService->getCategories());


    // 尝试恢复之前的选中项
    for (int i = 0; i < m_anniversaryCategoryWidget->count(); ++i) {
        if (m_anniversaryCategoryWidget->item(i)->text() == currentCategoryText) {
            m_anniversaryCategoryWidget->setCurrentRow(i);
            break;
        }
    }
    if (!m_anniversaryCategoryWidget->currentItem() && m_anniversaryCategoryWidget->count() > 0) {
        m_anniversaryCategoryWidget->setCurrentRow(0);
    }

    connect(m_anniversaryCategoryWidget, &QListWidget::currentItemChanged, this, &MainWindow::onAnniversaryCategoryChanged);
}

void MainWindow::onAnniversaryCategoryChanged()
{
    // 当左侧的分类选择变化时，我们只需要调用刷新函数即可
    // 刷新函数内部会自动获取当前选中的分类并进行筛选
    refreshAnniversaryView();
}


// 显示分类右键菜单的槽函数
void MainWindow::showAnniversaryCategoryContextMenu(const QPoint& pos)
{
    QListWidgetItem* item = m_anniversaryCategoryWidget->itemAt(pos);
    if (!item) {
        return;
    }

    // 不允许对“所有项目”进行操作
    if (item->text() == tr("所有项目")) {
        return;
    }

    QMenu contextMenu(this);
    contextMenu.addAction(tr("重命名分类"), this, &MainWindow::onRenameAnniversaryCategory);
    contextMenu.addAction(tr("删除分类"), this, &MainWindow::onDeleteAnniversaryCategory);
    contextMenu.exec(m_anniversaryCategoryWidget->mapToGlobal(pos));
}

// 处理删除分类请求的槽函数
void MainWindow::onDeleteAnniversaryCategory()
{
    QListWidgetItem* currentItem = m_anniversaryCategoryWidget->currentItem();
    if (!currentItem) {
        return;
    }

    QString categoryName = currentItem->text();

    // 弹出确认对话框，防止误操作
    auto reply = QMessageBox::warning(this,
                                      tr("确认删除分类"),
                                      tr("确定要删除“%1”分类吗？\n\n（注意：此分类下的所有项目将变为“未分类”，但不会被删除。）")
                                          .arg(categoryName),
                                      QMessageBox::Yes | QMessageBox::Cancel);

    if (reply == QMessageBox::Yes) {
        m_anniversaryService->deleteCategory(categoryName);
    }
}

void MainWindow::onAddNewAnniversaryCategory()
{
    bool ok;
    QString newCategoryName = QInputDialog::getText(this,
                                                    tr("新建纪念日分类"),
                                                    tr("请输入新分类的名称:"),
                                                    QLineEdit::Normal, "", &ok);
    if (ok && !newCategoryName.isEmpty()) {
        m_anniversaryService->addCategory(newCategoryName);
    }
}


void MainWindow::onRenameAnniversaryCategory()
{
    QListWidgetItem* currentItem = m_anniversaryCategoryWidget->currentItem();
    if (!currentItem || currentItem->text() == tr("所有项目")) return;

    QString oldName = currentItem->text();
    bool ok;
    QString newName = QInputDialog::getText(this, tr("重命名分类"), tr("请输入新的分类名称:"), QLineEdit::Normal, oldName, &ok);

    if (ok && !newName.isEmpty() && newName != oldName) {
        m_anniversaryService->renameCategory(oldName, newName);
    }
}


void MainWindow::onAddMomentRequested(const QUuid& anniversaryId)
{
    AddMomentDialog dialog(this); // 创建我们新的对话框
    if (dialog.exec() == QDialog::Accepted) {
        Moment newMoment = dialog.getMoment();
        m_anniversaryService->addMomentToItem(anniversaryId, newMoment);
    }
}


// 处理点击纪念日卡片事件
void MainWindow::onAnniversaryItemClicked(QListWidgetItem* item)
{
    if (!item) return;
    auto* widget = qobject_cast<AnniversaryItemWidget*>(m_anniversaryItemsWidget->itemWidget(item));
    if (!widget) return;

    const auto* anniversaryItem = m_anniversaryService->findItemById(widget->item().id());
    if (anniversaryItem) {
        // 【诊断日志 2】检查从主窗口传递到详情页前，项目有多少个“瞬间”
        qDebug() << "[DIAGNOSTIC 2] In MainWindow::onAnniversaryItemClicked, item has"
                 << anniversaryItem->moments().count() << "moments.";

        m_anniversaryDetailView->displayAnniversary(*anniversaryItem);
        m_anniversaryContentStack->setCurrentWidget(m_anniversaryDetailView);
    }
}

//处理从详情页返回的请求
void MainWindow::onBackFromAnniversaryDetail() {
    m_anniversaryContentStack->setCurrentWidget(m_anniversaryOverviewPanel);
}


