#pragma once
#include <QString>
#include <QDateTime>
#include <QUuid>

class SharedFile {
public:
        // 此处仅为示例结构
    SharedFile(){}

private:
    QUuid m_id;
    QString m_localFilePath;
    QString m_remoteUrl;
    QDateTime m_expiryDate;
    int m_maxDownloads;
    int m_remainingDownloads;
};
