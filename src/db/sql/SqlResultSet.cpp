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
#include "SqlResultSet.h"
#include "SqlTransaction.h"

namespace metacpp
{
namespace db
{
namespace sql
{

namespace detail
{

    SqlResultSetData::SqlResultSetData(SqlTransaction& transaction,
                                       std::shared_ptr<connectors::SqlStatementImpl> statement,
                                       SqlStorable *storable)
        : m_transaction(transaction), m_statement(statement), m_storable(storable),
          m_endIterator(this, ROW_ID_PAST_THE_END), m_iterator(this, ROW_ID_INVALID)
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

    size_t SqlResultSetData::size()
    {
        return m_transaction.impl()->size(m_statement.get());
    }

    SqlResultIterator SqlResultSetData::begin()
    {
        if (m_iterator.rowId() != ROW_ID_INVALID)
            throw std::runtime_error("SqlResultSet::begin() had already been called");


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

} // namespace detail

SqlResultSet::SqlResultSet(SqlTransaction &transaction, std::shared_ptr<connectors::SqlStatementImpl> statement, SqlStorable *storable)
    : SharedDataPointer(new detail::SqlResultSetData(transaction, statement, storable))
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

size_t SqlResultSet::size()
{
    return this->data()->size();
}

} // namespace sql
} // namespace db
} // namespace metacpp
