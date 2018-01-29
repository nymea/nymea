/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Simon Stürz <simon.stuerz@guh.io>                   *
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
        break;
    case QVariant::List: {
        QStringList valueStringList;
        foreach (const QVariant &variantValue, value.toList()) {
            valueStringList.append(convertVariantToString(variantValue));
        }
        return valueStringList.join(", ");
    }
    default:
        return value.toString();
        break;
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
