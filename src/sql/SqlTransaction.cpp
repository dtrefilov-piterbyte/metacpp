#include "SqlTransaction.h"

namespace metacpp
{
namespace sql
{

SqlTransaction::SqlTransaction(connectors::SqlConnectorBase *connector)
    : m_connector(connector)
{
}

connectors::SqlConnectorBase *SqlTransaction::connector()
{
    return m_connector;
}

} // namespace sql
} // namespace metacpp
