#include "PostgresStatementImpl.h"

namespace metacpp {
namespace sql {
namespace connectors {
namespace postgres {

PostgresStatementImpl::PostgresStatementImpl(SqlStatementType type, const String &queryText)
    : SqlStatementImpl(type, queryText), m_result(nullptr), m_currentRow(-1)
{

}

PostgresStatementImpl::~PostgresStatementImpl()
{
    if (prepared() && m_result)
        PQclear(m_result);
}

void PostgresStatementImpl::setResult(PGresult *result, const String &idString)
{
    m_result = result;
    m_idString = idString;
}

PGresult *PostgresStatementImpl::getResult() const
{
    return m_result;
}

const String &PostgresStatementImpl::getIdString() const
{
    return m_idString;
}

int PostgresStatementImpl::currentRow() const
{
    return m_currentRow;
}

void PostgresStatementImpl::setCurrentRow(int row)
{
    m_currentRow = row;
}

} // namespace postgres
} // namespace connectors
} // namespace sql
} // namespace metacpp

