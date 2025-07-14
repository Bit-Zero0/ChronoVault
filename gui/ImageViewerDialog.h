#pragma once

#include <QDialog>
#include <QString>

QT_BEGIN_NAMESPACE
class QLabel;
class QGraphicsView;
class QGraphicsScene;
class QVBoxLayout;
QT_END_NAMESPACE

class ImageViewerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImageViewerDialog(const QString& imagePath, QWidget *parent = nullptr);

private:
    QGraphicsView* m_view;
    QGraphicsScene* m_scene;
};
