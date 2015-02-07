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
    SqliteConnector(const String& databaseName, int poolSize = 3);
    ~SqliteConnector();

    // test database connectivity, initialize connection pool
    bool connect() override;
    bool disconnect() override;
    SqlTransactionImpl *createTransaction() override;
    bool closeTransaction(SqlTransactionImpl *transaction) override;
    SqlSyntax sqlSyntax() const override;
    EConnectorType connectorType() const override;
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
