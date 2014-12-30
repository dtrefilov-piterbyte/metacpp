#include "SqlStorable.h"
#include "Nullable.h"

namespace metacpp
{
namespace sql
{

SqlStorable::SqlStorable(std::unique_ptr<Object> record)
    : m_record(std::move(record))
{
}

SqlStorable::~SqlStorable()
{
}

} // namespace sql
} // namespace metacpp
