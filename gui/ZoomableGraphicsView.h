#pragma once

#include <QGraphicsView>
#include <QWheelEvent>

class ZoomableGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ZoomableGraphicsView(QWidget *parent = nullptr);

protected:
    // 重写滚轮事件，来实现缩放功能
    void wheelEvent(QWheelEvent *event) override;
};
