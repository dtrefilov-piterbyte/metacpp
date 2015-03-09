/****************************************************************************
* Copyright 2014-2015 Trefilov Dmitrij                                      *
*                                                                           *
* Licensed under the Apache License, Version 2.0 (the "License");           *
* you may not use this file except in compliance with the License.          *
* You may obtain a copy of the License at                                   *
*                                                                           *
*    http://www.apache.org/licenses/LICENSE-2.0                             *
*                                                                           *
* Unless required by applicable law or agreed to in writing, software       *
* distributed under the License is distributed on an "AS IS" BASIS,         *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
* See the License for the specific language governing permissions and       *
* limitations under the License.                                            *
****************************************************************************/
#include "DateTime.h"
#include <mutex>
#include <stdio.h>
#include <locale>
#include <ctime>
#include <iomanip>

namespace metacpp {

// standard time lib isn't thread safe
static std::mutex g_stdTimeMutex;

namespace detail
{
    DateTimeData::DateTimeData(time_t stdTime)
    {
        std::lock_guard<std::mutex> _guard(g_stdTimeMutex);
        m_tm = *localtime(&stdTime);
    }

    DateTimeData::DateTimeData(const tm &tm)
        : m_tm(tm)
    {

    }

    DateTimeData::~DateTimeData()
    {

    }

    bool DateTimeData::operator ==(const DateTimeData& rhs) const
    {
        return  m_tm.tm_year == rhs.m_tm.tm_year &&
                m_tm.tm_mon == rhs.m_tm.tm_mon &&
                m_tm.tm_mday == rhs.m_tm.tm_mday &&
                m_tm.tm_hour == rhs.m_tm.tm_hour &&
                m_tm.tm_min == rhs.m_tm.tm_min &&
                m_tm.tm_sec == rhs.m_tm.tm_sec;
    }

    bool DateTimeData::operator !=(const DateTimeData& rhs) const
    {
        return !(*this == rhs);
    }

    int DateTimeData::year() const
    {
        return m_tm.tm_year + 1900;
    }

    EMonth DateTimeData::month() const
    {
        switch (m_tm.tm_mon)
        {
        case 0:  return January;
        case 1:  return February;
        case 2:  return March;
        case 3:  return April;
        case 4:  return May;
        case 5:  return June;
        case 6:  return July;
        case 7:  return August;
        case 8:  return September;
        case 9:  return October;
        case 10: return Novermber;
        case 11: return December;
        default: throw std::invalid_argument("Invalid tm_mon");
        }

        return static_cast<EMonth>(m_tm.tm_mon);
    }

    int DateTimeData::day() const
    {
        return m_tm.tm_mday;
    }

    EDayOfWeek DateTimeData::dayOfWeek() const
    {
        switch (m_tm.tm_wday)
        {
        case 0:  return Sunday;
        case 1:  return Monday;
        case 2:  return Tuesday;
        case 3:  return Wednesday;
        case 4:  return Thursday;
        case 5:  return Friday;
        case 6:  return Sataday;
        default: throw std::invalid_argument("Invalid tm_wday");
        }
    }

    int DateTimeData::hours() const
    {
        return m_tm.tm_hour;
    }

    int DateTimeData::minutes() const
    {
        return m_tm.tm_min;
    }

    int DateTimeData::seconds() const
    {
        return m_tm.tm_sec;
    }

    time_t DateTimeData::toStdTime() const
    {
        return mktime(const_cast<struct tm *>(&m_tm));
    }

    tm DateTimeData::toTm() const
    {
        return m_tm;
    }

    String DateTimeData::toString() const
    {
        char buf[50];
        //strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &m_tm);
        sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", year(), static_cast<int>(month()) + 1, day(), hours(), minutes(), seconds());
        return buf;
    }

    String DateTimeData::toString(const char *format) const
    {
        for (size_t bufSize = 50; ; bufSize += (size_t)(0.3 * bufSize))
        {
            char *buf = reinterpret_cast<char *>(alloca(bufSize));
            size_t size = strftime(buf, bufSize, format, &m_tm);
            if (size) return String(buf, size - 1);
        }
    }

    void DateTimeData::fromString(const char *isoString)
    {
		return fromString(isoString, "%Y-%m-%d %H:%M:%S");
    }

    void DateTimeData::fromString(const char *str, const char *format)
    {
#ifdef _MSC_VER	  
		StringStream ss(str);
		try
        {
			ss.exceptions(std::ios_base::failbit | std::ios_base::badbit);
			ss >> std::get_time(&m_tm, format);	 // TODO: seems to be buggy in MSVC 2013
        }
        catch (const std::exception& ex)
        {
            throw std::invalid_argument("Invalid DateTime string");
		}
		ss.exceptions(std::ios_base::goodbit);
		char ch; ss.get();
		if (!ss.eof())
			throw std::invalid_argument("Found extra characters at end of DateTime string");
#else
        const char *res = strptime(str, format, &m_tm);
        if (NULL == res || *res)
            throw std::invalid_argument(String(String(str) + " is not a datetime in specified format").c_str());
#endif
    }

    SharedDataBase *DateTimeData::clone() const
    {
        return new DateTimeData(m_tm);
    }


} // namespace detail

DateTime::DateTime(detail::DateTimeData *d)
    : SharedDataPointer(d)
{
}

DateTime::DateTime(time_t stdTime)
    : SharedDataPointer(new detail::DateTimeData(stdTime))
{

}

DateTime::DateTime(int y, EMonth mo, int d, int h, int m, int s)
{
    char buf[50];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", y, static_cast<int>(mo) + 1, d, h, m, s);
    *this = fromString(buf);
}

DateTime::DateTime()
{
}

DateTime::~DateTime()
{
}

bool DateTime::valid() const
{
    return m_d != nullptr;
}

bool DateTime::operator==(const DateTime& rhs) const
{
    if (!valid() || !rhs.valid())
        return valid() == rhs.valid();
    return *m_d == *rhs.m_d;
}

bool DateTime::operator!=(const DateTime& rhs) const
{
    return !(*this == rhs);
}

int DateTime::year() const
{
    return getData()->year();
}

EMonth DateTime::month() const
{
    return getData()->month();
}

int DateTime::day() const
{
    return getData()->day();
}

EDayOfWeek DateTime::dayOfWeek() const
{
    return getData()->dayOfWeek();
}

int DateTime::hours() const
{
    return getData()->hours();
}

int DateTime::minutes() const
{
    return getData()->minutes();
}

int DateTime::seconds() const
{
    return getData()->seconds();
}

DateTime &DateTime::addYears(int years)
{
    char buf[50];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            getData()->year() + years,
            static_cast<int>(getData()->month()) + 1,
            getData()->day(),
            getData()->hours(),
            getData()->minutes(),
            getData()->seconds());

    return *this = fromString(buf);
}

DateTime &DateTime::addMonths(int months)
{
    char buf[50];
    int m = static_cast<int>(getData()->month()) + months;
    int y = getData()->year();
    if (m < 0)
    {
        y -= m / 12 + 1;
        m = 12 + (m % 12);
    }
    else
    {
        y += m / 12;
        m %= 12;
    }
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            y,
            m + 1,
            getData()->day(),
            getData()->hours(),
            getData()->minutes(),
            getData()->seconds());

    return *this = fromString(buf);
}

DateTime &DateTime::addDays(int days)
{
    return addSeconds(days * 24 * 60 * 60);
}

DateTime &DateTime::addHours(int hours)
{
    return addSeconds(hours * 60 * 60);
}

DateTime &DateTime::addMinutes(int minutes)
{
    return addSeconds(minutes * 60);
}

DateTime &DateTime::addSeconds(int seconds)
{
    time_t time = getData()->toStdTime();
    time += seconds;
    this->m_d->deref();
    this->m_d = new detail::DateTimeData(time);
    return *this;
}

DateTime &DateTime::setYear(int year)
{
    char buf[50];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            year,
            static_cast<int>(getData()->month()) + 1,
            getData()->day(),
            getData()->hours(),
            getData()->minutes(),
            getData()->seconds());

    return *this = fromString(buf);
}

DateTime &DateTime::setMonth(EMonth month)
{
    char buf[50];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            getData()->year(),
            static_cast<int>(month) + 1,
            getData()->day(),
            getData()->hours(),
            getData()->minutes(),
            getData()->seconds());

    return *this = fromString(buf);
}

DateTime &DateTime::setDay(int day)
{
    char buf[50];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            getData()->year(),
            static_cast<int>(getData()->month()) + 1,
            day,
            getData()->hours(),
            getData()->minutes(),
            getData()->seconds());

    return *this = fromString(buf);
}

DateTime &DateTime::setYMD(int year, EMonth month, int day)
{
    if (day < 1 || day > 31)
        throw std::invalid_argument("Incorrect day of month");
    int h = 0, m = 0, s = 0;
    if (m_d)
    {
        h = m_d->hours();
        m = m_d->minutes();
        s = m_d->seconds();
    }
    char buf[50];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", year, static_cast<int>(month) + 1, day, h, m, s);
    return *this = fromString(buf);
}

DateTime &DateTime::setHours(int hours)
{
    if (hours < 0 || hours > 23)
        throw std::invalid_argument("Incorrect hours");
    char buf[50];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            getData()->year(),
            static_cast<int>(getData()->month()) + 1,
            getData()->day(),
            hours,
            getData()->minutes(),
            getData()->seconds());

    return *this = fromString(buf);
}

DateTime &DateTime::setMinutes(int minutes)
{
    if (minutes < 0 || minutes > 59)
        throw std::invalid_argument("Incorrect minutes");
    char buf[50];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            getData()->year(),
            static_cast<int>(getData()->month()) + 1,
            getData()->day(),
            getData()->hours(),
            minutes,
            getData()->seconds());

    return *this = fromString(buf);
}

DateTime &DateTime::setSeconds(int seconds)
{
    if (seconds < 0 || seconds > 60) // leap second
        throw std::invalid_argument("Incorrect minutes");
    char buf[50];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            getData()->year(),
            static_cast<int>(getData()->month()) + 1,
            getData()->day(),
            getData()->hours(),
            getData()->minutes(),
            seconds);

    return *this = fromString(buf);
}

DateTime &DateTime::setHMS(int hour, int minute, int second)
{
    if (hour < 0 || hour > 23)
        throw std::invalid_argument("Incorrect hours");
    if (minute < 0 || minute > 59)
        throw std::invalid_argument("Incorrect minutes");
    if (second < 0 || second > 60) // leap second
        throw std::invalid_argument("Incorrect minutes");
    int y = getData()->year(), m = static_cast<int>(getData()->month()) + 1, d = getData()->day();
    char buf[50];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", y, m, d, hour, minute, second);
    return *this = fromString(buf);

}

detail::DateTimeData *DateTime::getData() const
{
    if (!m_d)
        throw std::runtime_error("DateTime is invalid");
    return m_d;
}

time_t DateTime::toStdTime() const
{
    return getData()->toStdTime();
}

String DateTime::toString() const
{
    return getData()->toString();
}

String DateTime::toString(const char *format) const
{
    return getData()->toString(format);
}

DateTime DateTime::fromString(const char *isoString)
{
    DateTime res(new detail::DateTimeData());
    res.m_d->fromString(isoString);
    return res;
}

DateTime DateTime::fromString(const char *string, const char *format)
{
    DateTime res(new detail::DateTimeData());
    res.m_d->fromString(string, format);
    return res;
}

DateTime DateTime::now()
{
    return DateTime(time(NULL));
}

std::ostream &operator<<(std::ostream &stream, const DateTime &dt)
{
    return stream << (dt.valid() ? dt.toString() : "(null)");
}

} // namespace metacpp

