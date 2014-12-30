#ifndef SQLSTATEMENT_H
#define SQLSTATEMENT_H
#include "SqlStorable.h"
#include "SqlWhereClause.h"
#include "SqlTransaction.h"
#include "SqlResultSet.h"
#include <list>

namespace metacpp
{
namespace sql
{

// TODO: think how to implement composite statements, i.e.
// select city.* from city where not (select avg(age) from person where cityid = city.id) > 12;

enum SqlStatementType
{
    eSqlStatementTypeUnknown,
    eSqlStatementTypeSelect,
    eSqlStatementTypeInsert,
    eSqlStatementTypeUpdate,
    eSqlStatementTypeDelete,
};

enum SqlSyntax
{
    SqlSyntaxUnknown,
    SqlSyntaxSqlite,
    SqlSyntaxPostgresql,
    SqlSyntaxMysql,
    SqlSyntaxMssql
};

/** \brief Base class for all common-types statement builders */
class SqlStatementBase
{
protected:
    explicit SqlStatementBase(SqlStorable *storable);
    SqlStatementBase(const SqlStatementBase&)=delete;
    SqlStatementBase& operator=(const SqlStatementBase&)=delete;
public:
    virtual ~SqlStatementBase();
    virtual SqlStatementType type() const = 0;
protected:
    virtual String buildQuery(SqlSyntax syntax) const = 0;
protected:
    SqlStorable *m_storable;
};

class SqlStatementSelect : public SqlStatementBase
{
public:
    // TODO: select only certain columns
    explicit SqlStatementSelect(SqlStorable *storable);
    ~SqlStatementSelect();

    SqlStatementType type() const override;

    String buildQuery(SqlSyntax syntax) const override;

    template<typename TObj>
    SqlStatementSelect& innerJoin()
    {
        if (m_joinType != JoinTypeInner && m_joinType != JoinTypeNone)
            throw std::logic_error("Invalid use: cannot mix joins");
        m_joinType = JoinTypeInner;
        m_joins.push_back(TObj::staticMetaObject());
        return *this;
    }

    template<typename TObj1, typename TObj2, typename... TOthers>
    SqlStatementSelect& innerJoin()
    {
        innerJoin<TObj1>();
        return innerJoin<TObj2, TOthers...>();
    }

    /** construct left outer join statement */
    template<typename TObj>
    SqlStatementSelect& outerJoin()
    {
        if (m_joinType != JoinTypeLeftOuter && m_joinType != JoinTypeNone)
            throw std::logic_error("Invalid use: cannot mix joins");
        m_joinType = JoinTypeLeftOuter;
        m_joins.push_back(TObj::staticMetaObject());
        return *this;
    }

    template<typename TObj1, typename TObj2, typename... TOthers>
    SqlStatementSelect& outerJoin()
    {
        outerJoin<TObj1>();
        return outerJoin<TObj2, TOthers...>();
    }


    SqlStatementSelect& limit(size_t lim);
    SqlStatementSelect& offset(size_t off);
    SqlStatementSelect& where(const WhereClauseBuilder& whereClause);

    SqlResultSet exec(SqlTransaction& transaction);
private:
    enum JoinType
    {
        JoinTypeNone,
        JoinTypeInner,
        JoinTypeLeftOuter
    };

    String m_whereClause;
    JoinType m_joinType;
    Nullable<size_t> m_limit;
    Nullable<size_t> m_offset;
    std::vector<const MetaObject *> m_joins;
};


class SqlStatementInsert : public SqlStatementBase
{
public:
    explicit SqlStatementInsert(SqlStorable *storable);
    ~SqlStatementInsert();

    SqlStatementType type() const override;

    void exec(SqlTransaction& transaction);
};

class SqlStatementUpdate : public SqlStatementBase
{
public:
    explicit SqlStatementUpdate(SqlStorable *storable);
    ~SqlStatementUpdate();

    SqlStatementType type() const override;

    /** reference other tables */
    SqlStatementUpdate& join(...);
    SqlStatementUpdate& where(const WhereClauseBuilder& whereClause);
    void exec(SqlTransaction& transaction);
};

class SqlStatementDelete : public SqlStatementBase
{
public:
    explicit SqlStatementDelete(SqlStorable *storable);
    ~SqlStatementDelete();

    /** reference other tables */
    SqlStatementUpdate& join(...);
    SqlStatementUpdate& where(const WhereClauseBuilder& whereClause);
    void exec(SqlTransaction& transaction);
};


} // namespace sql
} // namespace metacpp

#endif // SQLSTATEMENT_H
