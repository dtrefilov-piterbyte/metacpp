#include "SqliteTransactionImpl.h"
#include "CDebug.h"
#include "SqliteConnector.h"

namespace metacpp
{
namespace sql
{
namespace connectors
{
namespace sqlite
{

SqliteTransactionImpl::SqliteTransactionImpl(sqlite3 *dbHandle)
    : m_dbHandle(dbHandle)
{

}

SqliteTransactionImpl::~SqliteTransactionImpl()
{
    int error;
    if (SQLITE_OK != (error = sqlite3_close(m_dbHandle)))
        cerror() << "sqlite3_close(): " << describeSqliteError(error);
}

SqlStatementBase *SqliteTransactionImpl::createStatement(SqlStatementType type, const String& queryText)
{
    std::lock_guard<std::mutex> _guard(m_statementsMutex);
    SqliteStatement *statement = new SqliteStatement(type, queryText);
    m_statements.push_back(statement);
    return statement;
}

bool SqliteTransactionImpl::prepare(SqlStatementBase *statement)
{
    const String& query = statement->queryText();
    sqlite3_stmt *stmt;
    int error = sqlite3_prepare_v2(m_dbHandle, query.c_str(), query.size() + 1,
        &stmt, nullptr);
    if (SQLITE_OK != error)
    {
        cerror() << "sqlite3_prepare_v2(): " << describeSqliteError(error);
        return false;
    }
    reinterpret_cast<SqliteStatement *>(statement)->setHandle(stmt);
    return true;
}

bool SqliteTransactionImpl::bindArguments(SqlStatementBase *statement, SqlStorable *storable)
{
    if (statement->type() == eSqlStatementTypeUpdate || statement->type() == eSqlStatementTypeUpdate)
    {
        // TODO:
    }
}

bool SqliteTransactionImpl::execStatement(SqlStatementBase *statement)
{
    if (!statement->prepared())
    {
        cerror() << "SqliteTransactionImpl::execStatement(): should be prepared first";
        return false;
    }
    int error = sqlite3_step(reinterpret_cast<SqliteStatement *>(statement)->handle());
    if (SQLITE_DONE == error)
    {
        statement->setDone();
        return true;
    }
    else if (SQLITE_ROW == error)
    {
        return true;
    }
    else
    {
        cerror() << "sqlite3_stop(): " << describeSqliteError(error);
        return false;
    }
}

bool SqliteTransactionImpl::fetchNext(SqlStatementBase *statement, SqlStorable *storable)
{
    if (!statement->prepared())
    {
        cerror() << "SqliteTransactionImpl::execStatement(): should be prepared first";
        return false;
    }
    // no more rows
    if (statement->done())
        return false;

    int error = sqlite3_step(reinterpret_cast<SqliteStatement *>(statement)->handle());
    if (SQLITE_DONE == error)
    {
        statement->setDone();
        return false;
    }
    if (SQLITE_ROW == error)
    {
        // TODO:
        return true;
    }
    else
    {
        cerror() << "sqlite3_stop(): " << describeSqliteError(error);
        return false;
    }
}

bool SqliteTransactionImpl::closeStatement(SqlStatementBase *statement)
{
    std::lock_guard<std::mutex> _guard(m_statementsMutex);
    SqliteStatement *sqliteStatement = reinterpret_cast<SqliteStatement *>(statement);
    auto it = std::find(m_statements.begin(), m_statements.end(), sqliteStatement);
    if (it == m_statements.end())
    {
        cerror() << "SqliteTransactionImpl::closeStatement(): there's no such statement";
        return false;
    }
    m_statements.erase(it);
    delete sqliteStatement;
    return true;
}

} // namespace sqlite
} // namespace connectors
} // namespace sql
} // namespace metacpp
