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

String SqlStorable::whereId()
{
    auto pkey = primaryKey();
    if (!pkey)
        throw std::runtime_error(std::string("Table ") + record()->metaObject()->name() +
                                 " has no primary key");
    String res = String(record()->metaObject()->name()) + "." + pkey->name() + " = ";
#define _APPEND_PKEY(type) \
    { \
        ValueEvaluator<type> eval; \
        if (pkey->nullable()) \
            res += eval(pkey->access<Nullable<type> >(record()).get()); \
        else \
            res += eval(pkey->access<type>(record())); \
    }

    switch (pkey->type())
    {
    case eFieldInt:
        _APPEND_PKEY(int32_t);
        break;
    case eFieldUint:
    case eFieldEnum:
        _APPEND_PKEY(uint32_t);
        break;
    case eFieldInt64:
        _APPEND_PKEY(int64_t);
        break;
    case eFieldUint64:
        _APPEND_PKEY(uint64_t);
        break;
    case eFieldString:
        _APPEND_PKEY(String);
        break;
    default:
        throw std::runtime_error(String("Dunno how to handle type " +
                                        String::fromValue(pkey->type())).c_str());
    }
    return res;
}

} // namespace sql
} // namespace metacpp
