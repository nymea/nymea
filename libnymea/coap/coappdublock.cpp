// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea.
*
* nymea is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <math.h>

#include "coappdublock.h"

CoapPduBlock::CoapPduBlock() {}

CoapPduBlock::CoapPduBlock(const QByteArray &blockData)
{
    if (blockData.size() == 1) {
        quint8 block = (quint8) blockData.at(0);
        m_blockNumber = (block & 0xf0) >> 4;
        m_blockSize = (quint8) pow(2, (block & 0x07) + 4);
        m_moreFlag = (bool) ((block & 0x8) >> 3);
    } else if (blockData.size() == 2) {
        quint16 block = (quint16) blockData.toHex().toUInt(0, 16);
        m_blockNumber = (int) (block & 0xff0) >> 4;
        m_blockSize = (int) pow(2, (block & 0x07) + 4);
        m_moreFlag = (bool) ((block & 0x08) >> 3);
    }
}

QByteArray CoapPduBlock::createBlock(const int &blockNumber, const int &blockSize, const bool &moreFlag)
{
    QByteArray blockData;
    if (blockNumber < 16) {
        quint8 block = (quint8) blockSize;
        block |= (quint8) moreFlag << 3;
        block |= (quint8) blockNumber << 4;
        blockData = QByteArray(1, (char) block);
    } else {
        quint16 block = (quint16) blockSize;
        block |= (quint16) moreFlag << 3;
        block |= (quint8) blockNumber << 4;
        blockData.resize(2);
        blockData.fill(0);
        blockData[0] = (quint8) (block >> 8);
        blockData[1] = (quint8) (block & 0xff);
    }
    return blockData;
}

int CoapPduBlock::blockNumber() const
{
    return m_blockNumber;
}

int CoapPduBlock::blockSize() const
{
    return m_blockSize;
}

bool CoapPduBlock::moreFlag() const
{
    return m_moreFlag;
}
