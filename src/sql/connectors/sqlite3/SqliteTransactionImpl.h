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
#ifndef SQLITETRANSACTIONIMPL_H
#define SQLITETRANSACTIONIMPL_H
#include "SqlTransactionImpl.h"
#include "SqliteStatementImpl.h"
#include "Array.h"
#include <sqlite3.h>
#include <mutex>

namespace metacpp
{
namespace sql
{
namespace connectors
{
namespace sqlite
{

class SqliteTransactionImpl : public SqlTransactionImpl
{
public:
    SqliteTransactionImpl(sqlite3 *dbHandle);
    ~SqliteTransactionImpl();

    bool begin() override;
    bool commit() override;
    bool rollback() override;

    SqlStatementImpl *createStatement(SqlStatementType type, const String& queryText) override;
    bool prepare(SqlStatementImpl *statement) override;
    bool execStatement(SqlStatementImpl *statement, int *numRowsAffected = nullptr) override;
    bool fetchNext(SqlStatementImpl *statement, SqlStorable *storable) override;
    bool getLastInsertId(SqlStatementImpl *statement, SqlStorable *storable) override;
    bool closeStatement(SqlStatementImpl *statement) override;

    sqlite3 *dbHandle() const { return m_dbHandle; }
private:
    sqlite3 *m_dbHandle;
    Array<SqliteStatementImpl *> m_statements;
    std::mutex m_statementsMutex;
};

} // namespace sqlite
} // namespace connectors
} // namespace sql
} // namespace metacpp

#endif // SQLITETRANSACTIONIMPL_H
