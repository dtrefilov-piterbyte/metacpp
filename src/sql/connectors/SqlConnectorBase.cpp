#include "SqlConnectorBase.h"

namespace metacpp
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

    bool SqlConnectorBase::setNamedConnector(SqlConnectorBase *connector, const String& connectionName)
    {
        std::lock_guard<std::mutex> _guard(ms_namedConnectorsMutex);
        if (ms_namedConnectors.end() != ms_namedConnectors.find(connectionName))
        {
            std::cerr << "SqlConnectorBase::setNamedConnector(): connector " << connectionName
                     << " already set" << std::endl;
            return false;
        }
        ms_namedConnectors[connectionName] = connector;
        return true;
    }

    SqlConnectorBase *SqlConnectorBase::getNamedConnector(const String& connectionName)
    {
        std::lock_guard<std::mutex> _guard(ms_namedConnectorsMutex);
        auto it = ms_namedConnectors.find(connectionName);
        if (it == ms_namedConnectors.end())
            return nullptr;
        return it->second;
    }

    std::atomic<SqlConnectorBase *> SqlConnectorBase::ms_defaultConnector;
    std::mutex SqlConnectorBase::ms_namedConnectorsMutex;
    std::map<String, SqlConnectorBase *> SqlConnectorBase::ms_namedConnectors;
} // namespace connectors
} // namespace sql
} // namespace metacpp
