/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  This file is part of guh.                                              *
 *                                                                         *
 *  Guh is free software: you can redistribute it and/or modify            *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  Guh is distributed in the hope that it will be useful,                 *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with guh. If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef BOBCLIENT_H
#define BOBCLIENT_H

#include <QObject>
#include <QTimer>
#include <QMap>
#include <QColor>
#include <QTime>

class BobClient : public QObject
{
    Q_OBJECT
public:
    explicit BobClient(QObject *parent = 0);
    bool connect(const QString &hostname, int port);
    bool connected() const;

    int lightsCount();
    QColor currentColor(int channel);

    void setPriority(int priority);
    
signals:
    
public slots:
    void setColor(int channel, QColor color);
    void sync();

private:
    QMap<int, QColor> m_colors; //channel, color
    void *m_boblight;
    bool m_connected;
    QString m_hostname;
    int m_port;

    QTime m_lastSyncTime;
    QTimer m_resyncTimer;
};

#endif // BOBCLIENT_H
