#ifndef SQLCONNECTORBASE_H
#define SQLCONNECTORBASE_H
#include "SqlResultSet.h"
#include "SqlStatementImpl.h"
#include "SqlTransaction.h"
#include "SqlStorable.h"

namespace metacpp
{
namespace sql
{
namespace connectors
{

/** \brief Result of the fetchNext operation */
enum FetchResult
{
    eFetchError,    /** operation unsupported */
    eFetchRow,      /** next row successfully fetched */
    eFetchEnd       /** end of query result reached, no more rows */
};

class SqlConnectorBase
{
protected:
    SqlConnectorBase();
public:
    SqlConnectorBase(const SqlConnectorBase&)=delete;
    SqlConnectorBase& operator=(const SqlConnectorBase&)=delete;

    virtual ~SqlConnectorBase();

    /** \brief Perform initial connection to the database */
    virtual bool connect() = 0;

    /** \brief Close connection to the database */
    virtual bool disconnect() = 0;

    /** \brief create a new transaction using existing connection to the database.
     * After you finish with transaction you should return it back to the connector
     * using either commitTransaction or or rollbackTransaction
     * NOTE: Level of isolation for transaction is implementation defined
    */
    virtual SqlTransaction *beginTransaction() = 0;

    /** \brief execute all commands and make all changes made within given transaction persistent */
    virtual bool commitTransaction(SqlTransaction *transaction) = 0;

    /** \brief cancel all changes made within given transaction */
    virtual bool rollbackTransaction(SqlTransaction *transaction) = 0;

};

} // namespace connectors
} // namespace sql
} // namespace metacpp

#endif
