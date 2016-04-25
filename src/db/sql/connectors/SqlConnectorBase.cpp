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
#include "SqlConnectorBase.h"

namespace metacpp
{
namespace db
{
namespace sql
{
namespace connectors
{

    SqlConnectorBase::SqlConnectorBase()
    {
    }

    SqlConnectorBase::~SqlConnectorBase()
    {
    }

    void SqlConnectorBase::setDefaultConnector(SqlConnectorBase *connector)
    {
        ms_defaultConnector.store(connector);
    }

    SqlConnectorBase *SqlConnectorBase::getDefaultConnector()
    {
        return ms_defaultConnector;
    }

    void SqlConnectorBase::setNamedConnector(SqlConnectorBase *connector, const String& connectionName)
    {
        std::lock_guard<std::mutex> _guard(ms_namedConnectorsMutex);
        auto it = ms_namedConnectors.find(connectionName);
        if (connector)
        {
            if (ms_namedConnectors.end() != it)
                throw std::invalid_argument(String("Connector " + connectionName + " has already been set").c_str());
            ms_namedConnectors[connectionName] = connector;
        }
        else
            ms_namedConnectors.erase(it);
    }

    SqlConnectorBase *SqlConnectorBase::getNamedConnector(const String& connectionName)
    {
        std::lock_guard<std::mutex> _guard(ms_namedConnectorsMutex);
        auto it = ms_namedConnectors.find(connectionName);
        if (it == ms_namedConnectors.end())
            throw std::invalid_argument(String("Connector " + connectionName + " was not found").c_str());
        return it->second;
    }

    void SqlConnectorBase::registerConnectorFactory(const String &schemaName, std::shared_ptr<SqlConnectorFactory> factory)
    {
        std::lock_guard<std::mutex> _guard(ms_connectorFactoriesMutex);
        ms_connectorFactories[schemaName] = factory;
    }

    void SqlConnectorBase::unregisterConnectorFactory(const String &schemaName)
    {
        std::lock_guard<std::mutex> _guard(ms_connectorFactoriesMutex);
        ms_connectorFactories.erase(schemaName);
    }

    std::unique_ptr<SqlConnectorBase> SqlConnectorBase::createConnector(const Uri &uri)
    {
        std::lock_guard<std::mutex> _guard(ms_connectorFactoriesMutex);
        auto it = ms_connectorFactories.find(uri.schemeName());
        if (it == ms_connectorFactories.end())
        {
            throw std::invalid_argument(String("Unknown schema: " + uri.schemeName()).c_str());
        }
        return it->second->createInstance(uri);
    }

    std::atomic<SqlConnectorBase *> SqlConnectorBase::ms_defaultConnector;
    std::mutex SqlConnectorBase::ms_namedConnectorsMutex;
    std::map<String, SqlConnectorBase *> SqlConnectorBase::ms_namedConnectors;
    std::mutex SqlConnectorBase::ms_connectorFactoriesMutex;
    std::map<String, std::shared_ptr<SqlConnectorFactory> > SqlConnectorBase::ms_connectorFactories;

} // namespace connectors
} // namespace sql
} // namespace db
} // namespace metacpp
