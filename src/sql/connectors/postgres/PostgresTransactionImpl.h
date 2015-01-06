#ifndef POSTGRESTRANSACTIONIMPL_H
#define POSTGRESTRANSACTIONIMPL_H
#include "SqlTransactionImpl.h"
#include "PostgresStatementImpl.h"
#include <libpq-fe.h>
#include <pg_config.h>

namespace metacpp {
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
    bool prepare(SqlStatementImpl *statement) override;
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
} // namespace metacpp

#endif // POSTGRESTRANSACTIONIMPL_H
