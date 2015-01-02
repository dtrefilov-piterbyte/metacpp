#include "SqlStatement.h"
#include "SqlTransaction.h"

namespace metacpp
{
namespace sql
{

SqlStatementBase::SqlStatementBase(SqlStorable *storable)
    : m_storable(storable), m_impl(nullptr)
{

}

SqlStatementBase::~SqlStatementBase()
{

}

String SqlStatementBase::fieldValue(const MetaField *field) const
{
    switch(field->type())
    {
    case eFieldBool:
        if (field->nullable())
        {
            if (!field->access<Nullable<bool> >(m_storable->record()))
                return "NULL";
            else
                return String::fromValue(*field->access<Nullable<bool> >(m_storable->record()));
        }
        return String::fromValue(field->access<bool>(m_storable->record()));
    case eFieldInt:
        if (field->nullable())
        {
            if (!field->access<Nullable<int32_t> >(m_storable->record()))
                return "NULL";
            else
                return String::fromValue(*field->access<Nullable<int32_t> >(m_storable->record()));
        }
        return String::fromValue(field->access<int32_t>(m_storable->record()));
    case eFieldUint:
    case eFieldEnum:
        if (field->nullable())
        {
            if (!field->access<Nullable<uint32_t> >(m_storable->record()))
            return "NULL";
            else
                return String::fromValue(*field->access<Nullable<uint32_t> >(m_storable->record()));
        }
        return String::fromValue(field->access<uint32_t>(m_storable->record()));
    case eFieldFloat:
        if (field->nullable())
        {
            if (!field->access<Nullable<float> >(m_storable->record()))
                return "NULL";
            else
                return String::fromValue(*field->access<Nullable<float> >(m_storable->record()));
        }
        return String::fromValue(field->access<float>(m_storable->record()));
    case eFieldString:
        if (field->nullable())
        {
            if (!field->access<Nullable<String> >(m_storable->record()))
                return "NULL";
            else
                return "\'" + *field->access<Nullable<String> >(m_storable->record()) + "\'";
        }
        return "\'" + field->access<String>(m_storable->record()) + "\'";
    case eFieldObject:
        throw std::runtime_error("Can store only plain objects");
    case eFieldArray:
        throw std::runtime_error("Can store only plain objects");
    case eFieldTime:
        throw std::runtime_error("TODO: need datetime");
    default:
        throw std::runtime_error("Unknown field type");
    }
}

SqlStatementSelect::SqlStatementSelect(SqlStorable *storable)
    : SqlStatementBase(storable), m_joinType(JoinTypeNone)
{

}

SqlStatementSelect::~SqlStatementSelect()
{

}

SqlStatementType SqlStatementSelect::type() const
{
    return eSqlStatementTypeSelect;
}

String SqlStatementSelect::buildQuery(SqlSyntax syntax) const
{
    (void)syntax;
    if (!m_storable->record()->metaObject()->totalFields())
        throw std::runtime_error("Invalid storable");
    String res;
    String tblName = m_storable->record()->metaObject()->name();
    StringArray columns;
    for (size_t i = 0; i < m_storable->record()->metaObject()->totalFields(); ++i)
        columns.push_back(tblName + "." + m_storable->record()->metaObject()->field(i)->name());
    res = "SELECT " + columns.join(", ") + " FROM " + tblName;
    if (m_whereClause.size())
    {
        if (m_joins.size())
        {
            switch (m_joinType)
            {
            case JoinTypeInner:
                res += " INNER JOIN ";
                break;
            case JoinTypeLeftOuter:
                res += " LEFT OUTER JOIN ";
                break;
            default:
                throw std::runtime_error("Unknown join type");
            }

            for (size_t i = 0; i < m_joins.size(); ++i)
            {
                res += m_joins[i]->name();
                if (m_joins.size() - 1 != i) res += ", ";
            }
            res += " ON " + m_whereClause;
        }
        else
        {
            res += " WHERE " + m_whereClause;
        }
    }
    if (m_limit) res += " LIMIT " + String::fromValue(*m_limit);
    if (m_offset) res += " OFFSET " + String::fromValue(*m_offset);
    return res;
}

SqlStatementSelect &SqlStatementSelect::limit(size_t lim)
{
    m_limit = lim;
    return *this;
}

SqlStatementSelect &SqlStatementSelect::offset(size_t off)
{
    m_offset = off;
    return *this;
}

SqlStatementSelect &SqlStatementSelect::where(const WhereClauseBuilder &whereClause)
{
    m_whereClause = whereClause.expression();
    return *this;
}

SqlResultSet SqlStatementSelect::exec(SqlTransaction &transaction)
{
    m_impl = transaction.impl()->createStatement(type(), buildQuery(transaction.connector()->sqlSyntax()));
    if (!transaction.impl()->prepare(m_impl))
        throw std::runtime_error("Failed to prepare statement");
    return SqlResultSet(transaction, m_impl, m_storable);
}

SqlStatementInsert::SqlStatementInsert(SqlStorable *storable)
    : SqlStatementBase(storable)
{
}

SqlStatementInsert::~SqlStatementInsert()
{
}

SqlStatementType SqlStatementInsert::type() const
{
    return eSqlStatementTypeInsert;
}

String SqlStatementInsert::buildQuery(SqlSyntax syntax) const
{
    // TODO: bind arguments
    if (!m_storable->record()->metaObject()->totalFields())
        throw std::runtime_error("Invalid storable");
    String res;
    String tblName = m_storable->record()->metaObject()->name();
    res = "INSERT INTO " + tblName;
    auto pkey = m_storable->primaryKey();
    StringArray columns, values;
    for (size_t i = 0; i < m_storable->record()->metaObject()->totalFields(); ++i)
    {
        auto field = m_storable->record()->metaObject()->field(i);
        if (field != pkey)
        {
            columns.push_back(field->name());
            values.push_back(fieldValue(field));
        }
    }
    return res += "(" + columns.join(", ") + ") VALUES (" + values.join(", ") + ")";
}

SqlStatementUpdate::SqlStatementUpdate(SqlStorable *storable)
    : SqlStatementBase(storable)
{
}

SqlStatementUpdate::~SqlStatementUpdate()
{
}

SqlStatementType SqlStatementUpdate::type() const
{
    return eSqlStatementTypeUpdate;
}

String SqlStatementUpdate::buildQuery(SqlSyntax syntax) const
{
    // TODO: bind arguments
    if (!m_storable->record()->metaObject()->totalFields())
        throw std::runtime_error("Invalid storable");
    String res;
    String tblName = m_storable->record()->metaObject()->name();
    res = "UPDATE " + tblName;
    auto pkey = m_storable->primaryKey();
    StringArray sets;
    if (!m_sets.size())
    {
        for (size_t i = 0; i < m_storable->record()->metaObject()->totalFields(); ++i)
        {
            auto field = m_storable->record()->metaObject()->field(i);
            if (field != pkey)
                sets.push_back(String(field->name()) + " = " + fieldValue(field));
        }
    }
    else
        sets = m_sets;
    res += " SET " + sets.join(", ");

    if (m_joins.size())
    {
        if (SqlSyntaxSqlite == syntax)
        {
            res += " WHERE EXISTS (SELECT 1 FROM ";
            for (size_t i = 0; i < m_joins.size(); ++i)
            {
                res += m_joins[i]->name();
                if (i != m_joins.size() - 1)
                    res += ", ";
            }
            res += " WHERE " + m_whereClause + ")";
        }
        else
            throw std::invalid_argument("Syntax unimplemented");
    }
    else
        res += " WHERE " + m_whereClause;
    return res;
}

SqlStatementUpdate &SqlStatementUpdate::where(const WhereClauseBuilder &whereClause)
{
    m_whereClause = whereClause.expression();
    return *this;
}

SqlStatementDelete::SqlStatementDelete(SqlStorable *storable)
    : SqlStatementBase(storable)
{
}

SqlStatementDelete::~SqlStatementDelete()
{
}

SqlStatementType SqlStatementDelete::type() const
{
    return eSqlStatementTypeDelete;
}

String SqlStatementDelete::buildQuery(SqlSyntax syntax) const
{
    String res;
    String tblName = m_storable->record()->metaObject()->name();
    res = "DELETE FROM " + tblName;

    if (m_joins.size())
    {
        String refs;
        for (size_t i = 0; i < m_joins.size(); ++i)
        {
            refs += m_joins[i]->name();
            if (i != m_joins.size() - 1)
                refs += ", ";
        }
        if (SqlSyntaxSqlite == syntax)
            res += " WHERE EXISTS (SELECT 1 FROM " + refs + " WHERE " + m_whereClause + ")";
        else
        {
            res += "USING " + refs + " WHERE " + m_whereClause;
        }
    }
    else
        res += " WHERE " + m_whereClause;
    return res;

}

SqlStatementDelete &SqlStatementDelete::where(const WhereClauseBuilder &whereClause)
{
    m_whereClause = whereClause.expression();
    return *this;
}

} // namespace sql
} // namespace metacpp
