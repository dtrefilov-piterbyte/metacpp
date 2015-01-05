#ifndef METACPP_DATETIME_H
#define METACPP_DATETIME_H
#include <time.h>
#include "String.h"
#include "SharedDataPointer.h"

namespace metacpp {


class DateTimeData : public SharedDataBase
{
public:
    DateTimeData() { }
    explicit DateTimeData(time_t stdTime);
    explicit DateTimeData(const struct tm& tm);
    ~DateTimeData();

    bool operator==(const DateTimeData& rhs) const;
    bool operator!=(const DateTimeData& rhs) const;

    int year() const;
    int month() const;
    int day() const;
    int hours() const;
    int minutes() const;
    int seconds() const;

    time_t toStdTime() const;
    String toISOString() const;

    SharedDataBase *clone() const override;

    void fromISOString(const char *isoString);

private:
    struct tm m_tm;
};

class DateTime : protected SharedDataPointer<DateTimeData>
{
public:
    explicit DateTime(time_t stdTime);
    DateTime();
    ~DateTime();

    bool valid() const;

    bool operator==(const DateTime& rhs) const;
    bool operator!=(const DateTime& rhs) const;

    int year() const;
    int month() const;
    int day() const;
    int hours() const;
    int minutes() const;
    int seconds() const;

    time_t toStdTime() const;
    String toISOString() const;

    static DateTime fromISOString(const char *isoString);
    static DateTime now();
private:
    DateTimeData *getData() const;
};

} // namespace metacpp

#endif // METACPP_DATETIME_H
