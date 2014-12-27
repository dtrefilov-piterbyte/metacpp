#ifndef SQLITETRANSACTIONIMPL_H
#define SQLITETRANSACTIONIMPL_H
#include "SqlTransactionImpl.h"
#include "SqliteStatement.h"
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

    SqlStatementBase *createStatement(SqlStatementType type, const String& queryText) override;
    bool prepare(SqlStatementBase *statement) override;
    bool bindArguments(SqlStatementBase *statement, SqlStorable *storable) override;
    bool execStatement(SqlStatementBase *statement) override;
    bool fetchNext(SqlStatementBase *statement, SqlStorable *storable) override;
    bool closeStatement(SqlStatementBase *statement) override;
private:
    sqlite3 *m_dbHandle;
    Array<SqliteStatement *> m_statements;
    std::mutex m_statementsMutex;
};

} // namespace sqlite
} // namespace connectors
} // namespace sql
} // namespace metacpp

#endif // SQLITETRANSACTIONIMPL_H
