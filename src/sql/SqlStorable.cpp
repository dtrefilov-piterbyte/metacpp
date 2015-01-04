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

SqlStatementDelete SqlStorable::remove()
{
    return SqlStatementDelete(this);
}

SqlStatementUpdate SqlStorable::update()
{
    return SqlStatementUpdate(this);
}

bool SqlStorable::insertOne(SqlTransaction& transaction)
{
    SqlStatementInsert statement(this);
    int nRows = statement.exec(transaction);
    return nRows < 0 || nRows == 1;
}

bool SqlStorable::updateOne(SqlTransaction &transaction)
{
    SqlStatementUpdate statement(this);
    int nRows = statement.where(ExplicitWhereClauseBuilder(whereId())).exec(transaction);
    return nRows < 0 || nRows == 1;
}

bool SqlStorable::removeOne(SqlTransaction &transaction)
{
    SqlStatementDelete statement(this);
    int nRows = statement.where(ExplicitWhereClauseBuilder(whereId())).exec(transaction);
    return nRows < 0 || nRows == 1;
}

String SqlStorable::fieldValue(const MetaField *field) const
{
#define _FIELD_VAL_ARITH(type) \
    if (field->nullable()) \
    { \
        if (!field->access<Nullable<type> >(const_cast<SqlStorable *>(this)->record())) \
            return "NULL"; \
        else \
            return String::fromValue(*field->access<Nullable<type> >(const_cast<SqlStorable *>(this)->record())); \
    } \
    return String::fromValue(field->access<type>(const_cast<SqlStorable *>(this)->record()));

    switch(field->type())
    {
    case eFieldBool:
        _FIELD_VAL_ARITH(bool)
    case eFieldInt:
        _FIELD_VAL_ARITH(int32_t)
    case eFieldUint:
    case eFieldEnum:
        _FIELD_VAL_ARITH(uint32_t)
    case eFieldInt64:
        _FIELD_VAL_ARITH(int64_t)
    case eFieldUint64:
        _FIELD_VAL_ARITH(uint64_t)
    case eFieldFloat:
        _FIELD_VAL_ARITH(float)
    case eFieldDouble:
        _FIELD_VAL_ARITH(double)
    case eFieldString:
        if (field->nullable())
        {
            if (!field->access<Nullable<String> >(const_cast<SqlStorable *>(this)->record()))
                return "NULL";
            else
                return "\'" + *field->access<Nullable<String> >(const_cast<SqlStorable *>(this)->record()) + "\'";
        }
        return "\'" + field->access<String>(const_cast<SqlStorable *>(this)->record()) + "\'";
    case eFieldObject:
        throw std::runtime_error("Can store only plain objects");
    case eFieldArray:
        throw std::runtime_error("Can store only plain objects");
    case eFieldDateTime:
        if (field->nullable())
        {
            if (field->access<Nullable<DateTime> >(const_cast<SqlStorable *>(this)->record()))
                return "NULL";
            else
                return "\'" + field->access<Nullable<DateTime> >(const_cast<SqlStorable *>(this)->record()).get().toISOString() + "\'";
            return "\'" + field->access<DateTime>(const_cast<SqlStorable *>(this)->record()).toISOString() + "\'";
        }
    default:
        throw std::runtime_error("Unknown field type");
    }
}

String SqlStorable::whereId()
{
    auto pkey = primaryKey();
    if (!pkey)
        throw std::runtime_error(std::string("Table ") + record()->metaObject()->name() +
                                 " has no primary key");
    String res = String(record()->metaObject()->name()) + "." + pkey->name() + " = " +
            fieldValue(pkey);
    return res;
}

} // namespace sql
} // namespace metacpp
