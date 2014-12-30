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
    protected:
        SqlStorable();
    public:
        virtual ~SqlStorable();

        virtual const MetaField *primaryKey() const = 0;
        virtual const Object *record() const = 0;
    protected:
        int64_t m_id;
    };

    template<typename TObj, ptrdiff_t PKeyOff>
    class Storable : public SqlStorable
    {
    public:
        Storable()
        {
        }

        const MetaField *primaryKey() const override
        {
            return TObj::staticMetaObject()->fieldByOffset(PKeyOff);
        }

        const Object *record() const override
        {
            return &m_record;
        }

    private:
        TObj m_record;
    };

#define STORABLE(TObj, PKey) Storable<TObj, getMemberOffset(&TObj::PKey)>

} // namespace sql
} // namespace metacpp
#endif // SQLSTORABLE_H
