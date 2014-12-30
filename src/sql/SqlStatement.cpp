#include "SqlStatement.h"

namespace metacpp
{
namespace sql
{

SqlStatementBase::SqlStatementBase(SqlStorable *storable)
    : m_storable(storable)
{

}

SqlStatementBase::~SqlStatementBase()
{

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
    String res;
    String tblName = m_storable->record()->metaObject()->name();
    res = "SELECT " + tblName + ".* FROM " + tblName;
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
    throw std::runtime_error("Not implemented");
}

} // namespace sql
} // namespace metacpp
