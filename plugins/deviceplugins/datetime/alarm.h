#ifndef ALARM_H
#define ALARM_H

#include <QObject>
#include <QDateTime>

class Alarm : public QObject
{
    Q_OBJECT
public:
    enum TimeType {
        TimeTypeTime,
        TimeTypeDusk,
        TimeTypeSunrise,
        TimeTypeNoon,
        TimeTypeSunset,
        TimeTypeDawn
    };

    explicit Alarm(QObject *parent = 0);

    void setName(const QString &name);
    QString name() const;

    void setMonday(const bool &monday);
    bool monday() const;

    void setTuesday(const bool &tuesday);
    bool tuesday() const;

    void setWednesday(const bool &wednesday);
    bool wednesday() const;

    void setThursday(const bool &thursday);
    bool thursday() const;

    void setFriday(const bool &friday);
    bool friday() const;

    void setSaturday(const bool &saturday);
    bool saturday() const;

    void setSunday(const bool &sunday);
    bool sunday() const;

    void setMinutes(const int &minutes);
    int minutes() const;

    void setHours(const int &hours);
    int hours() const;

    void setOffset(const int &offset);
    int offset() const;

    void setDusk(const QDateTime &dusk);
    void setSunrise(const QDateTime &sunrise);
    void setNoon(const QDateTime &noon);
    void setDawn(const QDateTime &dawn);
    void setSunset(const QDateTime &sunset);

    void setTimeType(const TimeType &timeType);
    void setTimeType(const QString &timeType);
    TimeType timeType() const;

private:
    QString m_name;
    bool m_monday;
    bool m_tuesday;
    bool m_wednsday;
    bool m_thursday;
    bool m_friday;
    bool m_saturday;
    bool m_sunday;

    int m_minutes;
    int m_hours;
    int m_offset;
    TimeType m_timeType;

    QDateTime m_duskOffset;
    QDateTime m_sunriseOffset;
    QDateTime m_noonOffset;
    QDateTime m_sunsetOffset;
    QDateTime m_dawnOffset;

    QDateTime calculateOffsetTime(const QDateTime &dateTime) const;

    bool checkDayOfWeek(const QDateTime &dateTime);
    bool checkHour(const QDateTime &dateTime);
    bool checkMinute(const QDateTime &dateTime);

    bool checkTimeTypes(const QDateTime &dateTime);

signals:
    void alarm();

public slots:
    void validate(const QDateTime &dateTime);
    void validateTimes(const QDateTime &dateTime);

};

#endif // ALARM_H
