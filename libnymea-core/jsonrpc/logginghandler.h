/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef LOGGINGHANDLER_H
#define LOGGINGHANDLER_H

#include "jsonrpc/jsonhandler.h"
#include "logging/logentry.h"
#include "logging/logfilter.h"

namespace nymeaserver {

class LoggingHandler : public JsonHandler
{
    Q_OBJECT
public:
    explicit LoggingHandler(QObject *parent = nullptr);
    QString name() const override;

    Q_INVOKABLE JsonReply *GetLogEntries(const QVariantMap &params) const;

signals:
    void LogEntryAdded(const QVariantMap &params);
    void LogDatabaseUpdated(const QVariantMap &params);

private:
    static QVariantMap packLogEntry(const LogEntry &logEntry);

    static LogFilter unpackLogFilter(const QVariantMap &logFilterMap);

private slots:
    void logEntryAdded(const LogEntry &entry);
    void logDatabaseUpdated();

};

}

#endif // LOGGINGHANDLER_H
