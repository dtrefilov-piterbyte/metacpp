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

SqlStatementInsert SqlStorable::insert()
{
    return SqlStatementInsert(this);
}

SqlStatementDelete SqlStorable::remove()
{
    return SqlStatementDelete(this);
}

SqlStatementUpdate SqlStorable::update()
{
    return SqlStatementUpdate(this);
}

} // namespace sql
} // namespace metacpp
