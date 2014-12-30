#include "SqliteStatementImpl.h"

namespace metacpp
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
} // namespace metacpp
