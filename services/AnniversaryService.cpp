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
    // 初始化保存路径
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataDir.isEmpty()) { dataDir = "data"; }
    QDir dir(dataDir);
    if (!dir.exists()) { dir.mkpath("."); }
    m_savePath = dataDir + "/anniversaries.json";

    // 加载数据
    loadData();

    // 设置并启动定时器，每秒检查一次提醒
    m_reminderTimer = new QTimer(this);
    connect(m_reminderTimer, &QTimer::timeout, this, &AnniversaryService::checkReminders);
    m_reminderTimer->start(1000);
}

AnniversaryService::~AnniversaryService() {
    // 在程序退出时，不再使用后台线程，而是直接、同步地调用保存函数
    // 确保数据一定能被完整写入后，程序才退出。
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
    emit itemsChanged(); // 用这个信号同时触发分类和项目列表的刷新
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

// services/AnniversaryService.cpp

void AnniversaryService::checkReminders() {
    if (!m_trayIcon) return;

    QDateTime now = QDateTime::currentDateTime();
    bool dataChanged = false;

    for (AnniversaryItem& item : m_items) {
        // 【核心修正】在检查提醒时间前，增加一个额外的判断
        // 确保只有当事件本身还在未来时，我们才处理它的提醒
        // 这可以防止在倒计时结束的临界点发生意外的重复触发
        if (item.targetDateTime() > now && item.reminderDateTime().isValid() && item.reminderDateTime() <= now) {

            qDebug() << "Anniversary Reminder Triggered:" << item.title();
            m_trayIcon->showMessage(
                tr("ChronoVault 纪念日提醒"),
                item.title(),
                QSystemTrayIcon::Information,
                5000
                );

            // 清除提醒时间，这是防止重复的关键
            item.setReminderDateTime(QDateTime());
            dataChanged = true;
        }

        // 检查目标时间是否已过，以便为周期性事件计算下一个日期
        // (这部分逻辑保持不变)
        if (item.targetDateTime().isValid() && item.targetDateTime() <= now) {
            if (item.recurrence() != AnniversaryRecurrence::None) {
                calculateNextTargetDateTime(item);
                dataChanged = true;
            }
        }
    }

    if (dataChanged) {
        emit itemsChanged();
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
        return; // 非周期性事件，无需计算
    }

    // 将计算出的下一个日期，与原始时间结合，形成下一个目标QDateTime
    item.setTargetDateTime(QDateTime(nextDate, item.targetDateTime().time()));

    // 在这里，我们还可以根据用户的设置，重新计算下一次的提醒时间
    // 例如：item.setReminderDateTime(item.targetDateTime().addDays(-3));
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

    // 【诊断日志 1】检查加载到服务层后，第一个项目有多少个“瞬间”
    if (!m_items.isEmpty()) {
        qDebug() << "[DIAGNOSTIC 1] In AnniversaryService::loadData, first item has"
                 << m_items.first().moments().count() << "moments.";
    }

    emit itemsChanged();
}

void AnniversaryService::saveData() const {
    // 在程序正常运行时，依然使用后台线程保存，避免UI卡顿
    // 通过 (void) 转换，明确告诉编译器我们“有意忽略”返回值，以消除警告
    (void)QtConcurrent::run([this, lists = this->m_items] {
        this->saveDataInBackground(lists);
    });
}

void AnniversaryService::deleteItem(const QUuid& id) {
    if (id.isNull()) return; // 增加对null id的防御
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
        emit itemsChanged(); // 发射信号，让UI刷新（按钮会消失）
        saveData();
    }
}

void AnniversaryService::updateTargetDateTime(const QUuid& id, const QDateTime& newDateTime)
{
    if (auto* item = findItemById(id)) {
        item->setTargetDateTime(newDateTime);
        // 如果提醒时间晚于新的目标时间，也一并更新提醒时间
        if (item->reminderDateTime() > newDateTime) {
            item->setReminderDateTime(newDateTime);
        }
        emit itemsChanged(); // 通知UI刷新倒计时
        saveData();
    }
}




// 删除分类的逻辑
void AnniversaryService::deleteCategory(const QString& categoryName) {
    if (!m_categories.contains(categoryName)) return;

    // 弹出确认对话框的逻辑应该在UI层，服务层只负责执行
    // 步骤1: 将该分类下的所有项目变为“未分类”
    for (auto& item : m_items) {
        if (item.category() == categoryName) {
            item.setCategory("");
        }
    }
    // 步骤2: 从分类列表中移除该分类
    m_categories.removeAll(categoryName);

    emit itemsChanged();
    saveData();
}


// 重命名分类的逻辑
void AnniversaryService::renameCategory(const QString& oldName, const QString& newName) {
    if (newName.isEmpty() || oldName == newName || m_categories.contains(newName)) {
        return;
    }
    // 1. 更新所有相关项目
    for (auto& item : m_items) {
        if (item.category() == oldName) {
            item.setCategory(newName);
        }
    }
    // 2. 更新分类列表自身
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
        qDebug() << "[Service] Moment" << moment.id() << "added. Emitting itemsChanged() signal."; // 【新增日志】
        emit itemsChanged();
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

void AnniversaryService::updateMoment(const QUuid& anniversaryId, const Moment& updatedMoment)
{
    // findItemById 返回的是一个非const指针，因此可以修改
    if (auto* item = findItemById(anniversaryId)) {
        // item->moments() 现在调用的是非const版本，允许修改
        for (auto& moment : item->moments()) {
            if (moment.id() == updatedMoment.id()) {
                moment = updatedMoment; // 现在这个赋值操作是有效的
                emit itemsChanged();
                saveData();
                qDebug() << "Service: Moment" << moment.id() << "successfully updated.";
                return;
            }
        }
    }
}

void AnniversaryService::deleteMoment(const QUuid& anniversaryId, const QUuid& momentId)
{
    if (auto* item = findItemById(anniversaryId)) {
        int initialCount = item->moments().count();
        item->moments().removeIf([&](const Moment& moment){
            return moment.id() == momentId;
        });

        if (item->moments().count() < initialCount) {
            qDebug() << "[Service] Moment" << momentId << "deleted. Emitting itemsChanged() signal.";
            emit itemsChanged(); // 【关键】这个信号是通知外界数据已改变
            saveData();
        }
    }
}
