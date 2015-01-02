#include "DateTime.h"
#include <mutex>

namespace metacpp {

// standard time lib isn't thread safe
static std::mutex g_stdTimeMutex;


DateTime::DateTime(time_t stdTime)
{
    std::lock_guard<std::mutex> _guard(g_stdTimeMutex);
    m_tm = *localtime(&stdTime);
}

DateTime::DateTime(const DateTime &o)
{
    *this = o;
}

DateTime::~DateTime()
{

}

DateTime &DateTime::operator=(const DateTime &rhs)
{
    m_tm = rhs.m_tm;
    return *this;
}

bool DateTime::operator ==(const DateTime& rhs) const
{
    return  m_tm.tm_year == rhs.m_tm.tm_year &&
            m_tm.tm_mon == rhs.m_tm.tm_mon &&
            m_tm.tm_mday == rhs.m_tm.tm_mday &&
            m_tm.tm_hour == rhs.m_tm.tm_hour &&
            m_tm.tm_min == rhs.m_tm.tm_min &&
            m_tm.tm_sec == rhs.m_tm.tm_sec &&
            m_tm.tm_gmtoff == rhs.m_tm.tm_gmtoff;
}

bool DateTime::operator !=(const DateTime& rhs) const
{
    return !(*this == rhs);
}

int DateTime::year() const
{
    return m_tm.tm_year + 1900;
}

int DateTime::month() const
{
    return m_tm.tm_mon + 1;
}

int DateTime::day() const
{
    return m_tm.tm_mday;
}

int DateTime::hours() const
{
    return m_tm.tm_hour;
}

int DateTime::minutes() const
{
    return m_tm.tm_min;
}

int DateTime::seconds() const
{
    return m_tm.tm_sec;
}

time_t DateTime::toStdTime() const
{
    return mktime(const_cast<struct tm *>(&m_tm));
}

String DateTime::toISOString() const
{
    char buf[50];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &m_tm);
    return buf;
}

DateTime DateTime::fromISOString(const char *isoString)
{
    DateTime res(0);
    strptime(isoString, "%Y-%m-%d %H:%M:%S", &res.m_tm);
    return res;
}

} // namespace metacpp

