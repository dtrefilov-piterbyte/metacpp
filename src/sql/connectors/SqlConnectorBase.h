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
#ifndef SQLCONNECTORBASE_H
#define SQLCONNECTORBASE_H
#include "SqlResultSet.h"
#include "SqlStatementImpl.h"
#include "SqlTransactionImpl.h"
#include "SqlStorable.h"
#include <atomic>
#include <map>
#include "String.h"

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
    virtual SqlTransactionImpl *createTransaction() = 0;

    virtual bool closeTransaction(SqlTransactionImpl *transaction) = 0;

    virtual SqlSyntax sqlSyntax() const = 0;

    static void setDefaultConnector(SqlConnectorBase *connector);
    static SqlConnectorBase *getDefaultConnector();

    static bool setNamedConnector(SqlConnectorBase *connector, const String& connectionName);
    static SqlConnectorBase *getNamedConnector(const String &connectionName);
private:
    static std::atomic<SqlConnectorBase *> ms_defaultConnector;
    static std::mutex ms_namedConnectorsMutex;
    static std::map<String, SqlConnectorBase *> ms_namedConnectors;
};

} // namespace connectors
} // namespace sql
} // namespace metacpp

#endif
