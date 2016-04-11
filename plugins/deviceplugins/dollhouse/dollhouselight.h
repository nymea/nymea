#ifndef DOLLHOUSELIGHT_H
#define DOLLHOUSELIGHT_H

#include <QObject>
#include <QColor>

class DollhouseLight : public QObject
{
    Q_OBJECT
public:
    explicit DollhouseLight(QObject *parent = 0);

    QString name() const;
    void setName(const QString &name);

    QString connectionUuid() const;
    void setConnectionUuid(const QString &connectionUuid);

    QString hostAddress() const;
    void setHostAddress(const QString &address);

    int lightId() const;
    void setLightId(const int &lightId);

    // properties
    QColor color() const;
    void setColor(const QColor &color);

    int brightness() const;
    void setBrightness(const int &brightness);

    bool power() const;
    void setPower(const bool &power);

private:
    QString m_name;
    QString m_connectionUuid;
    QString m_hostAddress;
    int m_lightId;

    QColor m_color;
    int m_brightness;
    bool m_power;

};

#endif // DOLLHOUSELIGHT_H
