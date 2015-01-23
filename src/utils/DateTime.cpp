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

namespace metacpp {

// standard time lib isn't thread safe
static std::mutex g_stdTimeMutex;


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

int DateTimeData::month() const
{
    return m_tm.tm_mon + 1;
}

int DateTimeData::day() const
{
    return m_tm.tm_mday;
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

String DateTimeData::toString() const
{
    char buf[50];
    //strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &m_tm);
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", year(), month(), day(), hours(), minutes(), seconds());
    return buf;
}

void DateTimeData::fromString(const char *isoString)
{
    if (NULL == strptime(isoString, "%Y-%m-%d %H:%M:%S", &m_tm))
        throw std::invalid_argument(String(String(isoString) + " is not a datetime in ISO format").c_str());
}

SharedDataBase *DateTimeData::clone() const
{
    return new DateTimeData(m_tm);
}

DateTime::DateTime(time_t stdTime)
    : SharedDataPointer<DateTimeData>(new DateTimeData(stdTime))
{

}

DateTime::DateTime()
{
}

DateTime::~DateTime()
{
}

bool DateTime::valid() const
{
    return m_d;
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

int DateTime::month() const
{
    return getData()->month();
}

int DateTime::day() const
{
    return getData()->day();
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

DateTimeData *DateTime::getData() const
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

DateTime DateTime::fromString(const char *isoString)
{
    DateTime res;
    res.m_d = new DateTimeData();
    res.m_d->fromString(isoString);
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

