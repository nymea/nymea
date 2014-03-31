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

#ifndef GUHCORE_H
#define GUHCORE_H

#include "rule.h"
#include "event.h"

#include <QObject>

class JsonRPCServer;
class DeviceManager;
class RuleEngine;

class GuhCore : public QObject
{
    Q_OBJECT
public:
    static GuhCore* instance();

    DeviceManager* deviceManager() const;
    RuleEngine *ruleEngine() const;

private:
    explicit GuhCore(QObject *parent = 0);
    static GuhCore *s_instance;

    JsonRPCServer *m_jsonServer;
    DeviceManager *m_deviceManager;
    RuleEngine *m_ruleEngine;

private slots:
    void gotSignal(const Event &event);

};

#endif // GUHCORE_H
