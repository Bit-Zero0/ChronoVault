#include "gui/ImageViewerDialog.h"
#include "gui/ZoomableGraphicsView.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPixmap>
#include <QVBoxLayout>
#include <QScreen>
#include <QDebug>
#include <QGuiApplication>
#include <QLabel>


ImageViewerDialog::ImageViewerDialog(const QString& imagePath, QWidget *parent)
    : QDialog(parent), m_isInitialShow(true)
{
    setWindowTitle(tr("查看原图 - 使用滚轮缩放"));
    setModal(true);

    // 1. 使用我们创建的、支持缩放的 ZoomableGraphicsView
    m_view = new ZoomableGraphicsView(this);
    m_scene = new QGraphicsScene(this);
    m_view->setScene(m_scene);

    // 2. 加载图片
    QPixmap pixmap(imagePath);
    if (pixmap.isNull()) {
        qWarning() << "Failed to load image:" << imagePath;
        QLabel* errorLabel = new QLabel(tr("无法加载图片。"));
        QVBoxLayout* errorLayout = new QVBoxLayout(this);
        errorLayout->addWidget(errorLabel);
        // 如果加载失败，设置一个最小尺寸，避免窗口过小
        setMinimumSize(200, 100);
        return;
    }
    m_scene->addPixmap(pixmap);

    // 3. 设置布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0); // 移除边距，让视图填满窗口
    mainLayout->addWidget(m_view);

    // 4. 【核心逻辑】调整窗口大小以适应图片，而不是让图片适应窗口

    // 4a. 获取主屏幕的可用几何尺寸
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    QSize initialSize = pixmap.size().scaled(screenGeometry.size() * 0.8, Qt::KeepAspectRatio);
    resize(initialSize);
}

void ImageViewerDialog::showEvent(QShowEvent *event)
{
    // 首先，调用基类的实现，这是必须的
    QDialog::showEvent(event);

    // 检查是否是第一次显示这个窗口
    if (m_isInitialShow) {
        // 在这里调用 fitInView，因为此时窗口和视图已经拥有了正确的尺寸
        m_view->fitInView(m_scene->itemsBoundingRect(), Qt::KeepAspectRatio);
        // 将标志位设为 false，防止在窗口被隐藏后再次显示时重复缩放
        m_isInitialShow = false;
    }
}
