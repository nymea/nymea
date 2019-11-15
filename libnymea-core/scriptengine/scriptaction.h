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
public:
    explicit ScriptAction(QObject *parent = nullptr);
    void classBegin() override;
    void componentComplete() override;

    QString deviceId() const;
    void setDeviceId(const QString &deviceId);

    QString actionTypeId() const;
    void setActionTypeId(const QString &actionTypeId);

public slots:
    void execute(const QVariantList &params);

signals:
    void deviceIdChanged();
    void actionTypeIdChanged();

public:
    DeviceManager *m_deviceManager = nullptr;
    QString m_deviceId;
    QString m_actionTypeId;
};

}

#endif // SCRIPTACTION_H
