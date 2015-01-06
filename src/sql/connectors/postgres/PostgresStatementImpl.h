#ifndef POSTGRESSTATEMENTIMPL_H
#define POSTGRESSTATEMENTIMPL_H
#include "SqlStatementImpl.h"
#include <libpq-fe.h>
#include <pg_config.h>

namespace metacpp {
namespace sql {
namespace connectors {
namespace postgres {

class PostgresStatementImpl : public SqlStatementImpl
{
public:
    PostgresStatementImpl(SqlStatementType type, const String& queryText);
    ~PostgresStatementImpl();

    void setResult(PGresult *result, const String& idString);
    PGresult *getResult() const;
    const String& getIdString() const;
    int currentRow() const;
    void setCurrentRow(int row);
private:
    PGresult *m_result;
    String m_idString;
    int m_currentRow;
};

} // namespace postgres
} // namespace connectors
} // namespace sql
} // namespace metacpp

#endif // POSTGRESSTATEMENTIMPL_H
