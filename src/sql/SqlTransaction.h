#ifndef SQLTRANSACTION_H
#define SQLTRANSACTION_H
#include "connectors/SqlConnectorBase.h"

namespace metacpp
{
namespace sql
{

class SqlTransaction
{
    // created via SqlConnectorBase
    SqlTransaction(connectors::SqlConnectorBase *connector);
public:

    ~SqlTransaction();

    connectors::SqlConnectorBase *connector();
private:
    connectors::SqlConnectorBase *m_connector;

};

} // namespace sql
} // namespace metacpp

#endif // SQLTRANSACTION_H
