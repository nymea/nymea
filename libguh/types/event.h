/****************************************************************************
 *                                                                          *
 *  This file is part of guh.                                               *
 *                                                                          *
 *  Guh is free software: you can redistribute it and/or modify             *
 *  it under the terms of the GNU General Public License as published by    *
 *  the Free Software Foundation, version 2 of the License.                 *
 *                                                                          *
 *  Guh is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *  GNU General Public License for more details.                            *
 *                                                                          *
 *  You should have received a copy of the GNU General Public License       *
 *  along with guh.  If not, see <http://www.gnu.org/licenses/>.            *
 *                                                                          *
 ***************************************************************************/

#ifndef EVENT_H
#define EVENT_H

#include <QString>
#include <QUuid>
#include <QVariantList>

class Event
{
public:
    Event(const QUuid &eventTypeId, const QUuid &deviceId, const QVariantMap &params);

    QUuid eventTypeId() const;
    QUuid deviceId() const;

    QVariantMap params() const;
    void setParams(const QVariantMap &params);

    bool operator ==(const Event &other) const;

private:
    QUuid m_eventTypeId;
    QUuid m_deviceId;
    QVariantMap m_params;
};

#endif // EVENT_H
