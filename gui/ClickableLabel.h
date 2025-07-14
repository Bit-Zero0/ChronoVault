#pragma once

#include <QLabel>
#include <QMouseEvent>
#include <QString>

class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    ClickableLabel(const QString& path = QString(), QWidget *parent = nullptr);
    QString imagePath() const { return m_imagePath; }
    void setImagePath(const QString& path) { m_imagePath = path; }

signals:
    void clicked(const QString& path);

protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QString m_imagePath;
};
