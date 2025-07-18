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

MomentDetailDialog::MomentDetailDialog(const Moment& moment, QWidget *parent)
    : QDialog(parent), m_currentMoment(moment)
{
    setupUi(moment);

    m_switchToPreviewTimer = new QTimer(this);
    m_switchToPreviewTimer->setInterval(3000); // 设置延时为 3000 毫秒 (3秒)
    m_switchToPreviewTimer->setSingleShot(true); // 确保它只触发一次
    connect(m_switchToPreviewTimer, &QTimer::timeout, this, &MomentDetailDialog::switchToPreviewMode);

    // --- 【新增】连接文本变化的信号到我们的新槽函数 ---
    connect(m_textEdit, &QTextEdit::textChanged, this, &MomentDetailDialog::onTextChanged);

}

MomentDetailDialog::~MomentDetailDialog() = default;

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
    m_textStack = new QStackedWidget(this);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setPlainText(moment.text());



    m_textBrowser = new ClickableTextBrowser(); // 确保 ClickableTextBrowser 类型已定义
    m_textBrowser->setOpenExternalLinks(true);
    connect(m_textBrowser, &ClickableTextBrowser::clicked, this, &MomentDetailDialog::switchToEditMode);

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

    connect(m_textBrowser, &ClickableTextBrowser::clicked, this, &MomentDetailDialog::switchToEditMode);


    switchToPreviewMode();
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

// 【新增】实现 onTextChanged 槽函数
void MomentDetailDialog::onTextChanged()
{
    // 每当用户输入一个字符，就重新启动（或启动）这个3秒的倒计时
    if(m_switchToPreviewTimer) {
        m_switchToPreviewTimer->start();
    }
}
void MomentDetailDialog::switchToEditMode()
{
    m_textStack->setCurrentWidget(m_textEdit);
    m_textEdit->setFocus();
}

// 切换到预览模式
void MomentDetailDialog::switchToPreviewMode()
{
    m_textBrowser->setMarkdown(m_textEdit->toPlainText());
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



void MomentDetailDialog::closeEvent(QCloseEvent *event)
{
    // 在窗口关闭的最后时刻，将UI上的数据更新到内部的 m_currentMoment 成员中
    m_currentMoment.setText(m_textEdit->toPlainText());
    // 注意：这里不再需要 m_isDirty 标志，因为我们总是以UI上的最终状态为准

    event->accept(); // 允许窗口关闭
}

// 公共接口，让外部可以获取最终修改后的 Moment 对象
Moment MomentDetailDialog::getMoment() const {
    return m_currentMoment;
}
