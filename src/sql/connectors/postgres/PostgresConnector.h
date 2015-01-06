#ifndef POSTGRESCONNECTOR_H
#define POSTGRESCONNECTOR_H
#include "SqlConnectorBase.h"
#include <libpq-fe.h>
#include <pg_config.h>
#include <mutex>
#include <condition_variable>
#include "Array.h"
#include "PostgresTransactionImpl.h"

namespace metacpp {
namespace sql {
namespace connectors {
namespace postgres {

class PostgresConnector : public SqlConnectorBase
{
public:
    PostgresConnector(const char *connectionString, int poolSize = 3);
    ~PostgresConnector();

    bool connect() override;
    bool disconnect() override;
    SqlTransactionImpl *createTransaction() override;
    bool closeTransaction(SqlTransactionImpl *transaction) override;
    SqlSyntax sqlSyntax() const override;
private:
    String m_connectionString;
    const int m_poolSize;
    Array<PGconn *> m_freeDbHandles, m_usedDbHandles;
    std::mutex m_poolMutex;
    std::condition_variable m_dbHandleFreedEvent;
    bool m_connected;
    Array<PostgresTransactionImpl *> m_transactions;
    std::mutex m_transactionMutex;
};

} // namespace postgres
} // namespace connectors
} // namespace sql
} // namespace metacpp

#endif // POSTGRESCONNECTOR_H
