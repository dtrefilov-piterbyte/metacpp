#include "SqlStatementBase.h"

namespace metacpp
{
namespace sql
{
namespace connectors
{


SqlStatementBase::SqlStatementBase(SqlStatementType type, const String& queryText)
    : m_prepared(false), m_queryText(queryText), m_type(type), m_done(false)
{

}

SqlStatementBase::~SqlStatementBase()
{

}

bool SqlStatementBase::prepared() const
{
    return m_prepared;
}

void SqlStatementBase::setPrepared(bool val)
{
    m_prepared = val;
}

bool SqlStatementBase::done() const
{
    return m_done;
}

const String& SqlStatementBase::queryText() const
{
    return m_queryText;
}

} // namespace connectors
} // namespace sql
} // namespace metacpp
