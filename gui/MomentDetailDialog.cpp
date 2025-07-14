#include "gui/MomentDetailDialog.h"
#include "gui/ImageViewerDialog.h"
#include "gui/ClickableLabel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QTextEdit> // 【新增】
#include <QStackedWidget>
#include <QToolButton>
#include <QTimer>
#include <QCloseEvent> // 【新增】
#include <QDebug>
#include <QDialogButtonBox>

MomentDetailDialog::MomentDetailDialog(const Moment& moment, const QUuid& anniversaryId, QWidget *parent)
    : QDialog(parent), m_currentMoment(moment), m_anniversaryId(anniversaryId)
{
    setupUi(moment);

    // 初始化自动保存定时器
    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setInterval(500); // 停止输入0.5秒后保存
    m_autoSaveTimer->setSingleShot(true);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MomentDetailDialog::performAutoSave);

    // 初始化切换到预览模式的定时器
    m_switchToPreviewTimer = new QTimer(this);
    m_switchToPreviewTimer->setInterval(5000); // 停止输入5秒后预览
    m_switchToPreviewTimer->setSingleShot(true);
    connect(m_switchToPreviewTimer, &QTimer::timeout, this, &MomentDetailDialog::switchToPreviewMode);

    // 连接文本变化的信号
    connect(m_textEdit, &QTextEdit::textChanged, this, &MomentDetailDialog::onTextChanged);
}

MomentDetailDialog::~MomentDetailDialog() {}

void MomentDetailDialog::setupUi(const Moment& moment)
{
    // 1. 初始化对话框基本属性
    setWindowTitle(tr("瞬间详情"));
    setMinimumSize(450, 500);
    setStyleSheet("QDialog { background-color: #f7f7f7; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);

    //=========================================================================
    // 2. 构建图片展示与切换区域
    //=========================================================================
    QFrame* imageViewerFrame = new QFrame();
    imageViewerFrame->setStyleSheet("background-color: black; border-radius: 8px;");
    QVBoxLayout* imageViewerLayout = new QVBoxLayout(imageViewerFrame);
    imageViewerLayout->setContentsMargins(5, 5, 5, 10); // 底部留出更多空间给控制栏

    // -- 2a. 用于切换图片的堆叠窗口 --
    m_imageStack = new QStackedWidget();
    if (!moment.imagePaths().isEmpty()) {
        for (const QString& path : moment.imagePaths()) {
            ClickableLabel* imgLabel = new ClickableLabel(path); // 使用可点击的Label
            imgLabel->setAlignment(Qt::AlignCenter);
            QPixmap pixmap(path);
            imgLabel->setPixmap(pixmap.scaled(400, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            imgLabel->setCursor(Qt::PointingHandCursor);
            imgLabel->setToolTip(tr("点击查看原图"));
            m_imageStack->addWidget(imgLabel);
            // 连接点击信号，以便未来实现图片放大查看
            connect(imgLabel, &ClickableLabel::clicked, this, [this](const QString& p){
                // ImageViewerDialog viewer(p, this);
                // viewer.exec();
                qDebug() << "Image clicked:" << p; // 临时的占位逻辑
            });
        }
    } else {
        QLabel* noImageLabel = new QLabel(tr("无图片"));
        noImageLabel->setAlignment(Qt::AlignCenter);
        noImageLabel->setStyleSheet("color: white; font-size: 18px;");
        m_imageStack->addWidget(noImageLabel);
    }

    // -- 2b. 图片控制栏 (切换按钮和计数器) --
    QHBoxLayout* imageControlLayout = new QHBoxLayout();
    m_prevButton = new QToolButton();
    m_prevButton->setText("<");
    m_prevButton->setStyleSheet("QToolButton { color: white; border: none; font-size: 24px; font-weight: bold; background: transparent; }");
    m_nextButton = new QToolButton();
    m_nextButton->setText(">");
    m_nextButton->setStyleSheet("QToolButton { color: white; border: none; font-size: 24px; font-weight: bold; background: transparent; }");
    m_imageCounterLabel = new QLabel();
    m_imageCounterLabel->setStyleSheet("color: white;");

    imageControlLayout->addWidget(m_prevButton);
    imageControlLayout->addStretch();
    imageControlLayout->addWidget(m_imageCounterLabel);
    imageControlLayout->addStretch();
    imageControlLayout->addWidget(m_nextButton);

    // -- 2c. 组合图片展示区 --
    imageViewerLayout->addWidget(m_imageStack, 1);
    imageViewerLayout->addLayout(imageControlLayout);
    mainLayout->addWidget(imageViewerFrame);

    //=========================================================================
    // 3. 构建文本编辑/预览区域
    //=========================================================================
    m_textStack = new QStackedWidget();

    // 编辑视图
    m_textEdit = new QTextEdit();
    m_textEdit->setPlainText(moment.text()); // 使用 setMarkdown 以保留格式
    m_textEdit->setStyleSheet("QTextEdit { border: 1px solid #e0e0e0; background-color: white; border-radius: 4px; padding: 5px; }");

    // 预览视图
    m_textBrowser = new QTextBrowser();
    m_textBrowser->setOpenExternalLinks(true);
    m_textBrowser->setStyleSheet("QTextBrowser { border: 1px solid #e0e0e0; background-color: white; border-radius: 4px; padding: 5px; }");
    // 【核心修正】为预览框安装事件过滤器
    m_textBrowser->installEventFilter(this);


    m_textStack->addWidget(m_textEdit);
    m_textStack->addWidget(m_textBrowser);

    mainLayout->addWidget(m_textStack, 1); // 让文本区域占据更多垂直空间

    //=========================================================================
    // 4. 初始化UI状态和连接 (无OK/Cancel按钮)
    //=========================================================================
    updateImageCounter(); // 根据图片数量更新一次计数器
    bool hasMultipleImages = m_imageStack->count() > 1;
    m_prevButton->setVisible(hasMultipleImages);
    m_nextButton->setVisible(hasMultipleImages);
    m_imageCounterLabel->setVisible(hasMultipleImages);

    // 连接图片切换按钮的信号
    connect(m_prevButton, &QToolButton::clicked, this, &MomentDetailDialog::showPreviousImage);
    connect(m_nextButton, &QToolButton::clicked, this, &MomentDetailDialog::showNextImage);
}

void MomentDetailDialog::showPreviousImage() {
    int newIndex = m_imageStack->currentIndex() - 1;
    if (newIndex < 0) {
        newIndex = m_imageStack->count() - 1;
    }
    m_imageStack->setCurrentIndex(newIndex);
    updateImageCounter();
}

void MomentDetailDialog::showNextImage() {
    int newIndex = (m_imageStack->currentIndex() + 1) % m_imageStack->count();
    m_imageStack->setCurrentIndex(newIndex);
    updateImageCounter();
}

void MomentDetailDialog::updateImageCounter() {
    int current = m_imageStack->currentIndex() + 1;
    int total = m_imageStack->count();
    m_imageCounterLabel->setText(QString("%1 / %2").arg(current).arg(total));
}

// 当文本框内容改变时，启动(或重置)自动保存定时器
void MomentDetailDialog::onTextChanged()
{
    m_isDirty = true; // 标记为“脏”，表示有未保存的修改
    m_autoSaveTimer->start(); // 重启自动保存定时器
    m_switchToPreviewTimer->start(); // 重启切换预览定时器
}

void MomentDetailDialog::switchToEditMode()
{
    m_textStack->setCurrentWidget(m_textEdit);
    m_textEdit->setFocus();
    // 将光标移动到文本末尾
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_textEdit->setTextCursor(cursor);
}

// 切换到预览模式
void MomentDetailDialog::switchToPreviewMode()
{
    // 【核心修正】确保在预览前，所有修改都已保存
    if (m_isDirty) {
        performAutoSave();
    }
    m_textBrowser->setMarkdown(m_textEdit->toMarkdown());
    m_textStack->setCurrentWidget(m_textBrowser);
}

// 事件过滤器，用于处理预览视图的点击
bool MomentDetailDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_textBrowser && event->type() == QEvent::MouseButtonPress) {
        switchToEditMode();
        return true;
    }
    return QDialog::eventFilter(watched, event);
}

// 执行实际的保存操作
void MomentDetailDialog::performAutoSave()
{
    if (!m_isDirty) return;

    // 【核心修正】使用 toPlainText() 获取并保存原始Markdown文本
    m_currentMoment.setText(m_textEdit->toPlainText());
    emit momentUpdated(m_anniversaryId, m_currentMoment);
    m_isDirty = false;
    qDebug() << "Auto-saved moment:" << m_currentMoment.id();
}

void MomentDetailDialog::closeEvent(QCloseEvent *event)
{
    // 如果定时器正在等待，说明有刚输入的、还未触发保存的修改
    if (m_autoSaveTimer->isActive()) {
        m_autoSaveTimer->stop(); // 停止定时器
        performAutoSave();       // 立即执行保存
    }
    // 检查是否还有其他未保存的修改
    else if (m_isDirty) {
        performAutoSave();
    }
    event->accept();
}

