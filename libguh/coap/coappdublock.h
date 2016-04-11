/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015-2016 Simon Stuerz <simon.stuerz@guh.guru>           *
 *                                                                         *
 *  This file is part of QtCoap.                                           *
 *                                                                         *
 *  QtCoap is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 3 of the License.                *
 *                                                                         *
 *  QtCoap is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the           *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with QtCoap. If not, see <http://www.gnu.org/licenses/>.         *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef COAPPDUBLOCK_H
#define COAPPDUBLOCK_H

#include <QByteArray>

class CoapPduBlock
{
public:
    CoapPduBlock();
    CoapPduBlock(const QByteArray &blockData);

    static QByteArray createBlock(const int &blockNumber, const int &blockSize = 2, const bool &moreFlag = false);

    int blockNumber() const;
    int blockSize() const;
    bool moreFlag() const;

private:
    int m_blockNumber;
    int m_blockSize;
    bool m_moreFlag;

};

#endif // COAPPDUBLOCK_H
