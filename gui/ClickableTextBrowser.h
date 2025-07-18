#pragma once

#include <QTextBrowser>
#include <QMouseEvent>

class ClickableTextBrowser : public QTextBrowser
{
    Q_OBJECT
public:
    using QTextBrowser::QTextBrowser; // 继承父类的构造函数

signals:
    void clicked();

protected:
    // 重写鼠标按下事件，当用户点击时发射 clicked 信号
    void mousePressEvent(QMouseEvent *e) override {
        if (e->button() == Qt::LeftButton) {
            emit clicked();
        }
        QTextBrowser::mousePressEvent(e);
    }
};
