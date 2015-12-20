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
#include "MySqlTransactionImpl.h"

namespace metacpp {
namespace db {
namespace sql {
namespace connectors {
namespace mysql {

MySqlTransactionImpl::MySqlTransactionImpl(MYSQL *dbConn)
    : m_dbConn(dbConn)
{

}

MySqlTransactionImpl::~MySqlTransactionImpl()
{
    std::lock_guard<std::mutex> _guard(m_statementsMutex);
    if (m_statements.size())
    {
        std::cerr << "There's still " << m_statements.size() <<
                     " unclosed statements while destroing the postgres transaction" << std::endl;
    }
}

bool MySqlTransactionImpl::begin()
{
    return execCommand("BEGIN", "MySqlTransactionImpl::begin()");
}

bool MySqlTransactionImpl::commit()
{
    return execCommand("COMMIT", "MySqlTransactionImpl::commit()");
}

bool MySqlTransactionImpl::rollback()
{
    return execCommand("ROLLBACK", "MySqlTransactionImpl::rollback()");
}

SqlStatementImpl *MySqlTransactionImpl::createStatement(SqlStatementType type, const String& queryText)
{
    std::lock_guard<std::mutex> _guard(m_statementsMutex);
    MYSQL_STMT *stmt = mysql_stmt_init(dbConn());
    if (!stmt)
    {
        std::cerr << "mysql_stmt_init() failed:" << mysql_error(dbConn()) << std::endl;
        return nullptr;
    }
    MySqlStatementImpl *statement = new MySqlStatementImpl(stmt, type, queryText);
    m_statements.push_back(statement);
    return statement;
}

bool MySqlTransactionImpl::prepare(SqlStatementImpl *statement, size_t numParams)
{
    (void)numParams;
    MySqlStatementImpl *mysqlStatement = reinterpret_cast<MySqlStatementImpl *>(statement);
    int res = mysql_stmt_prepare(mysqlStatement->getStmt(), statement->queryText().c_str(), statement->queryText().length());
    if (0 != res)
    {
        std::cerr << statement->queryText() << std::endl;
        std::cerr << "mysql_stmt_prepare() failed: " << mysql_error(dbConn()) << std::endl;
        return false;
    }
    statement->setPrepared();
    return true;
}

bool MySqlTransactionImpl::bindValues(SqlStatementImpl *statement, const VariantArray &values)
{
    if (!statement->prepared())
        throw std::runtime_error("SqliteTransactionImpl::bindValues(): statement should be prepared first");
    MySqlStatementImpl *mysqlStatement = reinterpret_cast<MySqlStatementImpl *>(statement);
    MYSQL_BIND *binds = (MYSQL_BIND *)alloca(sizeof(MYSQL_BIND) * values.size());
    MYSQL_TIME *time_buffers = (MYSQL_TIME *)alloca(sizeof(MYSQL_TIME) * values.size());
    for (size_t i = 0; i < values.size(); ++i)
    {
        MYSQL_BIND& bind = binds[i];
        Variant v = values[i];
        memset(&bind, 0, sizeof(bind));
        if (!v.valid())
        {
            bind.is_null = &bind.is_null_value;
            bind.is_null_value = true;
        }
        else
        {
            switch (values[i].type())
            {
            case eFieldBool:
                bind.buffer_type = MYSQL_TYPE_TINY;
                bind.buffer = v.buffer();
                break;
            case eFieldUint:
                bind.is_unsigned = true; // fall through
            case eFieldInt:
            case eFieldEnum:
                bind.buffer_type = MYSQL_TYPE_LONG;
                bind.buffer = v.buffer();
                break;
            case eFieldUint64:
                bind.is_unsigned = true; // fall through
            case eFieldInt64:
                bind.buffer_type = MYSQL_TYPE_LONGLONG;
                bind.buffer = v.buffer();
                break;
            case eFieldFloat:
                bind.buffer_type = MYSQL_TYPE_FLOAT;
                bind.buffer = v.buffer();
                break;
            case eFieldDouble:
                bind.buffer_type = MYSQL_TYPE_DOUBLE;
                bind.buffer = v.buffer();
                break;
            case eFieldString: {
                const String *pStr = reinterpret_cast<const String *>(v.buffer());
                bind.buffer_type = MYSQL_TYPE_STRING;
                bind.buffer_length = pStr->length();
                bind.buffer = const_cast<char *>(pStr->data());
                break;
            }
            case eFieldDateTime: {
                bind.buffer_type = MYSQL_TYPE_DATETIME;
                bind.buffer = &time_buffers[i];
                memset(&time_buffers[i], 0, sizeof(MYSQL_TIME));
                DateTime dt = v.value<DateTime>();
                time_buffers[i].year = std::abs(dt.year());
                time_buffers[i].month = static_cast<int>(dt.month()) + 1;
                time_buffers[i].day = dt.day();
                time_buffers[i].hour = dt.hours();
                time_buffers[i].minute = dt.minutes();
                time_buffers[i].second = dt.seconds();
                time_buffers[i].neg = dt.year() < 0;
                time_buffers[i].time_type = MYSQL_TIMESTAMP_DATETIME;
                break;
            }
            case eFieldObject:
            case eFieldArray:
                throw std::runtime_error("Cannot bind denormalized data");
            default:
                throw std::runtime_error("Unknown field type");
            }
        }
    }
    int res = mysql_stmt_bind_param(mysqlStatement->getStmt(), binds);
    if (0 != res)
    {
        std::cerr << "mysql_stmt_bind_param() failed: " << mysql_error(dbConn()) << std::endl;
        return false;
    }
    return true;
}

bool MySqlTransactionImpl::execStatement(SqlStatementImpl *statement, int *numRowsAffected)
{
    if (!statement->prepared())
        throw std::runtime_error("MySqlTransactionImpl::execStatement(): should be prepared first");
    MySqlStatementImpl *mysqlStatement = reinterpret_cast<MySqlStatementImpl *>(statement);
    int res = mysql_stmt_execute(mysqlStatement->getStmt());
    if (0 != res)
    {
        std::cerr << "mysql_stmt_execute() failed: " << mysql_error(dbConn()) << std::endl;
        return false;
    }
    if (numRowsAffected) *numRowsAffected = mysql_stmt_affected_rows(mysqlStatement->getStmt());
    return true;
}

template<typename T>
void accessField(const MetaFieldBase *field, Object *object, MYSQL_BIND *bind)
{
    bind->is_unsigned = false;
    switch (field->type())
    {
    case eFieldBool:
        bind->buffer_type = MYSQL_TYPE_TINY;
        break;
    case eFieldUint:
        bind->is_unsigned = true; // fall through
    case eFieldInt:
    case eFieldEnum:
        bind->buffer_type = MYSQL_TYPE_LONG;
        break;
    case eFieldUint64:
        bind->is_unsigned = true; // fall through
    case eFieldInt64:
        bind->buffer_type = MYSQL_TYPE_LONGLONG;
        break;
    case eFieldFloat:
        bind->buffer_type = MYSQL_TYPE_FLOAT;
        break;
    case eFieldDouble:
        bind->buffer_type = MYSQL_TYPE_DOUBLE;
        break;
    case eFieldString:
        bind->buffer_type = MYSQL_TYPE_STRING;
        field->access<String>(object).resize(*bind->length);
        bind->buffer = const_cast<char *>(field->access<String>(object).data());
        return;
    case eFieldDateTime:
        bind->buffer_type = MYSQL_TYPE_DATETIME;
        break;
    case eFieldObject:
    case eFieldArray:
        throw std::runtime_error("Cannot bind denormalized data");
    default:
        throw std::runtime_error("Unknown field type");
    }
    if (field->nullable())
    {
        field->access<Nullable<T> >(object).reset(true);
        bind->buffer = &field->access<Nullable<T> >(object).get();
    }
    else
        bind->buffer = &field->access<T>(object);
}

bool MySqlTransactionImpl::fetchNext(SqlStatementImpl *statement, SqlStorable *storable)
{
    if (!statement->prepared())
        throw std::runtime_error("MySqlTransactionImpl::execStatement(): should be prepared first");
    if (statement->done())
        return false;
    MySqlStatementImpl *mysqlStatement = reinterpret_cast<MySqlStatementImpl *>(statement);
    if (!mysqlStatement->getExecuted())
    {
        int execRes = mysql_stmt_execute(mysqlStatement->getStmt());
        if (0 != execRes)
            throw std::runtime_error(std::string() + "mysql_stmt_execute() failed: " + mysql_error(dbConn()));
        mysqlStatement->setExecuted();
    }

    MYSQL_RES * res = mysqlStatement->getResult();
    if (!res)
    {
        res = mysql_stmt_result_metadata(mysqlStatement->getStmt());
        if (!res)
            throw std::runtime_error(std::string() + "mysql_stmt_result_metadata() failed: " + mysql_error(dbConn()));
        mysqlStatement->setResult(res);
    }
    unsigned int nFields = mysql_num_fields(res);
    mysqlStatement->prefetch();
    int fetchRes = mysql_stmt_fetch(mysqlStatement->getStmt());
    if (MYSQL_NO_DATA == fetchRes)
    {
        mysqlStatement->setDone();
        return false;
    }
    if (MYSQL_DATA_TRUNCATED != fetchRes && 0 != fetchRes)
        throw std::runtime_error(std::string() + "mysql_stmt_fetch() failed: " + mysql_error(dbConn()));

    for (unsigned int i = 0; i < nFields; ++i)
    {
        MYSQL_FIELD *mysqlField = mysql_fetch_field_direct(res, i);
        auto field = storable->record()->metaObject()->fieldByName(String(mysqlField->name, mysqlField->name_length), false);
        if (!field)
        {
            std::cerr << "Cannot bind sql result to an object field " << field->name() << std::endl;
            continue;
        }

        if (mysqlStatement->bindResult(i)->is_null)
        {
            if (!field->nullable()) throw std::runtime_error(std::string() + "Field " + field->name() + " is not nullable");
            field->setValue(Variant(), storable->record());
        }
        MYSQL_BIND *bind = mysqlStatement->bindResult(i);
        MYSQL_TIME timeBuffer;
        bind->buffer_length = *bind->length;

        switch (field->type())
        {
        case eFieldBool:
            accessField<bool>(field, storable->record(), bind);
            break;
        case eFieldInt:
        case eFieldEnum:
            accessField<int32_t>(field, storable->record(), bind);
            break;
        case eFieldUint:
            accessField<uint32_t>(field, storable->record(), bind);
            break;
        case eFieldInt64:
            accessField<int64_t>(field, storable->record(), bind);
            break;
        case eFieldUint64:
            accessField<uint64_t>(field, storable->record(), bind);
            break;
        case eFieldFloat:
            accessField<float>(field, storable->record(), bind);
            break;
        case eFieldDouble:
            accessField<double>(field, storable->record(), bind);
            break;
        case eFieldString:
            accessField<String>(field, storable->record(), bind);
            break;
        case eFieldDateTime:
            bind->buffer_length = sizeof(timeBuffer);
            bind->buffer = &timeBuffer;
            bind->buffer_type = MYSQL_TYPE_DATETIME;
            break;
        case eFieldObject:
        case eFieldArray:
            throw std::runtime_error("Cannot handle denormalized data");
        default:
            throw std::runtime_error("Unknown field type");
        }
        int fetchRes = mysql_stmt_fetch_column(mysqlStatement->getStmt(), bind, i, 0);
        if (0 != fetchRes)
            throw std::runtime_error(std::string() + "mysql_stmt_fetch_column() failed:" + mysql_error(dbConn()));
        if (eFieldDateTime == field->type())
            field->setValue(DateTime(timeBuffer.year,
                                     static_cast<EMonth>(timeBuffer.month - 1), timeBuffer.day, timeBuffer.hour,
                                     timeBuffer.minute, timeBuffer.second), storable->record());
    }
    return true;
}

size_t MySqlTransactionImpl::size(SqlStatementImpl *statement)
{
    const size_t def = std::numeric_limits<size_t>::max();
    if (!statement->prepared())
        throw std::runtime_error("MySqlTransactionImpl::execStatement(): should be prepared first");
    if (statement->done())
        return def;
    MySqlStatementImpl *mysqlStatement = reinterpret_cast<MySqlStatementImpl *>(statement);
    MYSQL_RES * res = mysqlStatement->getResult();
    if (res)
        return mysql_num_rows(res);
    return def;
}

bool MySqlTransactionImpl::getLastInsertId(SqlStatementImpl *statement, SqlStorable *storable)
{
    (void)statement;
    auto pkey = storable->primaryKey();
    if (!pkey) return false;
    pkey->setValue((uint64_t)mysql_insert_id(dbConn()), storable->record());

    return true;
}

bool MySqlTransactionImpl::closeStatement(SqlStatementImpl *statement)
{
    std::lock_guard<std::mutex> _guard(m_statementsMutex);
    MySqlStatementImpl *mysqlStatement = reinterpret_cast<MySqlStatementImpl *>(statement);
    auto it = std::find(m_statements.begin(), m_statements.end(), mysqlStatement);
    if (it == m_statements.end())
    {
        std::cerr << "MySqlTransactionImpl::closeStatement(): there's no such statement" << std::endl;
        return false;
    }
    m_statements.erase(it);
    delete mysqlStatement;
    return true;
}

bool MySqlTransactionImpl::execCommand(const char *query, const char *invokeContext)
{
    int res = mysql_query(dbConn(), query);
    if (0 != res)
    {
        std::cerr << invokeContext << ": mysql_query() failed: " << mysql_error(dbConn()) << std::endl;
        return false;
    }
    return true;
}

} // namespace mysql
} // namespace connectors
} // namespace sql
} // namespace db
} // namespace metacpp
