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

#ifndef JSONHANDLER_H
#define JSONHANDLER_H

#include "jsontypes.h"

#include <QObject>
#include <QVariantMap>

class JsonHandler : public QObject
{
    Q_OBJECT
public:
    explicit JsonHandler(QObject *parent = 0);

    virtual QString name() const = 0;

    QVariantMap introspect();

    bool hasMethod(const QString &methodName);
    bool validateParams(const QString &methodName, const QVariantMap &params);
    bool validateReturns(const QString &methodName, const QVariantMap &returns);

protected:
    void setDescription(const QString &methodName, const QString &description);
    void setParams(const QString &methodName, const QVariantMap &params);
    void setReturns(const QString &methodName, const QVariantMap &returns);

private:
    QHash<QString, QString> m_descriptions;
    QHash<QString, QVariantMap> m_params;
    QHash<QString, QVariantMap> m_returns;
};

#endif // JSONHANDLER_H
