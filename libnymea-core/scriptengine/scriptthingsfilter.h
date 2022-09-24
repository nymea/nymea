#ifndef SCRIPTTHINGSFILTER_H
#define SCRIPTTHINGSFILTER_H

#include <QSortFilterProxyModel>

class ScriptThingsFilter : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY()
public:
    explicit ScriptThingsFilter(QObject *parent = nullptr);

signals:

};

#endif // SCRIPTTHINGSFILTER_H
