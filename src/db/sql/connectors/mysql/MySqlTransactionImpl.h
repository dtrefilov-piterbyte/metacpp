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
#ifndef MYSQLTRANSACTIONIMPL_H
#define MYSQLTRANSACTIONIMPL_H
#include "SqlTransactionImpl.h"
#include "MySqlStatementImpl.h"
#include <mysql.h>

namespace metacpp {
namespace db {
namespace sql {
namespace connectors {
namespace mysql {

class MySqlTransactionImpl : public SqlTransactionImpl
{
public:
    MySqlTransactionImpl(MYSQL *dbConn);
    ~MySqlTransactionImpl();

    bool begin() override;
    bool commit() override;
    bool rollback() override;

    SqlStatementImpl *createStatement(SqlStatementType type, const String& queryText) override;
    bool prepare(SqlStatementImpl *statement, size_t numParams) override;
    bool bindValues(SqlStatementImpl *statement, const VariantArray &values) override;
    bool execStatement(SqlStatementImpl *statement, int *numRowsAffected = nullptr) override;
    bool fetchNext(SqlStatementImpl *statement, SqlStorable *storable) override;
    bool getLastInsertId(SqlStatementImpl *statement, SqlStorable *storable) override;
    bool closeStatement(SqlStatementImpl *statement) override;

    MYSQL *dbConn() const { return m_dbConn; }
private:
    bool execCommand(const char *query, const char *invokeContext);
private:
    MYSQL *m_dbConn;
    Array<MySqlStatementImpl *> m_statements;
    std::mutex m_statementsMutex;
};

} // namespace mysql
} // namespace connectors
} // namespace sql
} // namespace db
} // namespace metacpp

#endif // MYSQLTRANSACTIONIMPL_H
