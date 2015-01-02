#include "SqliteConnector.h"
#include <CDebug.h>

namespace metacpp
{
namespace sql
{
namespace connectors
{
namespace sqlite
{

SqliteConnector::SqliteConnector(const String &databaseName)
    : m_databaseName(databaseName), m_dbHandle(nullptr)
{

}

SqliteConnector::~SqliteConnector()
{
    if (m_dbHandle)
        disconnect();
}

bool SqliteConnector::connect()
{
    if (m_dbHandle)
    {
        cwarning() << "SqliteConnector::connect(): database connection seems to be already opened";
        return true;
    }
    int error = sqlite3_open_v2(m_databaseName.c_str(), &m_dbHandle,
                             SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, nullptr);
    if (SQLITE_OK != error)
    {
        cerror() << "sqlite3_open_v2(): " << describeSqliteError(error);
        return false;
    }
    return true;
}

bool SqliteConnector::disconnect()
{
    if (!m_dbHandle)
    {
        cwarning() << "SqliteConnector::disconnect(): database connection was not previously successfully created";
        return true;
    }
    int error = sqlite3_close(m_dbHandle);
    if (SQLITE_OK != error)
    {
        cerror() << "sqlite3_close(): " << describeSqliteError(error);
        return false;
    }

    {
        std::lock_guard<std::mutex> _guard(m_transactionMutex);
        if (m_transactions.size())
            cwarning() << "SqliteConnector::disconnect(): there is still non-closed transaction connections left";
    }

    return true;
}

SqlTransactionImpl *SqliteConnector::beginTransaction()
{
    int error;
    sqlite3 *dbHandle;
    error = sqlite3_open_v2(m_databaseName.c_str(), &dbHandle,
                             SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE, nullptr);
    if (SQLITE_OK != error)
    {
        cfatal() << "sqlite3_open_v2(): " << describeSqliteError(error);
        return nullptr;
    }
    char *errorMessage;
    error = sqlite3_exec(dbHandle, "BEGIN TRANSACTION", nullptr, nullptr, &errorMessage);
    if (SQLITE_OK != error)
    {
        cfatal() << "Failed to start transaction with sqlite3_exec(): " << errorMessage;
        return nullptr;
    }
    SqliteTransactionImpl *result = new SqliteTransactionImpl(dbHandle);
    {
        std::lock_guard<std::mutex> _guard(m_transactionMutex);
        m_transactions.push_back(result);
    }
    return result;
}

bool SqliteConnector::commitTransaction(SqlTransactionImpl *transaction)
{
    return closeTransaction(transaction, "COMMIT TRANSACTION");
}

bool SqliteConnector::rollbackTransaction(SqlTransactionImpl *transaction)
{
    return closeTransaction(transaction, "ROLLBACK TRANSACTION");
}

SqlSyntax SqliteConnector::sqlSyntax() const
{
    return SqlSyntaxSqlite;
}

bool SqliteConnector::closeTransaction(SqlTransactionImpl *transaction, const char *closeStmt)
{
    SqliteTransactionImpl *sqliteTransaction = reinterpret_cast<SqliteTransactionImpl *>(transaction);
    char *errorMessage;
    int error;
    error = sqlite3_exec(sqliteTransaction->dbHandle(), closeStmt, nullptr, nullptr, &errorMessage);
    if (SQLITE_OK != error)
    {
        cfatal() << "Failed to commit transaction: " << errorMessage;
        return false;
    }
    {
        std::lock_guard<std::mutex> _guard(m_transactionMutex);
        auto it = std::find(m_transactions.begin(), m_transactions.end(), sqliteTransaction);
        if (it != m_transactions.end())
        {
            m_transactions.erase(it);
            delete transaction;
            return true;
        }
        cerror() << "SqliteConnector::commitTransaction(): no such transaction";
        return false;
    }
}

const char *describeSqliteError(int errorCode)
{
    return sqlite3_errstr(errorCode);
    //switch (errorCode)
    //{
    //case SQLITE_OK:
    //    return "Successful result";
    //case SQLITE_ERROR:
    //    return "SQL error or missing database";
    //case SQLITE_INTERNAL:
    //    return "Internal logic error in SQLite";
    //case SQLITE_PERM:
    //    return "Access permission denied";
    //case SQLITE_ABORT:
    //    return "Callback routine requested an abort";
    //case SQLITE_BUSY:
    //    return "The database file is locked";
    //case SQLITE_LOCKED:
    //    return "A table in the database is locked";
    //case SQLITE_NOMEM:
    //    return "A malloc() failed";
    //case SQLITE_READONLY:
    //    return "Attempt to write a readonly database";
    //case SQLITE_INTERRUPT:
    //    return "Operation terminated by sqlite3_interrupt()";
    //case SQLITE_IOERR:
    //    return "Some kind of disk I/O error occurred";
    //case SQLITE_CORRUPT:
    //    return "The database disk image is malformed";
    //case SQLITE_NOTFOUND:
    //    return "Unknown opcode in sqlite3_file_control()";
    //case SQLITE_FULL:
    //    return "Insertion failed because database is full";
    //case SQLITE_CANTOPEN:
    //    return "Unable to open the database file";
    //case SQLITE_PROTOCOL:
    //    return "Database lock protocol error";
    //case SQLITE_EMPTY:
    //    return "Database is empty";
    //case SQLITE_SCHEMA:
    //    return "The database schema changed";
    //case SQLITE_TOOBIG:
    //    return "String or BLOB exceeds size limit";
    //case SQLITE_CONSTRAINT:
    //    return "Abort due to constraint violation";
    //case SQLITE_MISMATCH:
    //    return "Data type mismatch";
    //case SQLITE_MISUSE:
    //    return "Library used incorrectly";
    //case SQLITE_NOLFS:
    //    return "Uses OS features not supported on host";
    //case SQLITE_AUTH:
    //    return "Authorization denied";
    //case SQLITE_FORMAT:
    //    return "Auxiliary database format error";
    //case SQLITE_RANGE:
    //    return "2nd parameter to sqlite3_bind out of range";
    //case SQLITE_NOTADB:
    //    return "File opened that is not a database file";
    //case SQLITE_NOTICE:
    //    return "Notifications from sqlite3_log()";
    //case SQLITE_WARNING:
    //    return "Warnings from sqlite3_log()";
    //case SQLITE_ROW:
    //    return "sqlite3_step() has another row ready";
    //case SQLITE_DONE:
    //    return "sqlite3_step() has finished executing";
    //default:
    //    return "Unknown error";
    //}
}

} // namespace sqlite
} // namespace metacpp
} // namespace sql
} // namespace connectors
