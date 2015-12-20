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
#ifndef MYSQLSTATEMENTIMPL_H
#define MYSQLSTATEMENTIMPL_H
#include "SqlStatementImpl.h"
#include <mysql.h>

namespace metacpp {
namespace db {
namespace sql {
namespace connectors {
namespace mysql {

class MySqlStatementImpl : public SqlStatementImpl
{
public:
    MySqlStatementImpl(MYSQL_STMT *stmt, SqlStatementType type, const String& queryText);
    ~MySqlStatementImpl();

    MYSQL_STMT *getStmt() const;
    MYSQL_RES *getResult() const;
    void setResult(MYSQL_RES *result);
    bool getExecuted() const;
    void setExecuted(bool val = true);
    /** Stores bind data which will receive field requirements on the next fetched row */
    void prefetch();
    MYSQL_BIND *bindResult(size_t nField);
private:
    MYSQL_STMT *m_stmt;
    MYSQL_RES *m_result;
    Array<MYSQL_BIND> m_bindResult;
    bool m_executed;
};

} // namespace mysql
} // namespace connectors
} // namespace sql
} // namespace db
} // namespace metacpp

#endif // MYSQLSTATEMENTIMPL_H
