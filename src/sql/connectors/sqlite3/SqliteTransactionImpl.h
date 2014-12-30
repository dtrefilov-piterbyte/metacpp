#ifndef SQLITETRANSACTIONIMPL_H
#define SQLITETRANSACTIONIMPL_H
#include "SqlTransactionImpl.h"
#include "SqliteStatementImpl.h"
#include "Array.h"
#include <sqlite3.h>
#include <mutex>

namespace metacpp
{
namespace sql
{
namespace connectors
{
namespace sqlite
{

class SqliteTransactionImpl : public SqlTransactionImpl
{
public:
    SqliteTransactionImpl(sqlite3 *dbHandle);
    ~SqliteTransactionImpl();

    SqlStatementImpl *createStatement(SqlStatementType type, const String& queryText) override;
    bool prepare(SqlStatementImpl *statement) override;
    bool execStatement(SqlStatementImpl *statement) override;
    bool fetchNext(SqlStatementImpl *statement, SqlStorable *storable) override;
    bool closeStatement(SqlStatementImpl *statement) override;
private:
    sqlite3 *m_dbHandle;
    Array<SqliteStatementImpl *> m_statements;
    std::mutex m_statementsMutex;
};

} // namespace sqlite
} // namespace connectors
} // namespace sql
} // namespace metacpp

#endif // SQLITETRANSACTIONIMPL_H
