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
#ifndef MYSQLCONNECTOR_H
#define MYSQLCONNECTOR_H
#include "SqlConnectorBase.h"
#include <mutex>
#include <condition_variable>
#include "Array.h"
#include "Uri.h"
#include "MySqlTransactionImpl.h"
#include <mysql.h>

namespace metacpp {
namespace db {
namespace sql {
namespace connectors {
namespace mysql {

class MySqlConnector : public SqlConnectorBase
{
public:
    MySqlConnector(const Uri& connectionUri);
    ~MySqlConnector();

    bool connect() override;
    bool disconnect() override;
    SqlTransactionImpl *createTransaction() override;
    bool closeTransaction(SqlTransactionImpl *transaction) override;
    SqlSyntax sqlSyntax() const override;
    void setConnectionPooling(size_t size);
private:
    Uri m_connectionUri;
    size_t m_poolSize;
    Array<MYSQL *> m_freeDbHandles, m_usedDbHandles;
    std::mutex m_poolMutex;
    std::condition_variable m_dbHandleFreedEvent;
    bool m_connected;
    Array<MySqlTransactionImpl *> m_transactions;
    std::mutex m_transactionMutex;
};

class MySqlConnectorFactory : public SqlConnectorFactory
{
    std::unique_ptr<SqlConnectorBase> createInstance(const Uri &uri);
};

} // namespace mysql
} // namespace connectors
} // namespace sql
} // namespace db
} // namespace metacpp

#endif // POSTGRESCONNECTOR_H
