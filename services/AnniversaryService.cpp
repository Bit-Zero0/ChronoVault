#include "services/AnniversaryService.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QtConcurrent>

AnniversaryService* AnniversaryService::instance() {
    static AnniversaryService service;
    return &service;
}

AnniversaryService::AnniversaryService(QObject* parent) : QObject(parent), m_trayIcon(nullptr) {
    // ��ʼ������·��
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataDir.isEmpty()) { dataDir = "data"; }
    QDir dir(dataDir);
    if (!dir.exists()) { dir.mkpath("."); }
    m_savePath = dataDir + "/anniversaries.json";

    // ��������
    loadData();

    // ���ò�������ʱ����ÿ����һ������
    m_reminderTimer = new QTimer(this);
    connect(m_reminderTimer, &QTimer::timeout, this, &AnniversaryService::checkReminders);
    m_reminderTimer->start(1000);
}

AnniversaryService::~AnniversaryService() {
    // �ڳ����˳�ʱ������ʹ�ú�̨�̣߳�����ֱ�ӡ�ͬ���ص��ñ��溯��
    // ȷ������һ���ܱ�����д��󣬳�����˳���
    saveDataInBackground(m_items);
}

const QList<AnniversaryItem>& AnniversaryService::getAllItems() const {
    return m_items;
}

const QStringList& AnniversaryService::getCategories() const {
    return m_categories;
}

void AnniversaryService::addCategory(const QString& categoryName) {
    if (categoryName.isEmpty() || m_categories.contains(categoryName)) {
        return;
    }
    m_categories.append(categoryName);
    emit itemsChanged(); // ������ź�ͬʱ�����������Ŀ�б��ˢ��
    saveData();
}

void AnniversaryService::addItem(const AnniversaryItem& item) {
    m_items.append(item);
    const QString& category = item.category();
    if (!category.isEmpty() && !m_categories.contains(category)) {
        m_categories.append(category);
    }
    if (m_items.last().recurrence() != AnniversaryRecurrence::None) {
        calculateNextTargetDateTime(m_items.last());
    }
    emit itemsChanged();
    saveData();
}

void AnniversaryService::setTrayIcon(QSystemTrayIcon* trayIcon) {
    m_trayIcon = trayIcon;
}

void AnniversaryService::checkReminders() {
    if (!m_trayIcon) return;

    QDateTime now = QDateTime::currentDateTime();
    bool dataChanged = false; // ����Ƿ��м�����������Ҫ����

    for (AnniversaryItem& item : m_items) {
        // �������ʱ���Ƿ��ѵ�
        if (item.reminderDateTime().isValid() && item.reminderDateTime() <= now) {
            qDebug() << "Anniversary Reminder Triggered:" << item.title();
            m_trayIcon->showMessage(
                tr("ChronoVault ����������"),
                item.title(),
                QSystemTrayIcon::Information,
                5000
            );
            // �������ʱ�䣬��ֹ�ظ�����
            item.setReminderDateTime(QDateTime());
            dataChanged = true;
        }

        // ���Ŀ��ʱ���Ƿ��ѹ����Ա�Ϊ�������¼�������һ������
        if (item.targetDateTime().isValid() && item.targetDateTime() <= now) {
            if (item.recurrence() != AnniversaryRecurrence::None) {
                calculateNextTargetDateTime(item);
                dataChanged = true;
            }
        }
    }

    if (dataChanged) {
        emit itemsChanged(); // ֪ͨUIˢ�£�������µ���ʱ������
        saveData();
    }
}

void AnniversaryService::calculateNextTargetDateTime(AnniversaryItem& item) {
    QDate originalDate = item.originalDate();
    QDate today = QDate::currentDate();
    QDate nextDate;

    if (item.recurrence() == AnniversaryRecurrence::Yearly) {
        nextDate = QDate(today.year(), originalDate.month(), originalDate.day());
        if (nextDate < today) {
            nextDate = nextDate.addYears(1);
        }
    }
    else if (item.recurrence() == AnniversaryRecurrence::Monthly) {
        nextDate = QDate(today.year(), today.month(), originalDate.day());
        if (nextDate < today) {
            nextDate = nextDate.addMonths(1);
        }
    }
    else {
        return; // ���������¼����������
    }

    // �����������һ�����ڣ���ԭʼʱ���ϣ��γ���һ��Ŀ��QDateTime
    item.setTargetDateTime(QDateTime(nextDate, item.targetDateTime().time()));

    // ��������ǻ����Ը����û������ã����¼�����һ�ε�����ʱ��
    // ���磺item.setReminderDateTime(item.targetDateTime().addDays(-3));
    qDebug() << "Calculated next occurrence for" << item.title() << "is" << item.targetDateTime();
}


void AnniversaryService::loadData() {
    QFile file(m_savePath);
    if (!file.open(QIODevice::ReadOnly)) { return; }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) return;

    QJsonObject rootObj = doc.object();
    m_items.clear();
    m_categories.clear();

    if (rootObj.contains("categories") && rootObj["categories"].isArray()) {
        QJsonArray array = rootObj["categories"].toArray();
        for (const QJsonValue& value : array) { m_categories.append(value.toString()); }
    }

    if (rootObj.contains("items") && rootObj["items"].isArray()) {
        QJsonArray array = rootObj["items"].toArray();
        for (const QJsonValue& value : array) {
            m_items.append(AnniversaryItem::fromJson(value.toObject()));
        }
    }

    // �������־ 1�������ص������󣬵�һ����Ŀ�ж��ٸ���˲�䡱
    if (!m_items.isEmpty()) {
        qDebug() << "[DIAGNOSTIC 1] In AnniversaryService::loadData, first item has"
                 << m_items.first().moments().count() << "moments.";
    }

    emit itemsChanged();
}

void AnniversaryService::saveData() const {
    // �ڳ�����������ʱ����Ȼʹ�ú�̨�̱߳��棬����UI����
    // ͨ�� (void) ת������ȷ���߱��������ǡ�������ԡ�����ֵ������������
    (void)QtConcurrent::run([this, lists = this->m_items] {
        this->saveDataInBackground(lists);
    });
}

void AnniversaryService::deleteItem(const QUuid& id) {
    if (id.isNull()) return; // ���Ӷ�null id�ķ���
    const int initialCount = m_items.count();
    m_items.removeIf([id](const AnniversaryItem& item) {
        return item.id() == id;
    });
    if (m_items.count() < initialCount) {
        emit itemsChanged();
        saveData();
    }
}


AnniversaryItem* AnniversaryService::findItemById(const QUuid& id)
{
    for (auto& item : m_items) {
        if (item.id() == id) {
            return &item;
        }
    }
    return nullptr;
}

void AnniversaryService::markAsAddedToTodo(const QUuid& id)
{
    if (auto* item = findItemById(id)) {
        item->setAddedToTodo(true);
        emit itemsChanged(); // �����źţ���UIˢ�£���ť����ʧ��
        saveData();
    }
}

void AnniversaryService::updateTargetDateTime(const QUuid& id, const QDateTime& newDateTime)
{
    if (auto* item = findItemById(id)) {
        item->setTargetDateTime(newDateTime);
        // �������ʱ�������µ�Ŀ��ʱ�䣬Ҳһ����������ʱ��
        if (item->reminderDateTime() > newDateTime) {
            item->setReminderDateTime(newDateTime);
        }
        emit itemsChanged(); // ֪ͨUIˢ�µ���ʱ
        saveData();
    }
}

//void AnniversaryService::removeCategory(const QString& categoryName)
//{
//    if (categoryName.isEmpty() || categoryName == tr("������Ŀ")) {
//        return; // ������ɾ����������Ŀ������������
//    }

//    bool changed = false;
//    for (auto& item : m_items) {
//        if (item.category() == categoryName) {
//            item.setCategory(""); // ���������
//            changed = true;
//        }
//    }

//    if (changed) {
//        emit itemsChanged(); // �����źţ�����UIˢ��
//        saveData();
//    }
//}


// ɾ��������߼�
void AnniversaryService::deleteCategory(const QString& categoryName) {
    if (!m_categories.contains(categoryName)) return;

    // ����ȷ�϶Ի�����߼�Ӧ����UI�㣬�����ֻ����ִ��
    // ����1: ���÷����µ�������Ŀ��Ϊ��δ���ࡱ
    for (auto& item : m_items) {
        if (item.category() == categoryName) {
            item.setCategory("");
        }
    }
    // ����2: �ӷ����б����Ƴ��÷���
    m_categories.removeAll(categoryName);

    emit itemsChanged();
    saveData();
}


// ������������߼�
void AnniversaryService::renameCategory(const QString& oldName, const QString& newName) {
    if (newName.isEmpty() || oldName == newName || m_categories.contains(newName)) {
        return;
    }
    // 1. �������������Ŀ
    for (auto& item : m_items) {
        if (item.category() == oldName) {
            item.setCategory(newName);
        }
    }
    // 2. ���·����б�����
    for (int i = 0; i < m_categories.count(); ++i) {
        if (m_categories[i] == oldName) {
            m_categories[i] = newName;
            break;
        }
    }
    emit itemsChanged();
    saveData();
}

void AnniversaryService::addMomentToItem(const QUuid& anniversaryId, const Moment& moment)
{
    if (auto* item = findItemById(anniversaryId)) {
        item->addMoment(moment);
        emit itemsChanged(); // �����ź��Թ�UIˢ��
        saveData();
    }
}

void AnniversaryService::saveDataInBackground(const QList<AnniversaryItem> listsToSave) const {
    qDebug() << "[BG Save] Starting to save data...";
    QJsonObject rootObj;
    rootObj["categories"] = QJsonArray::fromStringList(m_categories);
    QJsonArray itemsArray;
    for (const auto& item : listsToSave) {
        itemsArray.append(item.toJson());
    }
    rootObj["items"] = itemsArray;

    QJsonDocument doc(rootObj);
    QFile file(m_savePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
        qDebug() << "[BG Save] Finished saving data.";
    } else {
        qWarning() << "[BG Save] Couldn't write to anniversaries save file:" << m_savePath;
    }
}

