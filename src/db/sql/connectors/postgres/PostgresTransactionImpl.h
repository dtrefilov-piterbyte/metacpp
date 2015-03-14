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
#ifndef POSTGRESTRANSACTIONIMPL_H
#define POSTGRESTRANSACTIONIMPL_H
#include "SqlTransactionImpl.h"
#include "PostgresStatementImpl.h"
#include <libpq-fe.h>
#include <pg_config.h>

namespace metacpp {
namespace db {
namespace sql {
namespace connectors {
namespace postgres {

class PostgresTransactionImpl : public SqlTransactionImpl
{
public:
    PostgresTransactionImpl(PGconn *dbConn);
    ~PostgresTransactionImpl();

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

    PGconn *dbConn() const { return m_dbConn; }
private:
    bool execCommand(const char *query, const char *invokeContext);
private:
    PGconn *m_dbConn;
    Array<PostgresStatementImpl *> m_statements;
    std::mutex m_statementsMutex;
};

} // namespace postgres
} // namespace connectors
} // namespace sql
} // namespace db
} // namespace metacpp

#endif // POSTGRESTRANSACTIONIMPL_H
