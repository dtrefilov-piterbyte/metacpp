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
    {
        std::lock_guard<std::mutex> _guard(m_statementsMutex);
        if (m_statements.size())
        {
            cwarning() << "There's still " << m_statements.size() <<
                          " unclosed statements while destroing the sqlite transaction";
        }
    }
    int error;
    if (SQLITE_OK != (error = sqlite3_close_v2(m_dbHandle)))
        cerror() << "sqlite3_close_v2(): " << describeSqliteError(error);
}

SqlStatementImpl *SqliteTransactionImpl::createStatement(SqlStatementType type, const String& queryText)
{
    std::lock_guard<std::mutex> _guard(m_statementsMutex);
    SqliteStatementImpl *statement = new SqliteStatementImpl(type, queryText);
    m_statements.push_back(statement);
    return statement;
}

bool SqliteTransactionImpl::prepare(SqlStatementImpl *statement)
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
    reinterpret_cast<SqliteStatementImpl *>(statement)->setHandle(stmt);
    return true;
}

bool SqliteTransactionImpl::execStatement(SqlStatementImpl *statement)
{
    if (!statement->prepared())
    {
        // TODO: should be an exception
        cerror() << "SqliteTransactionImpl::execStatement(): should be prepared first";
        return false;
    }
    int error = sqlite3_step(reinterpret_cast<SqliteStatementImpl *>(statement)->handle());
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

bool SqliteTransactionImpl::fetchNext(SqlStatementImpl *statement, SqlStorable *storable)
{
    if (!statement->prepared())
    {
        cerror() << "SqliteTransactionImpl::execStatement(): should be prepared first";
        return false;
    }
    // no more rows
    if (statement->done())
        return false;
    sqlite3_stmt *stmt = reinterpret_cast<SqliteStatementImpl *>(statement)->handle();
    int error = sqlite3_step(stmt);
    if (SQLITE_DONE == error)
    {
        statement->setDone();
        return false;
    }
    if (SQLITE_ROW == error)
    {
        size_t columnCount = sqlite3_data_count(stmt);
        for (size_t i = 0; i < columnCount; ++i)
        {
            String name = sqlite3_column_name(stmt, i);
            auto field = storable->record()->metaObject()->fieldByName(name, false);
            if (!field)
            {
                cwarning() << "Cannot bind sql result to an object field " << name;
                continue;
            }
            int sqliteType = sqlite3_column_type(stmt, i);
#define _ASSIGN_FIELD(type, expType, val) \
    if (field->nullable() && sqliteType == SQLITE_NULL) {  \
        field->access<Nullable<type> >(storable->record()).reset(); \
    } else { \
        if (sqliteType != expType) \
            throw std::runtime_error(String(name + ": Integer expected").c_str()); \
        field->nullable() ? *field->access<Nullable<type> >(storable->record()) : \
                            field->access<type>(storable->record()) = \
                val; \
    }
            switch (field->type())
            {
            case eFieldBool:
                _ASSIGN_FIELD(bool, SQLITE_INTEGER, sqlite3_column_int(stmt, i) != 0)
                break;
            case eFieldInt:
                _ASSIGN_FIELD(int32_t, SQLITE_INTEGER, sqlite3_column_int(stmt, i))
                break;
            case eFieldEnum:
            case eFieldUint:
                _ASSIGN_FIELD(uint32_t, SQLITE_INTEGER, sqlite3_column_int64(stmt, i))
                break;
            case eFieldFloat:
                _ASSIGN_FIELD(float, SQLITE_FLOAT, sqlite3_column_double(stmt, i))
                break;
            case eFieldDouble:
                _ASSIGN_FIELD(double, SQLITE_FLOAT, sqlite3_column_double(stmt, i))
                break;
            case eFieldString:
                _ASSIGN_FIELD(String, SQLITE_TEXT, (const char *)sqlite3_column_text(stmt, i))
                break;
            case eFieldTime:
                throw std::runtime_error("Datetime unimplemented");
                break;
            case eFieldObject:
            case eFieldArray:
                throw std::runtime_error("Cannot handle non-plain objects");
            }
        }
        return true;
    }
    else
    {
        cerror() << "sqlite3_step(): " << describeSqliteError(error);
        return false;
    }
}

bool SqliteTransactionImpl::closeStatement(SqlStatementImpl *statement)
{
    std::lock_guard<std::mutex> _guard(m_statementsMutex);
    SqliteStatementImpl *sqliteStatement = reinterpret_cast<SqliteStatementImpl *>(statement);
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
