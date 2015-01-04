#include "SqlResultSet.h"
#include "SqlTransaction.h"

namespace metacpp
{
namespace sql
{

SqlResultSetData::SqlResultSetData(SqlTransaction& transaction,
                                   std::shared_ptr<connectors::SqlStatementImpl> statement,
                                   SqlStorable *storable)
    : m_transaction(transaction), m_statement(statement), m_storable(storable),
      m_iterator(this, ROW_ID_INVALID), m_endIterator(this, ROW_ID_PAST_THE_END)
{
}

SharedDataBase *SqlResultSetData::clone() const
{
    throw std::runtime_error("SqlResultSetData is not clonable");
}

SqlResultSetData::~SqlResultSetData()
{
}

bool SqlResultSetData::moveIterator()
{
    if (m_transaction.impl()->fetchNext(m_statement.get(), m_storable)) {
        return true;
    } else {
        return false;
    }
}

SqlResultIterator SqlResultSetData::begin()
{
    if (m_iterator.rowId() != ROW_ID_INVALID)
        throw std::runtime_error("SqlResultSet::begin() already been called");
    // try fetch first row
    if (moveIterator())
        m_iterator.setRowId(0);
    else
        m_iterator.setRowId(ROW_ID_PAST_THE_END);
    return m_iterator;
}

SqlResultIterator SqlResultSetData::end()
{
    return m_endIterator;
}

SqlResultSet::SqlResultSet(SqlTransaction &transaction, std::shared_ptr<connectors::SqlStatementImpl> statement, SqlStorable *storable)
    : SharedDataPointer<SqlResultSetData>(new SqlResultSetData(transaction, statement, storable))
{

}

SqlResultSet::~SqlResultSet()
{
}

SqlResultIterator SqlResultSet::begin()
{
    return this->data()->begin();
}

SqlResultIterator SqlResultSet::end()
{
    return this->data()->end();
}

} // namespace sql
} // namespace metacpp
