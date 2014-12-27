#ifndef SQLTRANSACTION_H
#define SQLTRANSACTION_H

namespace metacpp
{
namespace sql
{

namespace connectors
{
    class SqlConnectorBase;
    class SqlTransactionImpl;
}

class SqlTransaction
{
    // never instantiated directly, created via SqlConnectorBase
    SqlTransaction(connectors::SqlConnectorBase *connector,
                   connectors::SqlTransactionImpl *impl);
public:

    virtual ~SqlTransaction();

    connectors::SqlConnectorBase *connector();
private:
    connectors::SqlConnectorBase *m_connector;
    connectors::SqlTransactionImpl *m_impl;
};

// TODO: needed some kind of SqlTransactionGuard with AutoCommit and AutoRollback disposition behaviour (as wrapper over SqlTransaction?)

} // namespace sql
} // namespace metacpp

#endif // SQLTRANSACTION_H
