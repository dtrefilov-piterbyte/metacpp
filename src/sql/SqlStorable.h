#ifndef SQLSTORABLE_H
#define SQLSTORABLE_H
#include <cstdint>
#include <memory>
#include "Object.h"

namespace metacpp
{
namespace sql
{
    /** Base interface for persistable objects */
    class SqlStorable
    {
        SqlStorable(std::unique_ptr<Object> record);
    public:
        virtual ~SqlStorable();

        virtual const MetaField *primaryKey() const = 0;
    protected:
        int64_t m_id;
        std::unique_ptr<Object> m_record;
};
} // namespace sql
} // namespace metacpp
#endif // SQLSTORABLE_H
