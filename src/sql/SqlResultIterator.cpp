#include "SqlResultIterator.h"
#include "SqlResultSet.h"
#include "SqlTransaction.h"

namespace metacpp
{
namespace sql
{

SqlResultIterator::SqlResultIterator(SqlResultSetData *resultSet, int rowId)
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
