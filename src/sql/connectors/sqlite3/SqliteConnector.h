#ifndef SQLITECONNECTOR_H
#define SQLITECONNECTOR_H
#include "SqlConnectorBase.h"
#include "SqliteTransactionImpl.h"
#include "Array.h"
#include <sqlite3.h>
#include <mutex>
#include <condition_variable>

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
    SqliteConnector(const String& databaseName = ":memory:", int poolSize = 3);
    ~SqliteConnector();

    // test database connectivity, initialize connection pool
    bool connect() override;
    bool disconnect() override;
    SqlTransactionImpl *createTransaction() override;
    bool closeTransaction(SqlTransactionImpl *transaction) override;
    SqlSyntax sqlSyntax() const override;
private:
    String m_databaseName;
    const int m_poolSize;
    Array<sqlite3 *> m_freeDbHandles, m_usedDbHandles;
    std::mutex m_poolMutex;
    std::condition_variable m_dbHandleFreedEvent;
    bool m_connected;
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
