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

#ifndef TESTJSONRPC_H
#define TESTJSONRPC_H

#include "guhtestbase.h"

class TestJSONRPC: public GuhTestBase
{
    Q_OBJECT
public:
    TestJSONRPC(QObject *parent=0): GuhTestBase(parent) {}
    TestJSONRPC (const TestJSONRPC &other) {}
    TestJSONRPC& operator=(const TestJSONRPC &other) {}

private slots:
    void testBasicCall();
    void introspect();

    void executeAction_data();
    void executeAction();

    void getActionTypes_data();
    void getActionTypes();

    void getEventTypes_data();
    void getEventTypes();

    void getStateTypes_data();
    void getStateTypes();

    void enableDisableNotifications_data();
    void enableDisableNotifications();

    void stateChangeEmitsNotifications();

    void getRules();

private:
    QStringList extractRefs(const QVariant &variant);

};
Q_DECLARE_METATYPE(TestJSONRPC)

#endif
