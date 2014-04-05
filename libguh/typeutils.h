#ifndef TYPEUTILS_H
#define TYPEUTILS_H

#include <QMetaType>
#include <QUuid>

#define DECLARE_TYPE_ID(type) class type##Id: public QUuid \
{ \
public: \
    type##Id(const QString &uuid): QUuid(uuid) {} \
    type##Id(): QUuid() {} \
    static type##Id create##type##Id() { return type##Id(QUuid::createUuid().toString()); } \
    static type##Id fromUuid(const QUuid &uuid) { return type##Id(uuid.toString()); } \
}; \
Q_DECLARE_METATYPE(type##Id);


DECLARE_TYPE_ID(Vendor)
DECLARE_TYPE_ID(DeviceClass)
DECLARE_TYPE_ID(Device)

DECLARE_TYPE_ID(EventType)
DECLARE_TYPE_ID(StateType)
DECLARE_TYPE_ID(ActionType)


#endif // TYPEUTILS_H
