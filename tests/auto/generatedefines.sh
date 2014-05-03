#!/bin/bash

echo generating defines... $@

cat << EOF > testdefines.h 
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

#include <QMetaType>
EOF

for i in $@; do
  echo \#include \"${i,,}.h\" >> testdefines.h
done

echo >> testdefines.h

echo \#define REGISTER_METATYPES \\ >> testdefines.h

for i in $@; do
  echo qRegisterMetaType\<$i\>\(\"$i\"\)\; \\ >> testdefines.h
done

echo >> testdefines.h

echo \#define TESTCASES \"$@\" >> testdefines.h
