#ifndef SCRIPTACTION_H
#define SCRIPTACTION_H

#include <QObject>
#include <QQmlParserStatus>

class DeviceManager;

namespace nymeaserver {

class ScriptAction : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QString deviceId READ deviceId WRITE setDeviceId NOTIFY deviceIdChanged)
    Q_PROPERTY(QString actionTypeId READ actionTypeId WRITE setActionTypeId NOTIFY actionTypeIdChanged)
    Q_PROPERTY(QString actionName READ actionName WRITE setActionName NOTIFY actionNameChanged)
public:
    explicit ScriptAction(QObject *parent = nullptr);
    void classBegin() override;
    void componentComplete() override;

    QString deviceId() const;
    void setDeviceId(const QString &deviceId);

    QString actionTypeId() const;
    void setActionTypeId(const QString &actionTypeId);

    QString actionName() const;
    void setActionName(const QString &actionName);

public slots:
    void execute(const QVariantList &params);

signals:
    void deviceIdChanged();
    void actionTypeIdChanged();
    void actionNameChanged();

public:
    DeviceManager *m_deviceManager = nullptr;
    QString m_deviceId;
    QString m_actionTypeId;
    QString m_actionName;
};

}

#endif // SCRIPTACTION_H
