/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU General Public License as published by the Free Software
* Foundation, GNU version 3. This project is distributed in the hope that it
* will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
* Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "logvaluetool.h"

#include <QBuffer>
#include <QByteArray>
#include <QDataStream>

LogValueTool::LogValueTool(QObject *parent) : QObject(parent)
{

}

QString LogValueTool::convertVariantToString(const QVariant &value)
{
    switch (value.type()) {
    case QVariant::Double:
        return QString::number(value.toDouble());
    case QVariant::List: {
        QStringList valueStringList;
        foreach (const QVariant &variantValue, value.toList()) {
            valueStringList.append(convertVariantToString(variantValue));
        }
        return valueStringList.join(", ");
    }
    default:
        return value.toString();
    }
}

QString LogValueTool::serializeValue(const QVariant &value)
{
    QByteArray byteArray;
    QBuffer writeBuffer(&byteArray);
    writeBuffer.open(QIODevice::WriteOnly);
    QDataStream out(&writeBuffer);
    out << value;
    writeBuffer.close();
    return QString(byteArray.toBase64());
}

QVariant LogValueTool::deserializeValue(const QString &serializedValue)
{
    QByteArray data = QByteArray::fromBase64(serializedValue.toUtf8());
    QBuffer readBuffer(&data);
    readBuffer.open(QIODevice::ReadOnly);
    QDataStream inputStream(&readBuffer);
    QVariant value;
    inputStream >> value;
    readBuffer.close();
    return value;
}
