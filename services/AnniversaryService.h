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

    // --- �����ӿ� ---
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

    // ���ڴ� main.cpp ����ϵͳ����ͼ��ָ��
    void setTrayIcon(QSystemTrayIcon* trayIcon);

    void addMomentToItem(const QUuid& anniversaryId, const Moment& moment);

signals:
    // ���������б����仯ʱ��������ź�֪ͨUIˢ��
    void itemsChanged();

private slots:
    // ��ʱ������ʱ���õĲۺ���
    void checkReminders();

private:
    explicit AnniversaryService(QObject* parent = nullptr);
    AnniversaryService(const AnniversaryService&) = delete;
    AnniversaryService& operator=(const AnniversaryService&) = delete;

    void saveDataInBackground(const QList<AnniversaryItem> listsToSave) const;

    // ˽�и�������
    void loadData();
    void saveData() const;
    void calculateNextTargetDateTime(AnniversaryItem& item); // �����ġ������´η���ʱ��ĺ���

    QList<AnniversaryItem> m_items;     // �ڴ��г��е����м�������Ŀ
    QString m_savePath;                 // ���ݱ���·�� (anniversaries.json)
    QTimer* m_reminderTimer;            // ���ڼ�����ѵĶ�ʱ��
    QSystemTrayIcon* m_trayIcon;        // ϵͳ����ͼ�꣬���ڷ���֪ͨ
    QStringList m_categories;
};
