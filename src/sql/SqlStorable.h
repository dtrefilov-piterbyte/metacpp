#ifndef SQLSTORABLE_H
#define SQLSTORABLE_H
#include <cstdint>
#include <memory>
#include "Object.h"
#include "SqlStatement.h"

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
        virtual Object *record() = 0;

        SqlStatementSelect select();
        SqlStatementDelete remove();    // delete is reserved
        SqlStatementUpdate update();

        /** Insert record into the database */
        bool insertOne(SqlTransaction& transaction);
        /** Persist changes on the record */
        bool updateOne(SqlTransaction& transaction);
        /** Delete the record */
        bool removeOne(SqlTransaction& transaction);
    private:
        String whereId();
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

        Object *record() override
        {
            return &m_record;
        }

        TObj *obj()
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
