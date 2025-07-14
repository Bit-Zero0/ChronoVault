#pragma once

#include <QFrame>
#include "core/Moment.h"

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE

class MomentCardWidget : public QFrame
{
    Q_OBJECT

public:
    explicit MomentCardWidget(const Moment& moment, QWidget *parent = nullptr);

signals:
    void clicked(const Moment& moment);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void setupUi(const Moment& moment);
    Moment m_moment;
};
