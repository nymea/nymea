/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
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
