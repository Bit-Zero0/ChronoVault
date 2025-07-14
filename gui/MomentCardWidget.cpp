#include "gui/MomentCardWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QMouseEvent>

MomentCardWidget::MomentCardWidget(const Moment& moment, QWidget *parent)
    : QFrame(parent), m_moment(moment)
{
    setupUi(moment);
}

void MomentCardWidget::setupUi(const Moment& moment) {
    setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    setCursor(Qt::PointingHandCursor);
    setFixedSize(160, 130); // 稍微调整尺寸以容纳时间戳
    setStyleSheet("MomentCardWidget { border: 1px solid #e0e0e0; border-radius: 4px; background-color: white; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(4);

    // 图片预览区
    QLabel* imageLabel = new QLabel();
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setFixedSize(150, 80); // 固定图片区域大小
    if (!moment.imagePaths().isEmpty()) {
        QPixmap pixmap(moment.imagePaths().first());
        imageLabel->setPixmap(pixmap.scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        imageLabel->setText(tr("无图片"));
        imageLabel->setStyleSheet("background-color: #f0f0f0; border-radius: 3px; color: gray;");
    }
    mainLayout->addWidget(imageLabel);

    // 【新增】时间戳标签
    QLabel* timestampLabel = new QLabel(moment.timestamp().toString("yyyy-MM-dd"));
    timestampLabel->setStyleSheet("font-size: 11px; color: gray;");
    mainLayout->addWidget(timestampLabel);

    // 文字摘要
    QLabel* textLabel = new QLabel(moment.text());
    textLabel->setStyleSheet("font-size: 12px;");
    mainLayout->addWidget(textLabel, 1); // 占据剩余空间
}

void MomentCardWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_moment);
    }
    QFrame::mouseReleaseEvent(event);
}
