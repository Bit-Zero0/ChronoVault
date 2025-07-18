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
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(4);

    // 图片预览区，将局部变量赋值给成员变量
    m_imageLabel = new QLabel();
    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_imageLabel->setFixedSize(150, 80);
    if (!moment.imagePaths().isEmpty()) {
        QPixmap pixmap(moment.imagePaths().first());
        m_imageLabel->setPixmap(pixmap.scaled(m_imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        m_imageLabel->setText(tr("无图片"));
        m_imageLabel->setStyleSheet("background-color: #f0f0f0; border-radius: 3px; color: gray;");
    }
    mainLayout->addWidget(m_imageLabel);

    // 时间戳标签，将局部变量赋值给成员变量
    m_timestampLabel = new QLabel(moment.timestamp().toString("yyyy-MM-dd"));
    m_timestampLabel->setStyleSheet("font-size: 11px; color: gray;");
    mainLayout->addWidget(m_timestampLabel);

    // 文字摘要，将局部变量赋值给成员变量
    m_textLabel = new QLabel(moment.text());
    m_textLabel->setStyleSheet("font-size: 12px;");
    mainLayout->addWidget(m_textLabel, 1);
}

void MomentCardWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_moment);
    }
    QFrame::mouseReleaseEvent(event);
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

    // 也可以在这里添加更新图片和时间戳的逻辑（如果它们也会变的话）
    // QPixmap pixmap(newMoment.imagePaths().first());
    // m_imageLabel->setPixmap(pixmap.scaled(m_imageLabel->size(), ...));
    // m_timestampLabel->setText(newMoment.timestamp().toString("yyyy-MM-dd"));
}
