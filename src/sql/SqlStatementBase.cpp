#include "SqlStatementBase.h"

namespace metacpp
{
namespace sql
{


SqlStatementBase::SqlStatementBase(const String& queryText)
    : m_prepared(false), m_queryText(queryText)
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

const String& SqlStatementBase::queryText() const
{
    return m_queryText;
}

} // namespace sql
} // namespace metacpp
