#include "SqlStorable.h"
#include "Nullable.h"

namespace metacpp
{
namespace sql
{

SqlStorable::SqlStorable()
{
}

SqlStorable::~SqlStorable()
{
}

SqlStatementSelect SqlStorable::select()
{
    return SqlStatementSelect(this);
}

} // namespace sql
} // namespace metacpp
