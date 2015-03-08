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
#include "MySqlStatementImpl.h"

namespace metacpp {
namespace db
{
namespace sql {
namespace connectors {
namespace mysql {

MySqlStatementImpl::MySqlStatementImpl(MYSQL_STMT *stmt, SqlStatementType type, const String &queryText)
    : SqlStatementImpl(type, queryText), m_stmt(stmt), m_result(nullptr), m_executed(false)
{

}

MySqlStatementImpl::~MySqlStatementImpl()
{
    if (m_stmt)
        mysql_stmt_close(m_stmt);
    if (m_result)
        mysql_free_result(m_result);
}

MYSQL_STMT *MySqlStatementImpl::getStmt() const
{
    return m_stmt;
}

MYSQL_RES *MySqlStatementImpl::getResult() const
{
    return m_result;
}

void MySqlStatementImpl::setResult(MYSQL_RES *result)
{
    m_result = result;
}

bool MySqlStatementImpl::getExecuted() const
{
    return m_executed;
}

void MySqlStatementImpl::setExecuted(bool val)
{
    m_executed = val;
}

void MySqlStatementImpl::prefetch()
{
    if (!m_result)
        throw std::runtime_error("Result was not set");
    unsigned int numFields = mysql_num_fields(m_result);
    m_bindResult.resize(numFields);
    m_fieldLengths.resize(numFields);
    memset(m_bindResult.data(), 0, m_bindResult.size() * sizeof(MYSQL_BIND));
    std::fill(m_fieldLengths.begin(), m_fieldLengths.end(), 0);
    for (size_t i = 0; i < numFields; ++i)
        m_bindResult[i].length = &m_fieldLengths[i];
    int res = mysql_stmt_bind_result(m_stmt, m_bindResult.data());
    if (0 != res)
        throw std::runtime_error("mysql_stmt_bind_result() failed");
}

size_t MySqlStatementImpl::bufferLengthRequired(size_t nField)
{
    return m_fieldLengths[nField];
}

MYSQL_BIND *MySqlStatementImpl::bindResult(size_t nField)
{
    return &m_bindResult[nField];
}

} // namespace mysql
} // namespace connectors
} // namespace sql
} // namespace db
} // namespace metacpp

