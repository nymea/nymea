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

#include "zwavenode.h"

#include <QDebug>

ZWaveNode::ZWaveNode(QObject *parent)
    : QObject{parent}
{}

QDebug operator<<(QDebug debug, ZWaveNode *node)
{
    QDebugStateSaver saver(debug);
    debug.nospace().noquote() << "\nNode ID: " << node->nodeId() << (node->name().isEmpty() ? "" : ", " + node->name()) << " Z-Wave Version: " << node->version()
                              << (node->isZWavePlusDevice() ? "+" : "") << "\n"
                              << "├ Type: " << node->nodeType() << "\n"
                              << "├ Role: " << node->role() << "\n"
                              << "├ DeviceType: " << node->deviceType() << ", Plus DeviceType: " << node->plusDeviceType() << "\n"
                              << "├ Manufacturer: " << node->manufacturerName() << " (" << QString("0x%1").arg(node->manufacturerId(), 4, 16, QChar('0')) << ")\n"
                              << "├ Product: " << node->productName() << " (" << QString("0x%1").arg(node->productId(), 4, 16, QChar('0'))
                              << "), Product type: " << QString("0x%1").arg(node->productType(), 4, 16, QChar('0')) << "\n";
    QMap<int, QList<ZWaveValue>> byInstance;
    foreach (const ZWaveValue &value, node->values()) {
        byInstance[value.instance()].append(value);
    }
    for (int i = 0; i < byInstance.count(); i++) {
        int instance = byInstance.keys().at(i);
        bool isLastInstance = i == byInstance.count() - 1;
        if (isLastInstance) {
            debug.nospace().noquote() << "└ Instance: " << instance << "\n";
        } else {
            debug.nospace().noquote() << "├ Instance: " << instance << "\n";
        }
        QMap<ZWaveValue::Genre, QList<ZWaveValue>> byGenre;
        foreach (const ZWaveValue &value, byInstance[instance]) {
            byGenre[value.genre()].append(value);
        }
        QString instancePrefix = (isLastInstance ? "  " : "│ ");

        for (int j = 0; j < byGenre.count(); j++) {
            ZWaveValue::Genre genre = byGenre.keys().at(j);
            bool isLastGenre = j == byGenre.count() - 1;
            if (isLastGenre) {
                debug.nospace().noquote() << instancePrefix << "└ " << genre << "\n";
            } else {
                debug.nospace().noquote() << instancePrefix << "├ " << genre << "\n";
            }
            QList<ZWaveValue> sorted = byGenre[genre];
            std::sort(sorted.begin(), sorted.end(), [](const ZWaveValue &left, const ZWaveValue &right) { return left.index() < right.index(); });

            QString genrePrefix = instancePrefix + (isLastGenre ? "  " : "│ ");

            for (int k = 0; k < sorted.count(); k++) {
                const ZWaveValue &value = sorted.at(k);
                bool isLastIndex = k == sorted.count() - 1;
                if (isLastIndex) {
                    debug.nospace().noquote() << genrePrefix << "└ Index: " << value.index() << ", ID: " << value.id() << "\n";
                } else {
                    debug.nospace().noquote() << genrePrefix << "├ Index: " << value.index() << ", ID: " << value.id() << "\n";
                }

                QString indexPrefix = genrePrefix + (isLastIndex ? "  " : "│ ");
                debug.nospace().noquote() << indexPrefix << "├ Types: " << value.type() << ", " << value.commandClass() << "\n";
                if (value.type() == ZWaveValue::TypeList) {
                    debug.nospace().noquote() << indexPrefix << "├ Value: " << value.value().toStringList() << "\n";
                    debug.nospace().noquote() << indexPrefix << "│ └ Selection: " << value.valueListSelection() << " ("
                                              << value.value().toList().at(value.valueListSelection()).toString() << ")\n";
                } else {
                    debug.nospace().noquote() << indexPrefix << "├ Value: " << value.value().toString() << "\n";
                }
                QStringList descriptionLines = value.description().trimmed().split("\n");
                for (int l = 0; l < descriptionLines.count(); l++) {
                    bool isFirstDescription = l == 0;
                    bool isLastDescription = l == descriptionLines.count() - 1;
                    QString line = descriptionLines.at(l);
                    if (isFirstDescription) {
                        debug.nospace().noquote() << indexPrefix << "└ Description: " << line.trimmed() << "\n";
                    } else if (isLastDescription) {
                        debug.nospace().noquote() << indexPrefix << "  └ " << line.trimmed() << "\n";
                    } else {
                        debug.nospace().noquote() << indexPrefix << "  ├ " << line.trimmed() << "\n";
                    }
                }
            }
        }
    }
    return debug;
}
