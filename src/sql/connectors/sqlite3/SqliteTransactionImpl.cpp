#include "SqliteTransactionImpl.h"
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
            std::cerr << "There's still " << m_statements.size() <<
                         " unclosed statements while destroing the sqlite transaction" << std::endl;
        }
    }
}

bool SqliteTransactionImpl::begin()
{
    int error = sqlite3_exec(m_dbHandle, "BEGIN TRANSACTION", nullptr, nullptr, nullptr);
    if (error != SQLITE_OK)
    {
        std::cerr << "SqliteTransactionImpl::begin(): sqlite3_exec(): "
                  << sqlite3_errmsg(m_dbHandle) << std::endl;
        return false;
    }
    return true;
}

bool SqliteTransactionImpl::commit()
{
    int error = sqlite3_exec(m_dbHandle, "COMMIT TRANSACTION", nullptr, nullptr, nullptr);
    if (error != SQLITE_OK)
    {
        std::cout << "SqliteTransactionImpl::begin(): sqlite3_exec(): "
                  << sqlite3_errmsg(m_dbHandle) << std::endl;
        return false;
    }
    return true;
}

bool SqliteTransactionImpl::rollback()
{
    int error = sqlite3_exec(m_dbHandle, "ROLLBACK TRANSACTION", nullptr, nullptr, nullptr);
    if (error != SQLITE_OK)
    {
        std::cerr << "SqliteTransactionImpl::begin(): sqlite3_exec(): "
                  << sqlite3_errmsg(m_dbHandle) << std::endl;
        return false;
    }
    return true;
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
        throw std::runtime_error(std::string("sqlite3_prepare_v2(): ") + sqlite3_errmsg(m_dbHandle));
    reinterpret_cast<SqliteStatementImpl *>(statement)->setHandle(stmt);
    return true;
}

bool SqliteTransactionImpl::execStatement(SqlStatementImpl *statement, int *numRowsAffected)
{
    if (!statement->prepared())
        throw std::runtime_error("SqliteTransactionImpl::execStatement(): should be prepared first");
    int error = sqlite3_step(reinterpret_cast<SqliteStatementImpl *>(statement)->handle());
    if (SQLITE_DONE == error)
    {
        statement->setDone();
        if (numRowsAffected) *numRowsAffected = sqlite3_changes(m_dbHandle);
        return true;
    }
    else if (SQLITE_ROW == error)
    {
        if (numRowsAffected) *numRowsAffected = sqlite3_changes(m_dbHandle);
        return true;
    }
    throw std::runtime_error(std::string("sqlite3_step(): ") + sqlite3_errmsg(m_dbHandle));
}

#define _ASSIGN_FIELD(field, type, sqliteType, expType, val) \
    if (field->nullable() && sqliteType == SQLITE_NULL) {  \
        field->access<Nullable<type> >(storable->record()).reset(); \
    } else { \
        if (sqliteType != expType) \
            throw std::runtime_error(String(String(field->name()) + ": Type mismatch").c_str()); \
        if (field->nullable()) \
            field->access<Nullable<type> >(storable->record()) = val; \
        else \
            field->access<type>(storable->record()) = val; \
    }

bool SqliteTransactionImpl::fetchNext(SqlStatementImpl *statement, SqlStorable *storable)
{
    if (!statement->prepared())
        throw std::runtime_error("SqliteTransactionImpl::execStatement(): should be prepared first");
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
                std::cerr << "Cannot bind sql result to an object field " << name << std::endl;
                continue;
            }
            int sqliteType = sqlite3_column_type(stmt, i);

            switch (field->type())
            {
            case eFieldBool:
                _ASSIGN_FIELD(field, bool, sqliteType, SQLITE_INTEGER, sqlite3_column_int(stmt, i) != 0)
                break;
            case eFieldInt:
                _ASSIGN_FIELD(field, int32_t, sqliteType, SQLITE_INTEGER, sqlite3_column_int(stmt, i))
                break;
            case eFieldEnum:
            case eFieldUint:
                _ASSIGN_FIELD(field, uint32_t, sqliteType, SQLITE_INTEGER, sqlite3_column_int64(stmt, i))
                break;
            case eFieldUint64:
            case eFieldInt64:
                _ASSIGN_FIELD(field, int64_t, sqliteType, SQLITE_INTEGER, sqlite3_column_int64(stmt, i))
                break;
            case eFieldFloat:
                _ASSIGN_FIELD(field, float, sqliteType, SQLITE_FLOAT, sqlite3_column_double(stmt, i))
                break;
            case eFieldDouble:
                _ASSIGN_FIELD(field, double, sqliteType, SQLITE_FLOAT, sqlite3_column_double(stmt, i))
                break;
            case eFieldString:
                _ASSIGN_FIELD(field, String, sqliteType, SQLITE_TEXT, (const char *)sqlite3_column_text(stmt, i))
                break;
            case eFieldDateTime:
                _ASSIGN_FIELD(field, DateTime, sqliteType, SQLITE_TEXT, DateTime::fromISOString((const char *)sqlite3_column_text(stmt, i)))
                break;
            case eFieldObject:
            case eFieldArray:
                throw std::runtime_error("Cannot handle non-plain objects");
            default:
                throw std::runtime_error("Unknown field type");
            }
        }
        return true;
    }
    throw std::runtime_error(std::string("sqlite3_step(): ") + sqlite3_errmsg(m_dbHandle));
}

bool SqliteTransactionImpl::getLastInsertId(SqlStatementImpl *statement, SqlStorable *storable)
{
    (void)statement;
    if (storable->primaryKey())
    {
        switch (storable->primaryKey()->type())
        {
        case eFieldInt:
            _ASSIGN_FIELD(storable->primaryKey(), int32_t, SQLITE_INTEGER, SQLITE_INTEGER,
                          sqlite3_last_insert_rowid(m_dbHandle))
            return true;
        case eFieldUint:
            _ASSIGN_FIELD(storable->primaryKey(), uint32_t, SQLITE_INTEGER, SQLITE_INTEGER,
                          sqlite3_last_insert_rowid(m_dbHandle))
            return true;
        case eFieldInt64:
            _ASSIGN_FIELD(storable->primaryKey(), int64_t, SQLITE_INTEGER, SQLITE_INTEGER,
                          sqlite3_last_insert_rowid(m_dbHandle))
            return true;
        case eFieldUint64:
            _ASSIGN_FIELD(storable->primaryKey(), uint64_t, SQLITE_INTEGER, SQLITE_INTEGER,
                          sqlite3_last_insert_rowid(m_dbHandle))
            return true;
        default:
            return false;
        }
    }
    return false;
}

bool SqliteTransactionImpl::closeStatement(SqlStatementImpl *statement)
{
    std::lock_guard<std::mutex> _guard(m_statementsMutex);
    SqliteStatementImpl *sqliteStatement = reinterpret_cast<SqliteStatementImpl *>(statement);
    auto it = std::find(m_statements.begin(), m_statements.end(), sqliteStatement);
    if (it == m_statements.end())
    {
        std::cerr << "SqliteTransactionImpl::closeStatement(): there's no such statement" << std::endl;
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
