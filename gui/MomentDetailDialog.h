#pragma once

#include <QDialog>
#include <QPointer>
#include "core/Moment.h"
#include "gui/ClickableTextBrowser.h"
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
    explicit MomentDetailDialog(const Moment& moment, QWidget *parent = nullptr);
    ~MomentDetailDialog();


    Moment getMoment() const;

// signals:
//     void momentUpdated(const QUuid& anniversaryId, const Moment& updatedMoment);

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void showNextImage();
    void showPreviousImage();
    void onTextChanged();
    void switchToPreviewMode();
    void switchToEditMode(); // 【新增】

private:
    void setupUi(const Moment& moment);
    void updateImageCounter();

    Moment m_currentMoment;
    //QUuid m_anniversaryId;
    bool m_isDirty = false;

    // UI 控件
    QStackedWidget* m_imageStack;
    QLabel* m_imageCounterLabel;
    QToolButton* m_prevButton;
    QToolButton* m_nextButton;
    QStackedWidget* m_textStack;
    QTextEdit* m_textEdit;
    ClickableTextBrowser* m_textBrowser;

    // 定时器

    QPointer<QTimer> m_switchToPreviewTimer;
};
