#ifndef SQLTRANSACTION_H
#define SQLTRANSACTION_H
#include "SqlConnectorBase.h"

namespace metacpp
{
namespace sql
{

namespace connectors
{
    class SqlConnectorBase;
    class SqlTransactionImpl;
}

enum SqlTransactionAutoCloseMode
{
    SqlTransactionAutoRollback,
    SqlTransactionAutoCommit
};

class SqlTransaction
{
public:
    SqlTransaction(SqlTransactionAutoCloseMode autoClose = SqlTransactionAutoRollback,
                   connectors::SqlConnectorBase *connector = connectors::SqlConnectorBase::getDefaultConnector());

    virtual ~SqlTransaction();

    SqlTransaction(const SqlTransaction&)=delete;
    SqlTransaction operator=(const SqlTransaction&)=delete;

    connectors::SqlConnectorBase *connector() const;
    connectors::SqlTransactionImpl *impl() const;
    bool started() const;
    void begin();
    void commit();
    void rollback();
private:
    connectors::SqlConnectorBase *m_connector;
    connectors::SqlTransactionImpl *m_impl;
    SqlTransactionAutoCloseMode m_autoCloseMode;
    bool m_transactionStarted;
};

// TODO: needed some kind of SqlTransactionGuard with AutoCommit and AutoRollback disposition behaviour (as wrapper over SqlTransaction?)

} // namespace sql
} // namespace metacpp

#endif // SQLTRANSACTION_H
