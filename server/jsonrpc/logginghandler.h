/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stuerz <simon.stuerz@guh.guru>                *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
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

#ifndef LOGGINGHANDLER_H
#define LOGGINGHANDLER_H

#include "jsonhandler.h"
#include "logging/logentry.h"

namespace guhserver {

class LoggingHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit LoggingHandler(QObject *parent = 0);
    QString name() const override;

    Q_INVOKABLE JsonReply *GetLogEntries(const QVariantMap &params) const;
signals:
    void LogEntryAdded(const QVariantMap &params);

private slots:
    void logEntryAdded(const LogEntry &entry);
};

}

#endif // LOGGINGHANDLER_H
