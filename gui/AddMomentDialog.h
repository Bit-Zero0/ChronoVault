#pragma once

#include <QDialog>
#include <QStringList>
#include "core/Moment.h"

QT_BEGIN_NAMESPACE
class QTextEdit;
class QTextBrowser;
class QLabel;
class QHBoxLayout;
class QScrollArea;
class QTabWidget;
QT_END_NAMESPACE

class AddMomentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddMomentDialog(QWidget* parent = nullptr);

    // 获取用户输入的 Moment 对象
    Moment getMoment() const;

private slots:
    void onAddImageClicked();
    void onTabChanged(int index);

private:
    void setupUi();

    // UI 控件
    QTabWidget* m_editorTabWidget;
    QTextEdit* m_markdownEditor;      // "编辑"页
    QTextBrowser* m_markdownPreview;  // "预览"页

    QScrollArea* m_imagePreviewArea;
    QWidget* m_imagePreviewWidget;
    QHBoxLayout* m_imagePreviewLayout;

    // 用于存储选择的图片路径列表
    QStringList m_selectedImagePaths;
};
