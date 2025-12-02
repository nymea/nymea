// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef DEBUGREPORTGENERATOR_H
#define DEBUGREPORTGENERATOR_H

#include <QDir>
#include <QObject>
#include <QProcess>

namespace nymeaserver {

class DebugReportGenerator : public QObject
{
    Q_OBJECT
public:
    explicit DebugReportGenerator(QObject *parent = nullptr);
    ~DebugReportGenerator();

    QByteArray reportFileData() const;
    QString reportFileName();
    QString md5Sum() const;

    bool isReady() const;
    bool isValid() const;

    void generateReport();

private:
    QDir m_reportDirectory;
    QString m_reportFileName;
    bool m_isReady = false;
    bool m_isValid = false;

    QProcess *m_compressProcess = nullptr;
    QList<QProcess *> m_runningProcesses;

    QByteArray m_reportFileData;
    QString m_md5Sum;

    void copyFileToReportDirectory(const QString &fileName, const QString &subDirectory = QString());
    void verifyRunningProcessesFinished();

    void saveSystemInformation();
    void saveLogFiles();
    void saveConfigs();
    void saveEnv();

    void cleanupReport();

signals:
    void finished(bool success);
    void timeout();

private slots:
    void onPingProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onDigProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onTracePathProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onCompressProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

};

}

#endif // DEBUGREPORTGENERATOR_H
