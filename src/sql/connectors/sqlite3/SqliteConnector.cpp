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
#include "SqliteConnector.h"

namespace metacpp
{
namespace sql
{
namespace connectors
{
namespace sqlite
{

SqliteConnector::SqliteConnector(const String &databaseName, int poolSize)
    : m_databaseName(databaseName), m_poolSize(poolSize), m_connected(false)
{
    if (m_poolSize <= 0)
        throw std::invalid_argument("Negative pool size");
    if (m_poolSize > 10)
        throw std::invalid_argument("Pool size is too large");
}

SqliteConnector::~SqliteConnector()
{
    if (m_connected)
        disconnect();
}

bool SqliteConnector::connect()
{

    if (m_connected)
    {
        std::cerr << "SqliteConnector::connect(): database connection seems to be already opened"
                  << std::endl;
        return true;
    }
    m_freeDbHandles.reserve(m_poolSize);
    for (int i = 0; i < m_poolSize; ++i)
    {
        sqlite3 *dbHandle;
        int error = sqlite3_open_v2(m_databaseName.c_str(), &dbHandle,
                                    SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, nullptr);
        if (SQLITE_OK != error)
        {
            std::cerr << "sqlite3_open_v2(): " << describeSqliteError(error) << std::endl;
            disconnect();
            return false;
        }
        m_freeDbHandles.push_back(dbHandle);
    }
    return m_connected = true;
}

bool SqliteConnector::disconnect()
{
    if (!m_connected)
    {
        std::cerr << "SqliteConnector::disconnect(): database connection was not previously successfully created" << std::endl;
        return true;
    }

    {
        std::lock_guard<std::mutex> _guard(m_transactionMutex);
        if (m_transactions.size())
        {
            std::cerr << "SqliteConnector::disconnect(): there is still non-closed transaction connections left" << std::endl;
            return false;
        }
    }

    {
        std::lock_guard<std::mutex> _guard(m_poolMutex);
        assert(m_usedDbHandles.empty());
        for (size_t i = 0; i < m_freeDbHandles.size(); ++i)
        {
            int error = sqlite3_close(m_freeDbHandles[i]);
            if (SQLITE_OK != error)
                std::cerr << "sqlite3_close(): " << describeSqliteError(error) << std::endl;
        }
        m_freeDbHandles.clear();

    }
    return true;
}

SqlTransactionImpl *SqliteConnector::createTransaction()
{
    sqlite3 *dbHandle = nullptr;
    {
        std::unique_lock<std::mutex> _guard(m_poolMutex);
        // check if there's already any free
        if (m_freeDbHandles.size())
        {
            dbHandle = m_freeDbHandles.back();
            m_freeDbHandles.pop_back();
        }
        else
        {
            // predicate against spurious wakes
            m_dbHandleFreedEvent.wait(_guard, [this](){ return !m_freeDbHandles.empty(); });
            dbHandle = m_freeDbHandles.back();
            m_freeDbHandles.pop_back();
        }
        m_usedDbHandles.push_back(dbHandle);
    }

    SqliteTransactionImpl *result = new SqliteTransactionImpl(dbHandle);
    {
        std::lock_guard<std::mutex> _guard(m_transactionMutex);
        m_transactions.push_back(result);
    }
    return result;
}

bool SqliteConnector::closeTransaction(SqlTransactionImpl *transaction)
{
    sqlite3 *dbHandle = nullptr;
    {
        std::lock_guard<std::mutex> _guard(m_transactionMutex);
        SqliteTransactionImpl *sqliteTransaction = reinterpret_cast<SqliteTransactionImpl *>(transaction);
        dbHandle = sqliteTransaction->dbHandle();
        auto it = std::find(m_transactions.begin(), m_transactions.end(), sqliteTransaction);
        if (it == m_transactions.end())
            return false;
        m_transactions.erase(it);
        delete transaction;
    }

    {
        std::lock_guard<std::mutex> _guard(m_poolMutex);
        auto it = std::find(m_usedDbHandles.begin(), m_usedDbHandles.end(), dbHandle);
        if (it == m_usedDbHandles.end())
            throw std::runtime_error("SqliteConnector: No such used dbHandle in connection pool");
        m_usedDbHandles.erase(it);
        m_freeDbHandles.push_back(dbHandle);
        m_dbHandleFreedEvent.notify_all();
    }
    return true;
}

SqlSyntax SqliteConnector::sqlSyntax() const
{
    return SqlSyntaxSqlite;
}

const char *describeSqliteError(int errorCode)
{
    switch (errorCode)
    {
    case SQLITE_OK:
        return "Successful result";
    case SQLITE_ERROR:
        return "SQL error or missing database";
    case SQLITE_INTERNAL:
        return "Internal logic error in SQLite";
    case SQLITE_PERM:
        return "Access permission denied";
    case SQLITE_ABORT:
        return "Callback routine requested an abort";
    case SQLITE_BUSY:
        return "The database file is locked";
    case SQLITE_LOCKED:
        return "A table in the database is locked";
    case SQLITE_NOMEM:
        return "A malloc() failed";
    case SQLITE_READONLY:
        return "Attempt to write a readonly database";
    case SQLITE_INTERRUPT:
        return "Operation terminated by sqlite3_interrupt()";
    case SQLITE_IOERR:
        return "Some kind of disk I/O error occurred";
    case SQLITE_CORRUPT:
        return "The database disk image is malformed";
    case SQLITE_NOTFOUND:
        return "Unknown opcode in sqlite3_file_control()";
    case SQLITE_FULL:
        return "Insertion failed because database is full";
    case SQLITE_CANTOPEN:
        return "Unable to open the database file";
    case SQLITE_PROTOCOL:
        return "Database lock protocol error";
    case SQLITE_EMPTY:
        return "Database is empty";
    case SQLITE_SCHEMA:
        return "The database schema changed";
    case SQLITE_TOOBIG:
        return "String or BLOB exceeds size limit";
    case SQLITE_CONSTRAINT:
        return "Abort due to constraint violation";
    case SQLITE_MISMATCH:
        return "Data type mismatch";
    case SQLITE_MISUSE:
        return "Library used incorrectly";
    case SQLITE_NOLFS:
        return "Uses OS features not supported on host";
    case SQLITE_AUTH:
        return "Authorization denied";
    case SQLITE_FORMAT:
        return "Auxiliary database format error";
    case SQLITE_RANGE:
        return "2nd parameter to sqlite3_bind out of range";
    case SQLITE_NOTADB:
        return "File opened that is not a database file";
        /*
    case SQLITE_NOTICE:
        return "Notifications from sqlite3_log()";
    case SQLITE_WARNING:
        return "Warnings from sqlite3_log()";
        */
    case SQLITE_ROW:
        return "sqlite3_step() has another row ready";
    case SQLITE_DONE:
        return "sqlite3_step() has finished executing";
    default:
        return "Unknown error";
    }
}

} // namespace sqlite
} // namespace metacpp
} // namespace sql
} // namespace connectors
