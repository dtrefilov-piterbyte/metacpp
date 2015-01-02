#ifndef SQLRESULTSET_H
#define SQLRESULTSET_H
#include "SqlResultIterator.h"

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

class SqlResultSet
{
public:
    /** Construct result set from select statement
     * Passes ownership of stmt to the constructed SqlResultSet
    */
    SqlResultSet(SqlTransaction& transaction,
                 connectors::SqlStatementImpl *stmt,
                 SqlStorable *storable);
    virtual ~SqlResultSet();
    SqlResultIterator begin();
    SqlResultIterator end();
private:
    friend class SqlResultIterator;

    SqlTransaction& m_transaction;
    connectors::SqlStatementImpl *m_stmt;
    SqlStorable *m_storable;
    SqlResultIterator m_endIterator;
    SqlResultIterator m_iterator;
};

} // namespace sql
} // namespace metacpp

#endif // SQLRESULTSET_H
