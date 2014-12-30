#include "SqlStatement.h"

namespace metacpp
{
namespace sql
{

SqlStatementBase::SqlStatementBase(SqlStorable *storable)
    : m_storable(storable)
{

}

SqlStatementBase::~SqlStatementBase()
{

}

} // namespace sql
} // namespace metacpp
