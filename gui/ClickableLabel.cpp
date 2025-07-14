#include "gui/ClickableLabel.h"
#include <QMouseEvent>

ClickableLabel::ClickableLabel(const QString& path, QWidget *parent)
    : QLabel(parent), m_imagePath(path)
{
}

void ClickableLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_imagePath);
    }
    QLabel::mouseReleaseEvent(event);
}
