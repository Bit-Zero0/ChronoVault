#pragma once
#include <QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QToolButton;
QT_END_NAMESPACE

class CompletedHeaderWidget : public QWidget {
    Q_OBJECT
public:
    explicit CompletedHeaderWidget(int completedCount, bool isExpanded, QWidget* parent = nullptr);

signals:
    void toggleExpansion();

protected:
    void mousePressEvent(QMouseEvent* event) override;

private:
    QToolButton* m_expandButton;
    QLabel* m_titleLabel;
};
