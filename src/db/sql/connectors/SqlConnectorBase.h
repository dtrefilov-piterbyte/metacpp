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
#include "StringBase.h"
#include "Uri.h"

namespace metacpp
{
namespace db
{
namespace sql
{
namespace connectors
{

class SqlConnectorBase;

/** \brief Factory type for SqlConnectorBase
 * \relates SqlConnectorBase
 */
typedef FactoryBase<std::unique_ptr<SqlConnectorBase>, const Uri&> SqlConnectorFactory;

/** \brief A base abstract class representing database connection
 *
 * Should never be used directly
 */
class SqlConnectorBase
{
protected:
    /** \brief Constructs a new instance of SqlConnectorBase */
    SqlConnectorBase();
public:
    SqlConnectorBase(const SqlConnectorBase&)=delete;
    SqlConnectorBase& operator=(const SqlConnectorBase&)=delete;

    virtual ~SqlConnectorBase();

    /** \brief Perform initial connection to the database */
    virtual bool connect() = 0;

    /** \brief Closes all connections to the database */
    virtual bool disconnect() = 0;

    /** \brief Creates a new transaction implementation using available connection to the database.
     *
     * After you finish with transaction you should return it back to the connector with closeTransaction
    */
    virtual SqlTransactionImpl *createTransaction() = 0;

    /** \brief Destroys the transaction and frees database connection which was occupied by it
     *
     * \see SqlConnectorBasse::setConnectionPooling
    */
    virtual bool closeTransaction(SqlTransactionImpl *transaction) = 0;

    /** \brief Gets type of the sql syntax accepted by this connector */
    virtual SqlSyntax sqlSyntax() const = 0;

    /** \brief Sets number of parallel connections used by this connector for parallel execution
     *
     * This method should be called before performing actual connection with SqlConnectorBase::connect
    */
    virtual void setConnectionPooling(size_t size) = 0;

    /** \brief Sets connector to be used as a default for all transactions */
    static void setDefaultConnector(SqlConnectorBase *connector);
    /** \brief Gets default connector previously set by SqlConnectorBase::setDefaultConnector */
    static SqlConnectorBase *getDefaultConnector();

    /** \brief Sets connector as a named connection */
    static bool setNamedConnector(SqlConnectorBase *connector, const String& connectionName);
    /** \brief Gets a named connection previously set by SqlConnectorBase::setNamedConnector */
    static SqlConnectorBase *getNamedConnector(const String &connectionName);

    /** \brief Registers schemaName to be used with specified factory for creation of connectors with
     * SqlConnectorBase::createConnector
    */
    static void registerConnectorFactory(const String& schemaName, std::shared_ptr<SqlConnectorFactory> factory);
    /** \brief Unregisters factory for the schemaName previously registered with SqlConnectorBase::registerConnectorFactory */
    static void unregisterConnectorFactory(const String& schemaName);

    /** \brief Creates a new connector with database connection parameters specified by uri
     *
     * \see SqlConnectorBase::registerConnectorFactory, SqlConnectorBase::unregisterConnectorFactory
    */
    static std::unique_ptr<SqlConnectorBase> createConnector(const Uri& uri);
private:
    static std::atomic<SqlConnectorBase *> ms_defaultConnector;
    static std::mutex ms_namedConnectorsMutex;
    static std::map<String, SqlConnectorBase *> ms_namedConnectors;

    static std::mutex ms_connectorFactoriesMutex;
    static std::map<String, std::shared_ptr<SqlConnectorFactory> > ms_connectorFactories;
};

} // namespace connectors
} // namespace sql
} // namespace db
} // namespace metacpp

#endif
