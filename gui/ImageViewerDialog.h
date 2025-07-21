#pragma once

#include <QDialog>
#include <QString>
#include "gui/ZoomableGraphicsView.h"

QT_BEGIN_NAMESPACE
class QLabel;
// class QGraphicsView;
class QGraphicsScene;
class QVBoxLayout;
QT_END_NAMESPACE

class ImageViewerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImageViewerDialog(const QString& imagePath, QWidget *parent = nullptr);


protected:
    void showEvent(QShowEvent *event) override;
private:
    // QGraphicsView* m_view;
    QGraphicsScene* m_scene;
    ZoomableGraphicsView* m_view;
    bool m_isInitialShow;
};
