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
class SqlStatementBase;

class SqlResultSetData : public SharedDataBase
{
public:
    SqlResultSetData(SqlTransaction& transaction,
                 SqlStatementBase *statement,
                 SqlStorable *storable);
    virtual ~SqlResultSetData();
    SqlResultIterator begin();
    SqlResultIterator end();
    SharedDataBase *clone() const override;
private:
    friend class SqlResultIterator;

    SqlTransaction& m_transaction;
    SqlStatementBase *m_statement;
    SqlStorable *m_storable;
    mutable SqlResultIterator m_endIterator;
    mutable SqlResultIterator m_iterator;
};

class SqlResultSet : private SharedDataPointer<SqlResultSetData>
{
public:
    /** Construct result set from select statement
     * Passes ownership of stmt to the constructed SqlResultSet
    */
    SqlResultSet(SqlTransaction& transaction,
                 SqlStatementBase *stmt,
                 SqlStorable *storable);
    virtual ~SqlResultSet();
    SqlResultIterator begin();
    SqlResultIterator end();
};

} // namespace sql
} // namespace metacpp

#endif // SQLRESULTSET_H
