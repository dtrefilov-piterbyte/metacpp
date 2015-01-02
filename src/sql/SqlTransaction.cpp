#include "SqlTransaction.h"
#include "SqlConnectorBase.h"
#include "SqlTransactionImpl.h"

namespace metacpp
{
namespace sql
{

SqlTransaction::SqlTransaction(SqlTransactionAutoCloseMode autoClose, connectors::SqlConnectorBase *connector)
    : m_connector(connector), m_impl(nullptr), m_autoCloseMode(autoClose),
      m_transactionOpened(false)
{
    m_impl = connector->beginTransaction();
    if (!(m_transactionOpened = m_impl != nullptr))
        throw std::runtime_error("Failed to create transaction");
}

SqlTransaction::~SqlTransaction()
{
    if (m_transactionOpened)
    {
        switch (m_autoCloseMode)
        {
        case SqlTransactionAutoCommit:
            commit();
            break;
        case SqlTransactionAutoRollback:
            rollback();
            break;
        default:
            throw std::runtime_error("Unknown autoclose mode");
        }
    }
}

connectors::SqlConnectorBase *SqlTransaction::connector() const
{
    return m_connector;
}

connectors::SqlTransactionImpl *SqlTransaction::impl() const
{
    return m_impl;
}

bool SqlTransaction::connected() const
{
    return m_transactionOpened;
}

void SqlTransaction::commit()
{
    if (!connected())
        throw std::runtime_error("Transaction already closed");
    if (m_connector->commitTransaction(m_impl))
    {
        m_impl = nullptr;
        m_transactionOpened = false;
    }
    else
        throw std::runtime_error("Commit failed");
}

void SqlTransaction::rollback()
{

    if (!connected())
        throw std::runtime_error("Transaction already closed");
    if (m_connector->rollbackTransaction(m_impl))
    {
        m_impl = nullptr;
        m_transactionOpened = false;
    }
    else
        throw std::runtime_error("Commit failed");
}

} // namespace sql
} // namespace metacpp
