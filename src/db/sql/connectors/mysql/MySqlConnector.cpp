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
#include "MySqlConnector.h"

namespace metacpp {
namespace db {
namespace sql {
namespace connectors {
namespace mysql {

MySqlConnector::MySqlConnector(const Uri &connectionUri)
    : m_connectionUri(connectionUri), m_poolSize(1), m_connected(false)
{
}

MySqlConnector::~MySqlConnector()
{
    if (m_connected)
        disconnect();
}

bool MySqlConnector::connect()
{

    if (m_connected)
    {
        std::cerr << "MySqlConnector::connect(): database connection seems to be already opened"
                  << std::endl;
        return true;
    }

    const char *host = m_connectionUri.host().isNullOrEmpty() ? NULL : m_connectionUri.host().c_str();
    const char *user = m_connectionUri.username().isNullOrEmpty() ? NULL : m_connectionUri.username().c_str();
    const char *password = m_connectionUri.password().isNullOrEmpty() ? NULL : m_connectionUri.password().c_str();
    const char *dbName = m_connectionUri.path().isNullOrEmpty() ? NULL : m_connectionUri.path().c_str();
    unsigned int port = m_connectionUri.port().isNullOrEmpty() ? 0 : m_connectionUri.port().toValue<unsigned int>();

    m_freeDbHandles.reserve(m_poolSize);
    m_usedDbHandles.reserve(m_poolSize);
    for (size_t i = 0; i < m_poolSize; ++i)
    {
        MYSQL *mysql = mysql_init(NULL);
        MYSQL *dbConn = mysql_real_connect(mysql, host, user, password, dbName, port, NULL, 0);
        if (!dbConn)
        {
            mysql_close(mysql);
            std::cerr << "mysql_real_connect(): failed to establish connection to database. ";
            std::cerr << m_connectionUri << std::endl;
            disconnect();
            return false;
        }
        m_freeDbHandles.push_back(dbConn);
    }
    return m_connected = true;
}

bool MySqlConnector::disconnect()
{
    if (!m_connected)
    {
        std::cerr << "MySqlConnector::disconnect(): database connection was not previously successfully created" << std::endl;
        return true;
    }

    {
        std::lock_guard<std::mutex> _guard(m_transactionMutex);
        if (m_transactions.size())
        {
            std::cerr << "MySqlConnector::disconnect(): there is still non-closed transaction connections left" << std::endl;
            return false;
        }
    }

    {
        std::lock_guard<std::mutex> _guard(m_poolMutex);
        assert(m_usedDbHandles.empty());
        for (size_t i = 0; i < m_freeDbHandles.size(); ++i)
            mysql_close(m_freeDbHandles[i]);
        m_freeDbHandles.clear();

    }
    return true;
}

SqlTransactionImpl *MySqlConnector::createTransaction()
{
    MYSQL *dbConn = nullptr;
    {
        std::unique_lock<std::mutex> _guard(m_poolMutex);
        // check if there's already any free
        if (m_freeDbHandles.size())
        {
            dbConn = m_freeDbHandles.back();
            m_freeDbHandles.pop_back();
        }
        else
        {
            // predicate against spurious wakes
            m_dbHandleFreedEvent.wait(_guard, [this](){ return !m_freeDbHandles.empty(); });
            dbConn = m_freeDbHandles.back();
            m_freeDbHandles.pop_back();
        }
        m_usedDbHandles.push_back(dbConn);
    }

    MySqlTransactionImpl *result = new MySqlTransactionImpl(dbConn);
    {
        std::lock_guard<std::mutex> _guard(m_transactionMutex);
        m_transactions.push_back(result);
    }
    return result;
}

bool MySqlConnector::closeTransaction(SqlTransactionImpl *transaction)
{
    MYSQL *dbConn = nullptr;
    {
        std::lock_guard<std::mutex> _guard(m_transactionMutex);
        MySqlTransactionImpl *mysqlTransaction = reinterpret_cast<MySqlTransactionImpl *>(transaction);
        dbConn = mysqlTransaction->dbConn();
        auto it = std::find(m_transactions.begin(), m_transactions.end(), mysqlTransaction);
        if (it == m_transactions.end())
            return false;
        m_transactions.erase(it);
        delete mysqlTransaction;
    }

    {
        std::lock_guard<std::mutex> _guard(m_poolMutex);
        auto it = std::find(m_usedDbHandles.begin(), m_usedDbHandles.end(), dbConn);
        if (it == m_usedDbHandles.end())
            throw std::runtime_error("MySqlConnector: No such used dbConn in connection pool");
        m_usedDbHandles.erase(it);
        m_freeDbHandles.push_back(dbConn);
        m_dbHandleFreedEvent.notify_all();
    }
    return true;
}

SqlSyntax MySqlConnector::sqlSyntax() const
{
    return SqlSyntaxMySql;
}

void MySqlConnector::setConnectionPooling(size_t size)
{
    if (size == 0 || size > 10)
        throw std::invalid_argument("size");
    m_poolSize = size;
}

std::unique_ptr<SqlConnectorBase> MySqlConnectorFactory::createInstance(const Uri &uri)
{
    return std::unique_ptr<SqlConnectorBase>(new MySqlConnector(uri));
}

} // namespace mysql
} // namespace connectors
} // namespace sql
} // namespace db
} // namespace metacpp

