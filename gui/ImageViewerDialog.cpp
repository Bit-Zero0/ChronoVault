#include "gui/ImageViewerDialog.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPixmap>
#include <QVBoxLayout>
#include <QScreen>
#include <QDebug>
#include <QGuiApplication>
#include <QLabel>
ImageViewerDialog::ImageViewerDialog(const QString& imagePath, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("查看原图"));
    setModal(true); // 设置为模态对话框，阻止父窗口操作

    m_view = new QGraphicsView(this);
    m_scene = new QGraphicsScene(this);
    m_view->setScene(m_scene);
    m_view->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) {
        qWarning() << "Failed to load image:" << imagePath;
        QLabel* errorLabel = new QLabel(tr("无法加载图片。"));
        QVBoxLayout* errorLayout = new QVBoxLayout(this);
        errorLayout->addWidget(errorLabel);
        setLayout(errorLayout);
        return;
    }
    m_scene->addPixmap(pixmap);
    m_view->fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_view);
    setLayout(mainLayout);

    // 根据屏幕大小调整对话框尺寸
    QSize screenSize = QGuiApplication::primaryScreen()->availableSize();
    QSize dialogSize = pixmap.size();
    dialogSize.scale(screenSize * 0.8, Qt::KeepAspectRatio); // 最大占据屏幕的80%
    resize(dialogSize);
}
