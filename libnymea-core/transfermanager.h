// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TRANSFERMANAGER_H
#define TRANSFERMANAGER_H

#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QUuid>
#include <QVariantMap>

#include "jsonrpc/jsoncontext.h"

namespace nymeaserver {

class TransferManager : public QObject
{
    Q_OBJECT
public:
    enum class Direction {
        Upload,
        Download
    };

    struct TransferSessionInfo
    {
        QString transferId;
        QString transferToken;
        QString fileName;
        qint64 size = 0;
    };

    struct DownloadInfo
    {
        QString downloadId;
        QString fileName;
        qint64 size = 0;
    };

    explicit TransferManager(QObject *parent = nullptr);

    TransferSessionInfo createUpload(const QString &fileName, qint64 size, const JsonContext &context);
    DownloadInfo createDownload(const QString &fileName, const QByteArray &data, const JsonContext &context, bool emitNotification = false);
    TransferSessionInfo createDownloadTransfer(const QString &downloadId, const JsonContext &context);

    bool uploadTransferExists(const QString &transferId) const;
    bool validateTransferToken(const QString &transferId, const QString &transferToken) const;
    Direction transferDirection(const QString &transferId) const;

    qint64 transferSize(const QString &transferId) const;
    QString transferFileName(const QString &transferId) const;
    qint64 transferOffset(const QString &transferId) const;

    bool appendUploadData(const QString &transferId, const QByteArray &data, QString *errorString = nullptr);
    DownloadInfo finishUpload(const QString &transferId, QString *errorString = nullptr);

    QByteArray readDownloadChunk(const QString &transferId, int maxSize, bool *finished, QString *errorString = nullptr);

signals:
    void downloadAvailable(const QUuid &clientId, const QVariantMap &params);

private:
    struct TransferSession
    {
        QString transferId;
        QString transferToken;
        Direction direction = Direction::Upload;
        QString fileName;
        qint64 size = 0;
        qint64 offset = 0;
        QByteArray data;
        QString downloadId;
        QUuid ownerClientId;
        QByteArray ownerToken;
    };

    struct DownloadEntry
    {
        QString downloadId;
        QString fileName;
        QByteArray data;
        QUuid ownerClientId;
        QByteArray ownerToken;
    };

    bool matchesOwner(const DownloadEntry &entry, const JsonContext &context) const;

    QHash<QString, TransferSession> m_transferSessions;
    QHash<QString, DownloadEntry> m_downloads;
};

} // namespace nymeaserver

#endif // TRANSFERMANAGER_H
