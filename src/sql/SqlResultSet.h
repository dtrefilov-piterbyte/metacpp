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
#ifndef SQLRESULTSET_H
#define SQLRESULTSET_H
#include "SqlResultIterator.h"
#include "SharedDataBase.h"

namespace metacpp
{
namespace sql
{

class SqlTransaction;
class SqlStorable;

namespace connectors
{
    class SqlStatementImpl;
}

namespace detail
{

    class SqlResultSetData : public SharedDataBase
    {
    public:
        SqlResultSetData(SqlTransaction& transaction,
                     std::shared_ptr<connectors::SqlStatementImpl> statement,
                     SqlStorable *storable);
        virtual ~SqlResultSetData();
        SqlResultIterator begin();
        SqlResultIterator end();
        SharedDataBase *clone() const override;
        bool moveIterator();
    private:
        friend class SqlResultIterator;

        SqlTransaction& m_transaction;
        std::shared_ptr<connectors::SqlStatementImpl> m_statement;
        SqlStorable *m_storable;
        mutable SqlResultIterator m_endIterator;
        mutable SqlResultIterator m_iterator;
    };

} // namespace detail

class SqlResultSet : private SharedDataPointer<detail::SqlResultSetData>
{
public:
    /** Construct result set from select statement
     * Passes ownership of stmt to the constructed SqlResultSet
    */
    SqlResultSet(SqlTransaction& transaction,
                 std::shared_ptr<connectors::SqlStatementImpl> statement,
                 SqlStorable *storable);
    virtual ~SqlResultSet();
    SqlResultIterator begin();
    SqlResultIterator end();
};

} // namespace sql
} // namespace metacpp

#endif // SQLRESULTSET_H
