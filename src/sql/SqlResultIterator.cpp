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
#include "SqlResultIterator.h"
#include "SqlResultSet.h"
#include "SqlTransaction.h"

namespace metacpp
{
namespace sql
{

SqlResultIterator::SqlResultIterator(detail::SqlResultSetData *resultSet, int rowId)
    : m_resultSet(resultSet), m_rowId(rowId)
{
}

SqlResultIterator::SqlResultIterator(const SqlResultIterator &other)
{
    *this = other;
}

SqlResultIterator::~SqlResultIterator()
{
}

SqlResultIterator &SqlResultIterator::operator=(const SqlResultIterator &rhs)
{
    m_resultSet = rhs.m_resultSet;
    m_rowId = rhs.m_rowId;
    return *this;
}

bool SqlResultIterator::operator ==(const SqlResultIterator& rhs)
{
    return m_resultSet == rhs.m_resultSet && m_rowId == rhs.m_rowId;
}

bool SqlResultIterator::operator !=(const SqlResultIterator& rhs)
{
    return !(*this == rhs);
}

int SqlResultIterator::operator*() const
{
    return m_rowId;
}

int SqlResultIterator::operator->() const
{
    return m_rowId;
}

SqlResultIterator& SqlResultIterator::operator ++()
{
    if (m_resultSet->moveIterator())
        m_rowId++;
    else
        m_rowId = ROW_ID_PAST_THE_END;
    return *this;
}

} // namespace sql
} // namespace metacpp
