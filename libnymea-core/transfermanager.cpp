// SPDX-License-Identifier: LGPL-3.0-or-later

#include "transfermanager.h"
#include "loggingcategories.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace nymeaserver {

TransferManager::TransferManager(QObject *parent)
    : QObject(parent)
{}

TransferManager::TransferSessionInfo TransferManager::createUpload(const QString &fileName, qint64 size, const JsonContext &context)
{
    TransferSession session;
    session.transferId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    session.transferToken = QUuid::createUuid().toString(QUuid::WithoutBraces);
    session.direction = Direction::Upload;
    session.fileName = fileName;
    session.size = size;
    session.ownerClientId = context.clientId();
    session.ownerToken = context.token();

    m_transferSessions.insert(session.transferId, session);

    TransferSessionInfo info;
    info.transferId = session.transferId;
    info.transferToken = session.transferToken;
    info.fileName = session.fileName;
    info.size = session.size;
    return info;
}

TransferManager::TransferSessionInfo TransferManager::createRestoreUpload(const QString &fileName, qint64 size, const QString &targetFilePath, const JsonContext &context)
{
    QFile::remove(targetFilePath);

    TransferSession session;
    session.transferId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    session.transferToken = QUuid::createUuid().toString(QUuid::WithoutBraces);
    session.direction = Direction::Upload;
    session.uploadAction = UploadAction::RestoreBackup;
    session.fileName = fileName;
    session.targetFilePath = targetFilePath;
    session.size = size;
    session.ownerClientId = context.clientId();
    session.ownerToken = context.token();

    m_transferSessions.insert(session.transferId, session);

    TransferSessionInfo info;
    info.transferId = session.transferId;
    info.transferToken = session.transferToken;
    info.fileName = session.fileName;
    info.size = session.size;
    return info;
}

TransferManager::DownloadInfo TransferManager::createDownload(const QString &fileName, const QByteArray &data, const JsonContext &context, bool emitNotification)
{
    DownloadInfo info;

    DownloadEntry entry;
    entry.downloadId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    entry.fileName = fileName;
    entry.data = data;
    entry.ownerClientId = context.clientId();
    entry.ownerToken = context.token();
    m_downloads.insert(entry.downloadId, entry);

    if (emitNotification) {
        QVariantMap params;
        params.insert("downloadId", entry.downloadId);
        params.insert("fileName", entry.fileName);
        params.insert("size", entry.data.size());
        emit downloadAvailable(entry.ownerClientId, params);
    }

    info.downloadId = entry.downloadId;
    info.fileName = entry.fileName;
    info.size = entry.data.size();
    return info;
}

TransferManager::TransferSessionInfo TransferManager::createDownloadTransfer(const QString &downloadId, const JsonContext &context)
{
    TransferSessionInfo info;
    const DownloadEntry entry = m_downloads.value(downloadId);
    if (entry.downloadId.isEmpty() || !matchesOwner(entry, context)) {
        return info;
    }

    TransferSession session;
    session.transferId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    session.transferToken = QUuid::createUuid().toString(QUuid::WithoutBraces);
    session.direction = Direction::Download;
    session.fileName = entry.fileName;
    session.size = entry.data.size();
    session.data = entry.data;
    session.downloadId = entry.downloadId;
    session.ownerClientId = entry.ownerClientId;
    session.ownerToken = entry.ownerToken;

    m_transferSessions.insert(session.transferId, session);
    m_downloads.remove(downloadId);
    info.transferId = session.transferId;
    info.transferToken = session.transferToken;
    info.fileName = session.fileName;
    info.size = session.size;
    return info;
}

bool TransferManager::uploadTransferExists(const QString &transferId) const
{
    return m_transferSessions.contains(transferId);
}

bool TransferManager::validateTransferToken(const QString &transferId, const QString &transferToken) const
{
    const TransferSession session = m_transferSessions.value(transferId);
    return !session.transferId.isEmpty() && session.transferToken == transferToken;
}

TransferManager::Direction TransferManager::transferDirection(const QString &transferId) const
{
    return m_transferSessions.value(transferId).direction;
}

qint64 TransferManager::transferSize(const QString &transferId) const
{
    return m_transferSessions.value(transferId).size;
}

QString TransferManager::transferFileName(const QString &transferId) const
{
    return m_transferSessions.value(transferId).fileName;
}

qint64 TransferManager::transferOffset(const QString &transferId) const
{
    return m_transferSessions.value(transferId).offset;
}

bool TransferManager::appendUploadData(const QString &transferId, const QByteArray &data, QString *errorString)
{
    if (!m_transferSessions.contains(transferId)) {
        if (errorString) {
            *errorString = QStringLiteral("Unknown transfer");
        }
        return false;
    }

    TransferSession &session = m_transferSessions[transferId];
    if (session.direction != Direction::Upload) {
        if (errorString) {
            *errorString = QStringLiteral("Transfer is not an upload");
        }
        return false;
    }

    if (session.size > 0 && session.offset + data.size() > session.size) {
        if (errorString) {
            *errorString = QStringLiteral("Upload exceeds announced size");
        }
        return false;
    }

    if (session.uploadAction == UploadAction::RestoreBackup) {
        QFileInfo targetFileInfo(session.targetFilePath);
        const QString targetDirectoryPath = targetFileInfo.absolutePath();
        if (!QDir(targetDirectoryPath).exists() && !QDir().mkpath(targetDirectoryPath)) {
            if (errorString) {
                *errorString = QStringLiteral("Failed to create upload destination directory");
            }
            return false;
        }

        QFile targetFile(session.targetFilePath);
        if (!targetFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
            if (errorString) {
                *errorString = targetFile.errorString();
            }
            return false;
        }

        if (targetFile.write(data) != data.size()) {
            if (errorString) {
                *errorString = targetFile.errorString();
            }
            return false;
        }

        targetFile.close();
    } else {
        session.data.append(data);
    }

    session.offset += data.size();

    return true;
}

TransferManager::FinishedUploadInfo TransferManager::finishUpload(const QString &transferId, QString *errorString)
{
    FinishedUploadInfo info;
    if (!m_transferSessions.contains(transferId)) {
        if (errorString) {
            *errorString = QStringLiteral("Unknown transfer");
        }
        return info;
    }

    const TransferSession session = m_transferSessions.value(transferId);
    if (session.direction != Direction::Upload) {
        if (errorString) {
            *errorString = QStringLiteral("Transfer is not an upload");
        }
        return info;
    }

    if (session.size > 0 && session.offset != session.size) {
        if (errorString) {
            *errorString = QStringLiteral("Upload incomplete");
        }
        return info;
    }

    info.fileName = session.fileName;
    info.size = session.offset;

    if (session.uploadAction == UploadAction::RestoreBackup) {
        if (!QFileInfo::exists(session.targetFilePath)) {
            QFile targetFile(session.targetFilePath);
            if (!targetFile.open(QIODevice::WriteOnly)) {
                if (errorString) {
                    *errorString = targetFile.errorString();
                }
                return info;
            }
            targetFile.close();
        }
        info.restoreTriggered = true;
    } else {
        JsonContext context(session.ownerClientId, QLocale(), !session.ownerToken.isEmpty());
        context.setToken(session.ownerToken);
        const DownloadInfo downloadInfo = createDownload(session.fileName, session.data, context, true);
        info.downloadId = downloadInfo.downloadId;
        info.fileName = downloadInfo.fileName;
        info.size = downloadInfo.size;
    }

    m_transferSessions.remove(transferId);
    if (info.restoreTriggered) {
        emit restoreUploadFinished(session.transferId, session.targetFilePath);
    }

    return info;
}

QByteArray TransferManager::readDownloadChunk(const QString &transferId, int maxSize, bool *finished, QString *errorString)
{
    if (finished) {
        *finished = false;
    }

    if (!m_transferSessions.contains(transferId)) {
        if (errorString) {
            *errorString = QStringLiteral("Unknown transfer");
        }
        return QByteArray();
    }

    TransferSession &session = m_transferSessions[transferId];
    if (session.direction != Direction::Download) {
        if (errorString) {
            *errorString = QStringLiteral("Transfer is not a download");
        }
        return QByteArray();
    }

    const QByteArray chunk = session.data.mid(session.offset, maxSize);
    session.offset += chunk.size();
    if (finished) {
        *finished = session.offset >= session.data.size();
    }
    if (finished && *finished) {
        m_transferSessions.remove(transferId);
    }
    return chunk;
}

bool TransferManager::matchesOwner(const DownloadEntry &entry, const JsonContext &context) const
{
    if (!entry.ownerToken.isEmpty()) {
        return entry.ownerToken == context.token();
    }
    return entry.ownerClientId == context.clientId();
}

} // namespace nymeaserver
