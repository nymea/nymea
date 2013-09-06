#ifndef SENSORMODEL_H
#define SENSORMODEL_H

#include <QObject>
#include <QAbstractListModel>

class SensorModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit SensorModel(QObject *parent = 0);

private:



signals:

public slots:

};

#endif // SENSORMODEL_H
