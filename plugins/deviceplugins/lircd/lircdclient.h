/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
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

#ifndef LIRCCLIENT_H
#define LIRCCLIENT_H

#include <QObject>
#include <QLocalSocket>
#include <QStringList>

class LircClient : public QObject
{
    Q_OBJECT
public:
    explicit LircClient(QObject *parent = 0);

    bool connect();

signals:
    void buttonPressed(const QString &remoteName, const QString &buttonName, int repeat);

private slots:
    void readyRead();

private:
    void readRemotes();

private:
    QLocalSocket *m_socket;

    QStringList m_remotes;
};

#endif // LIRCCLIENT_H
