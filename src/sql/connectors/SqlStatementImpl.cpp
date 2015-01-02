#include "SqlStatementImpl.h"

namespace metacpp
{
namespace sql
{
namespace connectors
{


SqlStatementImpl::SqlStatementImpl(SqlStatementType type, const String& queryText)
    : m_prepared(false), m_queryText(queryText), m_type(type), m_done(false)
{

}

SqlStatementImpl::~SqlStatementImpl()
{

}

SqlStatementType SqlStatementImpl::type() const
{
    return m_type;
}

bool SqlStatementImpl::prepared() const
{
    return m_prepared;
}

void SqlStatementImpl::setPrepared(bool val)
{
    m_prepared = val;
}

bool SqlStatementImpl::done() const
{
    return m_done;
}

void SqlStatementImpl::setDone(bool val)
{
    m_done = val;
}

const String& SqlStatementImpl::queryText() const
{
    return m_queryText;
}

} // namespace connectors
} // namespace sql
} // namespace metacpp
