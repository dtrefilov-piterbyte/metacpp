#include "SqlResultSet.h"
#include "SqlTransaction.h"

namespace metacpp
{
namespace sql
{

SqlResultSet::SqlResultSet(SqlTransaction &transaction,
                           connectors::SqlStatementImpl *stmt,
                           SqlStorable *storable)
    : m_transaction(transaction), m_stmt(stmt), m_storable(storable),
      m_iterator(this, ROW_ID_INVALID), m_endIterator(this, ROW_ID_PAST_THE_END)
{
}

SqlResultSet::~SqlResultSet()
{
    m_transaction.impl()->closeStatement(m_stmt);
}

SqlResultIterator SqlResultSet::begin()
{
    if (m_iterator.rowId() != ROW_ID_INVALID)
        throw std::runtime_error("SqlResultSet::begin() already been called");
    // try fetch first row
    if (m_transaction.impl()->fetchNext(m_stmt, m_storable))
        m_iterator.setRowId(0);
    else
        m_iterator.setRowId(ROW_ID_PAST_THE_END);
    return m_iterator;
}

SqlResultIterator SqlResultSet::end()
{
    return m_endIterator;
}

} // namespace sql
} // namespace metacpp
