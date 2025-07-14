#pragma once

#include <QDialog>
#include "core/Moment.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QTextBrowser;
class QTextEdit;
class QStackedWidget;
class QToolButton;
class QTimer;
class QUuid;
class QCloseEvent;
QT_END_NAMESPACE

class MomentDetailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MomentDetailDialog(const Moment& moment, const QUuid& anniversaryId, QWidget *parent = nullptr);
    ~MomentDetailDialog();

signals:
    void momentUpdated(const QUuid& anniversaryId, const Moment& updatedMoment);

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void showNextImage();
    void showPreviousImage();
    void onTextChanged();
    void performAutoSave();
    void switchToPreviewMode();
    void switchToEditMode(); // 【新增】

private:
    void setupUi(const Moment& moment);
    void updateImageCounter();

    Moment m_currentMoment;
    QUuid m_anniversaryId;
    bool m_isDirty = false;

    // UI 控件
    QStackedWidget* m_imageStack;
    QLabel* m_imageCounterLabel;
    QToolButton* m_prevButton;
    QToolButton* m_nextButton;
    QStackedWidget* m_textStack;
    QTextEdit* m_textEdit;
    QTextBrowser* m_textBrowser;

    // 定时器
    QTimer* m_autoSaveTimer;
    QTimer* m_switchToPreviewTimer;
};
