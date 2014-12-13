#include "SqlStatementBase.h"

namespace metacpp
{
namespace sql
{


SqlStatementBase::SqlStatementBase()
{

}

SqlStatementBase::~SqlStatementBase()
{

}

SqlStatementBasePtr SqlStatementFactory::createInstance(SqlStatementType type)
{
    SqlStatementBasePtr result;
    return result;
}

} // namespace sql
} // namespace metacpp
