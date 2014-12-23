#include "SqlStatementBase.h"

namespace metacpp
{
namespace sql
{


SqlStatementBase::SqlStatementBase()
    : m_prepared(false)
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

SqlStatementBasePtr SqlStatementFactory::createInstance(SqlStatementType type)
{
    SqlStatementBasePtr result;
    return result;
}

} // namespace sql
} // namespace metacpp
