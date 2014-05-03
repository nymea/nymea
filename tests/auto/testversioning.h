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

#ifndef TESTVERSIONING_H
#define TESTVERSIONING_H

#include "guhtestbase.h"

class TestVersioning: public GuhTestBase
{
    Q_OBJECT
public:
    TestVersioning () {}
    TestVersioning (const TestVersioning &other) {}
    TestVersioning& operator=(const TestVersioning &other) {}

private slots:
    void version();
    void apiChangeBumpsVersion();
};
Q_DECLARE_METATYPE(TestVersioning);

#endif //TESTVERSIONING_H
