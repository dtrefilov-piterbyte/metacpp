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

class SqlResultSet : private SharedDataPointer<SqlResultSetData>
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
