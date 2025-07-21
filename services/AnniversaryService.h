#pragma once

#include <QObject>
#include <QList>
#include <QTimer>
#include <QSystemTrayIcon>
#include "core/AnniversaryItem.h"

class AnniversaryService : public QObject
{
    Q_OBJECT

public:
    static AnniversaryService* instance();
    ~AnniversaryService();

    // --- 公共接口 ---
    const QList<AnniversaryItem>& getAllItems() const;
    void addItem(const AnniversaryItem& item);
    void deleteItem(const QUuid& id);

    AnniversaryItem* findItemById(const QUuid& id);
    void markAsAddedToTodo(const QUuid& id);
    void updateTargetDateTime(const QUuid& id, const QDateTime& newDateTime);
    const QStringList& getCategories() const;
    void addCategory(const QString& categoryName);
    void deleteCategory(const QString& categoryName);
    void renameCategory(const QString& oldName, const QString& newName);

    // 用于从 main.cpp 接收系统托盘图标指针
    void setTrayIcon(QSystemTrayIcon* trayIcon);

    void addMomentToItem(const QUuid& anniversaryId, const Moment& moment);

    void updateMoment(const QUuid& anniversaryId, const Moment& updatedMoment);
    void deleteMoment(const QUuid& anniversaryId, const QUuid& momentId);

signals:
    // 当纪念日列表发生变化时，发射此信号通知UI刷新
    void itemsChanged();

private slots:
    // 定时器触发时调用的槽函数
    void checkReminders();

private:
    explicit AnniversaryService(QObject* parent = nullptr);
    AnniversaryService(const AnniversaryService&) = delete;
    AnniversaryService& operator=(const AnniversaryService&) = delete;

    void saveDataInBackground(const QList<AnniversaryItem> listsToSave) const;

    // 私有辅助函数
    void loadData();
    void saveData() const;
    void calculateNextTargetDateTime(AnniversaryItem& item); // 【核心】计算下次发生时间的函数

    QList<AnniversaryItem> m_items;     // 内存中持有的所有纪念日项目
    QString m_savePath;                 // 数据保存路径 (anniversaries.json)
    QTimer* m_reminderTimer;            // 用于检查提醒的定时器
    QSystemTrayIcon* m_trayIcon;        // 系统托盘图标，用于发送通知
    QStringList m_categories;
};
