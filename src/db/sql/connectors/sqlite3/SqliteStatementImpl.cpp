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
#include "SqliteStatementImpl.h"

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

SqliteStatementImpl::SqliteStatementImpl(SqlStatementType type, const String &queryText)
    : SqlStatementImpl(type, queryText), m_stmt(nullptr)
{
}

SqliteStatementImpl::~SqliteStatementImpl()
{
    if (m_stmt && prepared()) sqlite3_finalize(m_stmt);
}

sqlite3_stmt *SqliteStatementImpl::handle() const
{
    return m_stmt;
}

void SqliteStatementImpl::setHandle(sqlite3_stmt *handle)
{
    m_stmt = handle;
    setPrepared(m_stmt != nullptr);
}

} // namespace sqlite
} // namespace connectors
} // namespace sql
} // namespace db
} // namespace metacpp
