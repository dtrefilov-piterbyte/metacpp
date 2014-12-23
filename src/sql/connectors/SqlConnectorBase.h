#ifndef SQLCONNECTORBASE_H
#define SQLCONNECTORBASE_H
#include "SqlResultSet.h"
#include "SqlStatementBase.h"

namespace metacpp
{
namespace sql
{
namespace connectors
{

class SqlConnectorBase
{
public:
    SqlConnectorBase();
    SqlConnectorBase(const SqlConnectorBase&)=delete;
    SqlConnectorBase& operator=(const SqlConnectorBase&)=delete;
    virtual ~SqlConnectorBase();

    /** prepare statement */
    virtual bool prepare(SqlStatementBasePtr statement);

};

} // namespace connectors
} // namespace sql
} // namespace metacpp

#endif
