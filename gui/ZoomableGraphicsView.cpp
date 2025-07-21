#include "gui/ZoomableGraphicsView.h"
#include <QWheelEvent>
#include <QDebug>

ZoomableGraphicsView::ZoomableGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
}

void ZoomableGraphicsView::wheelEvent(QWheelEvent *event)
{
    // 根据滚轮滚动的方向，确定缩放因子
    double scaleFactor = 1.15;
    if (event->angleDelta().y() < 0) {
        // 向下滚动，缩小
        scaleFactor = 1.0 / scaleFactor;
    }

    // 对视图进行缩放
    scale(scaleFactor, scaleFactor);
}
