#include "gui/MomentCardWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QMouseEvent>
#include <QToolButton>
#include <QEnterEvent>
#include <QFontMetrics>

MomentCardWidget::MomentCardWidget(const Moment& moment, QWidget *parent)
    : QFrame(parent), m_moment(moment)
{
    setupUi(moment);

    connect(m_deleteButton, &QToolButton::clicked, this, &MomentCardWidget::onDeleteButtonClicked);

}

MomentCardWidget::~MomentCardWidget()
{
    qDebug() << "Destroying MomentCardWidget for moment ID:" << m_moment.id();
}

void MomentCardWidget::setupUi(const Moment& moment) {
    setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    setCursor(Qt::PointingHandCursor);
    setFixedSize(160, 130);
    setStyleSheet("MomentCardWidget { border: 1px solid #e0e0e0; border-radius: 4px; background-color: white; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(4);

    // 1. 创建所有UI元素
    m_imageLabel = new QLabel();
    m_timestampLabel = new QLabel();
    m_textLabel = new QLabel();

    // 2. 将它们按固定顺序添加到布局中
    mainLayout->addWidget(m_imageLabel);
    mainLayout->addWidget(m_timestampLabel);
    mainLayout->addWidget(m_textLabel);
    mainLayout->addStretch(); // 弹簧，确保内容总是靠上对齐

    // 3. --- 【核心逻辑】根据有无图片，配置UI ---
    if (moment.imagePaths().isEmpty()) {
        // --- 无图片模式 ---
        m_imageLabel->setVisible(false); // 隐藏图片控件

        // 将时间戳标签用作标题
        m_timestampLabel->setText(moment.timestamp().toString("yyyy-MM-dd hh:mm"));
        // 【重要修正】设置更大、居中的标题样式
        m_timestampLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #333;");
        m_timestampLabel->setAlignment(Qt::AlignCenter);

        // 显示完整的文字内容，并启用自动换行
        m_textLabel->setText(moment.text());
        m_textLabel->setStyleSheet("font-size: 12px; color: #555;");
        m_textLabel->setWordWrap(true);

    } else {
        // --- 有图片模式 ---
        m_imageLabel->setVisible(true); // 显示图片控件
        m_imageLabel->setAlignment(Qt::AlignCenter);
        m_imageLabel->setFixedSize(144, 80);
        QPixmap pixmap(moment.imagePaths().first());
        m_imageLabel->setPixmap(pixmap.scaled(m_imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

        // 时间戳标签显示详细时间，并靠左对齐
        m_timestampLabel->setText(moment.timestamp().toString("yyyy-MM-dd hh:mm"));
        m_timestampLabel->setStyleSheet("font-size: 11px; color: gray;");
        m_timestampLabel->setAlignment(Qt::AlignLeft);

        // 【智能截断逻辑】解决文字穿模问题
        QString originalText = moment.text().replace('\n', ' '); // 将换行符替换为空格，确保是单行
        QFontMetrics metrics(m_textLabel->font());
        // elidedText会根据可用宽度（这里是144像素）自动截断文本并在末尾加上 "..."
        QString elidedText = metrics.elidedText(originalText, Qt::ElideRight, 144);

        m_textLabel->setText(elidedText);
        m_textLabel->setStyleSheet("font-size: 12px; color: #555;");
        m_textLabel->setWordWrap(false); // 摘要不换行
    }

    // 4. 设置完整的提示信息（鼠标悬停时显示完整内容）
    this->setToolTip(moment.text());

    // 【新增】创建悬浮的删除按钮
    m_deleteButton = new QToolButton(this); // **父对象是 this (MomentCardWidget)**
    m_deleteButton->setText("✕");
    m_deleteButton->setCursor(Qt::PointingHandCursor);
    m_deleteButton->setStyleSheet("QToolButton { background-color: rgba(0, 0, 0, 0.5); color: white; border-radius: 8px; padding: 2px; font-weight: bold; } QToolButton:hover { background-color: #ff3d71; }");
    m_deleteButton->setFixedSize(16, 16);
    m_deleteButton->move(this->width() - m_deleteButton->width() - 5, 5); // **手动定位到右上角**
    m_deleteButton->hide(); // 默认隐藏
}

void MomentCardWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_moment);
    }
    QFrame::mouseReleaseEvent(event);
}

void MomentCardWidget::enterEvent(QEnterEvent* event)
{
    m_deleteButton->show();
    QFrame::enterEvent(event);
}

void MomentCardWidget::leaveEvent(QEvent* event)
{
    m_deleteButton->hide();
    QFrame::leaveEvent(event);
}

void MomentCardWidget::onDeleteButtonClicked()
{
    emit deleteRequested(m_moment.id(), m_moment.text());
}

const Moment& MomentCardWidget::moment() const
{
    return m_moment;
}

void MomentCardWidget::updateData(const Moment& newMoment)
{
    // 1. 更新内部数据模型
    m_moment = newMoment;

    // 2. 只更新界面上变化的文本内容
    m_textLabel->setText(newMoment.text());

}
