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

    bool begin() override;
    bool commit() override;
    bool rollback() override;

    SqlStatementImpl *createStatement(SqlStatementType type, const String& queryText) override;
    bool prepare(SqlStatementImpl *statement) override;
    bool execStatement(SqlStatementImpl *statement, int *numRowsAffected = nullptr) override;
    bool fetchNext(SqlStatementImpl *statement, SqlStorable *storable) override;
    bool getLastInsertId(SqlStatementImpl *statement, SqlStorable *storable) override;
    bool closeStatement(SqlStatementImpl *statement) override;

    sqlite3 *dbHandle() const { return m_dbHandle; }
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
