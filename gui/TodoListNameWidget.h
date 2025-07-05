#pragma once
#include <QWidget>
#include <QUuid>

QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QStackedWidget;
class QMouseEvent;
QT_END_NAMESPACE

class TodoListNameWidget : public QWidget {
    Q_OBJECT

public:
    explicit TodoListNameWidget(QUuid id, const QString& name, QWidget* parent = nullptr);

    void enterEditMode();

signals:
    void listNameChanged(const QUuid& listId, const QString& newName);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private slots:
    void exitEditMode();

private:
    QUuid m_id;
    QStackedWidget* m_stackedWidget;
    QLabel* m_nameLabel;
    QLineEdit* m_nameEdit;
};
