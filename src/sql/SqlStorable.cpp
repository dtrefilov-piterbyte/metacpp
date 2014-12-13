#include "SqlStorable.h"
#include "Nullable.h"

namespace metacpp
{
namespace sql
{

SqlStorable::SqlStorable(Object *record)
    : m_record(record)
{
}

SqlStorable::~SqlStorable()
{
    if (m_record) delete m_record;
}

} // namespace sql
} // namespace metacpp
