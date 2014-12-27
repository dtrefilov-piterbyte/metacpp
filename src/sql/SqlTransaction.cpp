#include "SqlTransaction.h"
#include "connectors/SqlConnectorBase.h"

namespace metacpp
{
namespace sql
{

SqlTransaction::SqlTransaction(connectors::SqlConnectorBase *connector, connectors::SqlTransactionImpl *impl)
    : m_connector(connector), m_impl(impl)
{
}

connectors::SqlConnectorBase *SqlTransaction::connector()
{
    return m_connector;
}

} // namespace sql
} // namespace metacpp
