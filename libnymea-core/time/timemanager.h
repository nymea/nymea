/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <QTimer>
#include <QObject>
#include <QDateTime>
#include <QTimeZone>

namespace nymeaserver {

class TimeManager : public QObject
{
    Q_OBJECT
public:
    explicit TimeManager(QObject *parent = nullptr);

    QDateTime currentDateTime() const;

    // For testability only
    void stopTimer();
    void setTime(const QDateTime &dateTime);

signals:
    void tick();
    void dateTimeChanged(const QDateTime &dateTime);

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    int m_timerId = 0;
    QDateTime m_lastEvent;

    // For testability
    qint64 m_overrideDifference = 0;
};

}

#endif // TIMEMANAGER_H
