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
#ifndef SQLITESTATEMENT_H
#define SQLITESTATEMENT_H
#include "SqlStatementImpl.h"
#include <sqlite3.h>

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

class SqliteStatementImpl : public SqlStatementImpl
{
public:
    SqliteStatementImpl(SqlStatementType type, const String& queryText);
    ~SqliteStatementImpl();

    sqlite3_stmt *handle() const;
    void setHandle(sqlite3_stmt *handle);
private:
    sqlite3_stmt *m_stmt;
};

} // namespace sqlite
} // namespace connectors
} // namespace sql
} // namespace db
} // namespace metacpp

#endif // SQLITESTATEMENT_H
