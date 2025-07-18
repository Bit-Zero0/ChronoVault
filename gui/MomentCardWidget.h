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
    void updateData(const Moment& newMoment);

    const Moment& moment() const;
    ~MomentCardWidget();

signals:
    void clicked(const Moment& moment);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void setupUi(const Moment& moment);
    Moment m_moment;
    QLabel* m_imageLabel;
    QLabel* m_timestampLabel;
    QLabel* m_textLabel;
};
