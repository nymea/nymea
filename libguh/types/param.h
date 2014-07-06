/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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

#ifndef PARAM_H
#define PARAM_H

#include <QString>
#include <QVariant>

class Param
{
public:
    Param(const QString &name = QString(), const QVariant &value = QVariant());

    QString name() const;
    void setName(const QString &name);

    QVariant value() const;
    void setValue(const QVariant &value);

    bool isValid() const;

private:
    QString m_name;
    QVariant m_value;
};

Q_DECLARE_METATYPE(Param)
QDebug operator<<(QDebug dbg, const Param &param);
QDebug operator<<(QDebug dbg, const QList<Param> &params);

class ParamList: public QList<Param>
{
public:
    bool hasParam(const QString &paramName) const;
    QVariant paramValue(const QString &paramName) const;
    void setParamValue(const QString &paramName, const QVariant &value);
};

#endif // PARAM_H
