#ifndef SQLITECONNECTOR_H
#define SQLITECONNECTOR_H
#include "SqlConnectorBase.h"
#include "SqliteTransactionImpl.h"
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

class SqliteConnector : public SqlConnectorBase
{
public:
    SqliteConnector(const String& databaseName = ":memory:");
    ~SqliteConnector();

    // test database connectivity, initialize connection pool
    bool connect() override;
    bool disconnect() override;
    SqlTransactionImpl *createTransaction() override;
    bool closeTransaction(SqlTransactionImpl *transaction) override;
    SqlSyntax sqlSyntax() const override;
private:
    String m_databaseName;
    sqlite3 *m_dbHandle; // main database handle
    Array<SqliteTransactionImpl *> m_transactions;
    std::mutex m_transactionMutex;
};

/** \brief Get human-readable description of a sqlite3 error */
const char *describeSqliteError(int errorCode);

} // namespace sqlite
} // namespace connectors
} // namespace sql
} // namespace metacpp

#endif // SQLITECONNECTOR_H
