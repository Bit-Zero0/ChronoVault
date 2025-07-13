#include "gui/AddMomentDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QTextBrowser>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFileDialog>
#include <QScrollArea>
#include <QPixmap>
#include <QTabWidget>
#include <QAction>

AddMomentDialog::AddMomentDialog(QWidget *parent) : QDialog(parent)
{
    setupUi();
}

void AddMomentDialog::setupUi() {
    setWindowTitle(tr("记录一个新的瞬间"));
    setMinimumSize(550, 450);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);

    // --- Tab控件，用于编辑和预览Markdown ---
    m_editorTabWidget = new QTabWidget();
    m_markdownEditor = new QTextEdit();
    m_markdownPreview = new QTextBrowser();
    m_markdownPreview->setOpenExternalLinks(true); // 允许打开链接

    m_editorTabWidget->addTab(m_markdownEditor, tr("编辑"));
    m_editorTabWidget->addTab(m_markdownPreview, tr("预览"));
    connect(m_editorTabWidget, &QTabWidget::currentChanged, this, &AddMomentDialog::onTabChanged);

    mainLayout->addWidget(new QLabel(tr("记录文字:")));
    mainLayout->addWidget(m_editorTabWidget, 1); // 让文本编辑区占据更多空间

    // --- 图片选择和预览区域 ---
    QHBoxLayout* imageControlLayout = new QHBoxLayout();
    QPushButton* addImageButton = new QPushButton(tr("添加图片..."));
    connect(addImageButton, &QPushButton::clicked, this, &AddMomentDialog::onAddImageClicked);
    imageControlLayout->addWidget(addImageButton);
    imageControlLayout->addStretch();
    mainLayout->addLayout(imageControlLayout);

    m_imagePreviewArea = new QScrollArea();
    m_imagePreviewArea->setWidgetResizable(true);
    m_imagePreviewArea->setFixedHeight(100);
    m_imagePreviewWidget = new QWidget();
    m_imagePreviewLayout = new QHBoxLayout(m_imagePreviewWidget);
    m_imagePreviewLayout->setAlignment(Qt::AlignLeft);
    m_imagePreviewArea->setWidget(m_imagePreviewWidget);
    mainLayout->addWidget(m_imagePreviewArea);

    // --- OK 和 Cancel 按钮 ---
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

QAction* nextTabAction = new QAction(this);
    QAction* prevTabAction = new QAction(this);

    // 2. 为动作设置标准的“切换子窗口”快捷键
    nextTabAction->setShortcut(QKeySequence::NextChild);
    prevTabAction->setShortcut(QKeySequence::PreviousChild);

    // 3. 将这两个动作添加到对话框中，使其只在该对话框内生效
    this->addAction(nextTabAction);
    this->addAction(prevTabAction);

    // 4. 连接动作的 triggered 信号到处理逻辑 (使用Lambda函数)
    connect(nextTabAction, &QAction::triggered, this, [this]() {
        int count = m_editorTabWidget->count();
        if (count == 0) return;
        int nextIndex = (m_editorTabWidget->currentIndex() + 1) % count;
        m_editorTabWidget->setCurrentIndex(nextIndex);
    });

    connect(prevTabAction, &QAction::triggered, this, [this]() {
        int count = m_editorTabWidget->count();
        if (count == 0) return;
        int prevIndex = (m_editorTabWidget->currentIndex() - 1 + count) % count;
        m_editorTabWidget->setCurrentIndex(prevIndex);
    });
}

void AddMomentDialog::onAddImageClicked()
{
    // 支持多选
    QStringList filePaths = QFileDialog::getOpenFileNames(
        this,
        tr("选择一张或多张图片"),
        QDir::homePath(),
        tr("图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)")
        );

    if (filePaths.isEmpty()) {
        return;
    }

    m_selectedImagePaths.append(filePaths);

    // 为每张新选择的图片创建并添加预览图
    for (const QString& path : filePaths) {
        QLabel* imageLabel = new QLabel();
        QPixmap pixmap(path);
        imageLabel->setPixmap(pixmap.scaledToHeight(80, Qt::SmoothTransformation));
        imageLabel->setToolTip(path);
        m_imagePreviewLayout->addWidget(imageLabel);
    }
}

void AddMomentDialog::onTabChanged(int index) {
    // 如果用户切换到了“预览”Tab (index 1)
    if (index == 1) {
        // QTextBrowser 内置了对基础 Markdown 的渲染支持
        m_markdownPreview->setMarkdown(m_markdownEditor->toPlainText());
    }
}

Moment AddMomentDialog::getMoment() const
{
    Moment moment(m_markdownEditor->toPlainText());
    for (const QString& path : m_selectedImagePaths) {
        moment.addImagePath(path);
    }
    return moment;
}
