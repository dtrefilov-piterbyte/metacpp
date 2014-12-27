#include "SqliteStatement.h"

namespace metacpp
{
namespace sql
{
namespace connectors
{
namespace sqlite
{

SqliteStatement::SqliteStatement(SqlStatementType type, const String &queryText)
    : SqlStatementBase(type, queryText), m_stmt(nullptr)
{
}

SqliteStatement::~SqliteStatement()
{

}

sqlite3_stmt *SqliteStatement::handle() const
{
    return m_stmt;
}

void SqliteStatement::setHandle(sqlite3_stmt *handle)
{
    m_stmt = handle;
    setPrepared(m_stmt != nullptr);
}

} // namespace sqlite
} // namespace connectors
} // namespace sql
} // namespace metacpp
