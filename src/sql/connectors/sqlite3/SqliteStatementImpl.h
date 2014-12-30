#ifndef SQLITESTATEMENT_H
#define SQLITESTATEMENT_H
#include "SqlStatementImpl.h"
#include <sqlite3.h>

namespace metacpp
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
} // namespace metacpp

#endif // SQLITESTATEMENT_H
