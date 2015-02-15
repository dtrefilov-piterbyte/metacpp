/****************************************************************************
* Copyright 2014-2015 Trefilov Dmitrij                                      *
*                                                                           *
* Licensed under the Apache License, Version 2.0 (the "License");           *
* you may not use this file except in compliance with the License.          *
* You may obtain a copy of the License at                                   *
*                                                                           *
*    http://www.apache.org/licenses/LICENSE-2.0                             *
*                                                                           *
* Unless required by applicable law or agreed to in writing, software       *
* distributed under the License is distributed on an "AS IS" BASIS,         *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
* See the License for the specific language governing permissions and       *
* limitations under the License.                                            *
****************************************************************************/
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

/** \brief Defines consistency policy of SqlTransaction */
enum SqlTransactionAutoCloseMode
{
    SqlTransactionAutoRollback,     /**< Transaction is rollbacked automatically if was not previously closed manually. This is the default policy. */
    SqlTransactionAutoCommit,       /**< Transaction is commited automatically if was not closed manually */
    SqlTransactionAutoCloseManual   /**< In this mode transaction should always be closed manually */
};

/** \brief Provides ACID garantees on executed sql statements.
 *
 * All sql statements is only possible to execute in a context of transaction.
 * TODO: detailed example
 *
 * Note: You should avoid recursive instantiation of SqlTransaction since
 * each instance is holding a dedicated connection to the database, which
 * is freed automatically in SqlTransaction destructor. Having several
 * SqlTransaction instances in same thread is possible due to connection pooling,
 * but this is an error-prone practice beacause at some point you may
 * run out of available connections. In this case your program will become
 * dead-locked.
*/
class SqlTransaction
{
public:
    /** \brief Constructs a new instance of SqlTransaction
     * \param autoClose defines consistency policy.
     * \param connector provides connection to database
    */
    SqlTransaction(SqlTransactionAutoCloseMode autoClose = SqlTransactionAutoRollback,
                   connectors::SqlConnectorBase *connector = connectors::SqlConnectorBase::getDefaultConnector());
    /** \brief Constructs a new instance of SqlTransaction
     * \param connectionName references connection to the database. \see SqlConnectorBase::getNamedConnector
     * \param autoClose defines consistency policy
    */
    explicit SqlTransaction(const String& connectionName,
                            SqlTransactionAutoCloseMode autoClose = SqlTransactionAutoRollback);

    virtual ~SqlTransaction();

    SqlTransaction(const SqlTransaction&)=delete;
    SqlTransaction operator=(const SqlTransaction&)=delete;

    /** \brief Returns the connector used by this transaction */
    connectors::SqlConnectorBase *connector() const;
    /** \brief Returns a connector-specific implementation */
    connectors::SqlTransactionImpl *impl() const;
    /** \brief Checks whether transaction is started */
    bool started() const;
    /** \brief Starts a transaction block
     * \throws std::runtime_error
    */
    void begin();
    /** \brief Executes all statements in current transaction block and finishes it */
    void commit();
    /** \brief Drp[s all statements in current transaction block and finishes it.
     * The database is rollbacked to it's previous consistent state */
    void rollback();
private:
    connectors::SqlConnectorBase *m_connector;
    connectors::SqlTransactionImpl *m_impl;
    SqlTransactionAutoCloseMode m_autoCloseMode;
    bool m_transactionStarted;
};


} // namespace sql
} // namespace metacpp

#endif // SQLTRANSACTION_H
