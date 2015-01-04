#ifndef METACPP_DATETIME_H
#define METACPP_DATETIME_H
#include <time.h>
#include "String.h"

namespace metacpp {

// TODO: have to use third-party lib
class DateTime
{
public:
    explicit DateTime(time_t stdTime);
    DateTime();
    DateTime(const DateTime& o);
    ~DateTime();

    DateTime& operator=(const DateTime& rhs);
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
private:
    struct tm m_tm;
};

} // namespace metacpp

#endif // METACPP_DATETIME_H
