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
#include "SqliteTransactionImpl.h"
#include "SqliteConnector.h"

namespace metacpp
{
namespace db
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
    std::lock_guard<std::mutex> _guard(m_statementsMutex);
    if (m_statements.size())
    {
        std::cerr << "There's still " << m_statements.size() <<
                     " unclosed statements while destroing the sqlite transaction" << std::endl;
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
        std::cout << "SqliteTransactionImpl::commit(): sqlite3_exec(): "
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
        std::cerr << "SqliteTransactionImpl::rollback(): sqlite3_exec(): "
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

bool SqliteTransactionImpl::prepare(SqlStatementImpl *statement, size_t numParams)
{
    (void)numParams;
    const String& query = statement->queryText();
    sqlite3_stmt *stmt;
    //std::cout << query << std::endl;
    const char *pSqlTail;
    int error = sqlite3_prepare_v2(m_dbHandle, query.c_str(), (int)query.size() + 1,
        &stmt, &pSqlTail);
    if (SQLITE_OK != error)
    {
        std::cerr << "sqlite3_prepare_v2(): " << sqlite3_errmsg(m_dbHandle) << std::endl;
        std::cerr << query << std::endl;
        return false;
    }
    if (*pSqlTail)
        std::cerr << "Unused part of an SQL statement: " << pSqlTail;
    reinterpret_cast<SqliteStatementImpl *>(statement)->setHandle(stmt);
    return true;
}

bool SqliteTransactionImpl::bindValues(SqlStatementImpl *statement, const VariantArray &values)
{
    if (!statement->prepared())
        throw std::runtime_error("SqliteTransactionImpl::bindValues(): statement should be prepared first");
    sqlite3_stmt *stmt = reinterpret_cast<SqliteStatementImpl *>(statement)->handle();
    for (size_t i = 0; i < values.size(); ++i)
    {
        int error = SQLITE_OK;
        Variant v = values[i];
        if (!v.valid())
            error = sqlite3_bind_null(stmt, static_cast<int>(i + 1));
        else if (v.isIntegral())
            error = sqlite3_bind_int64(stmt, static_cast<int>(i + 1), variant_cast<int64_t>(v));
        else if (v.isFloatingPoint())
            error = sqlite3_bind_double(stmt, static_cast<int>(i + 1), variant_cast<double>(v));
        else if (v.isString() || v.isDateTime())
        {
            String s = variant_cast<String>(v);
            error = sqlite3_bind_text(stmt, static_cast<int>(i + 1), s.data(), s.length(), SQLITE_TRANSIENT);
        }
        else
            throw std::invalid_argument("Unsupported - not a scalar variant value");
        if (SQLITE_OK != error)
        {
            std::cerr << "sqlite3_bind_*(): " << sqlite3_errmsg(m_dbHandle) << std::endl;
            std::cerr << v << std::endl;
            return false;
        }
    }
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
    std::cerr << "sqlite3_step(): " << sqlite3_errmsg(m_dbHandle);
    return false;
}

template<typename T>
void assignField(const MetaFieldBase *field, int sqliteType, int expectedType, Object *obj, const T& val)
{
    if (field->nullable() && sqliteType == SQLITE_NULL)
        field->access<Nullable<T> >(obj).reset();
    else {
        if (sqliteType != expectedType)
            throw std::runtime_error(String(String(field->name()) + ": Type mismatch").c_str());
        if (field->nullable())
            field->access<Nullable<T> >(obj) = val;
        else
            field->access<T>(obj) = val;
    }
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
        int columnCount = sqlite3_data_count(stmt);
        for (int i = 0; i < columnCount; ++i)
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
                assignField<bool>(field, sqliteType, SQLITE_INTEGER, storable->record(), sqlite3_column_int(stmt, i) != 0);
                break;
            case eFieldInt:
                assignField<int32_t>(field, sqliteType, SQLITE_INTEGER, storable->record(), sqlite3_column_int(stmt, i));
                break;
            case eFieldEnum:
            case eFieldUint:
                assignField<uint32_t>(field, sqliteType, SQLITE_INTEGER, storable->record(), (uint32_t)sqlite3_column_int64(stmt, i));
                break;
            case eFieldUint64:
                assignField<uint64_t>(field, sqliteType, SQLITE_INTEGER, storable->record(), sqlite3_column_int64(stmt, i));
                break;
            case eFieldInt64:
                assignField<int64_t>(field, sqliteType, SQLITE_INTEGER, storable->record(), sqlite3_column_int64(stmt, i));
                break;
            case eFieldFloat:
                assignField<float>(field, sqliteType, SQLITE_FLOAT, storable->record(), (float)sqlite3_column_double(stmt, i));
                break;
            case eFieldDouble:
                assignField<double>(field, sqliteType, SQLITE_FLOAT, storable->record(), sqlite3_column_double(stmt, i));
                break;
            case eFieldString:
                assignField<String>(field, sqliteType, SQLITE_TEXT, storable->record(), (const char *)sqlite3_column_text(stmt, i));
                break;
            case eFieldDateTime:
                assignField<DateTime>(field, sqliteType, SQLITE_TEXT, storable->record(),
                                      sqliteType == SQLITE_NULL ? DateTime() :
                                          DateTime::fromString((const char *)sqlite3_column_text(stmt, i)));
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

size_t SqliteTransactionImpl::size(SqlStatementImpl *statement)
{
    (void)statement;
    return std::numeric_limits<size_t>::max();
}

bool SqliteTransactionImpl::getLastInsertId(SqlStatementImpl *statement, SqlStorable *storable)
{
    (void)statement;
    auto pkey = storable->primaryKey();
    if (pkey)
    {
        switch (storable->primaryKey()->type())
        {
        case eFieldInt:
            assignField<int32_t>(pkey, SQLITE_INTEGER, SQLITE_INTEGER, storable->record(),
                          (int32_t)sqlite3_last_insert_rowid(m_dbHandle));
            return true;
        case eFieldUint:
            assignField<uint32_t>(pkey, SQLITE_INTEGER, SQLITE_INTEGER, storable->record(),
                          (uint32_t)sqlite3_last_insert_rowid(m_dbHandle));
            return true;
        case eFieldInt64:
            assignField<int64_t>(pkey, SQLITE_INTEGER, SQLITE_INTEGER, storable->record(),
                          sqlite3_last_insert_rowid(m_dbHandle));
            return true;
        case eFieldUint64:
            assignField<uint64_t>(pkey, SQLITE_INTEGER, SQLITE_INTEGER, storable->record(),
                          sqlite3_last_insert_rowid(m_dbHandle));
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
} // namespace db
} // namespace metacpp
