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
