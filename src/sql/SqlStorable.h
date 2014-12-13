#ifndef SQLSTORABLE_H
#define SQLSTORABLE_H
#include <cstdint>
#include "Object.h"

namespace metacpp
{
namespace sql
{
    class SqlStorable
    {
        SqlStorable(Object *record);
    public:
        virtual ~SqlStorable();

        int getId() const;
        void setId();
    protected:
        Object *getRecord();
        void setRecord();
    protected:
        int64_t m_id;
        Object *m_record;
};
}
} // namespace metacpp
#endif // SQLSTORABLE_H
