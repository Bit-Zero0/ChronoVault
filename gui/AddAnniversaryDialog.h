#pragma once
#include <QDialog>
#include "core/AnniversaryItem.h"

QT_BEGIN_NAMESPACE
class QLineEdit;
class QDateEdit;
class QDateTimeEdit;
class QComboBox;
class QFormLayout;
QT_END_NAMESPACE

class AddAnniversaryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddAnniversaryDialog(const QString& initialCategory = "", QWidget *parent = nullptr);

    // 公共接口，用于从外部获取用户配置好的 AnniversaryItem
    AnniversaryItem getAnniversaryItem() const;

private slots:
    // 当用户切换事件类型时，动态调整界面
    void onEventTypeChanged(int index);

private:
    void setupUi();

    // UI 控件
    QLineEdit* m_titleEdit;
    QComboBox* m_eventTypeComboBox;
    QDateTimeEdit* m_targetDateTimeEdit;
    QDateEdit* m_originalDateEdit; // 专用于纪念日
    QComboBox* m_recurrenceComboBox; // 专用于纪念日
    QLineEdit* m_categoryEdit;

    // 用于动态显示/隐藏的布局和控件
    QWidget* m_anniversaryOptionsWidget;
    QFormLayout* m_formLayout;
};
