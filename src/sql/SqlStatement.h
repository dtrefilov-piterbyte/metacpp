#ifndef SQLSTATEMENT_H
#define SQLSTATEMENT_H
#include "SqlWhereClause.h"
#include "SqlResultSet.h"
#include "SqlColumnAssignment.h"

namespace metacpp
{
namespace sql
{

// TODO: think how to implement composite statements with agregate functions, i.e.
// select city.* from city where not (select avg(age) from person where cityid = city.id) > 12;

enum SqlStatementType
{
    eSqlStatementTypeUnknown,
    eSqlStatementTypeSelect,
    eSqlStatementTypeInsert,
    eSqlStatementTypeUpdate,
    eSqlStatementTypeDelete,
};

class SqlTransaction;
class SqlStorable;

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
    SqlStatementBase(const SqlStatementBase&)=default;
    SqlStatementBase& operator=(const SqlStatementBase&)=default;
public:
    virtual ~SqlStatementBase();
    virtual SqlStatementType type() const = 0;
protected:
    virtual String buildQuery(SqlSyntax syntax) const = 0;
    String fieldValue(const MetaField *field) const;
protected:
    SqlStorable *m_storable;
    connectors::SqlStatementImpl *m_impl;
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
    Array<const MetaObject *> m_joins;
};


class SqlStatementInsert : public SqlStatementBase
{
public:
    explicit SqlStatementInsert(SqlStorable *storable);
    ~SqlStatementInsert();

    SqlStatementType type() const override;
    String buildQuery(SqlSyntax syntax) const override;

    int exec(SqlTransaction& transaction);
};

class SqlStatementUpdate : public SqlStatementBase
{
public:
    explicit SqlStatementUpdate(SqlStorable *storable);
    ~SqlStatementUpdate();

    SqlStatementType type() const override;
    String buildQuery(SqlSyntax syntax) const override;

    /** reference other tables */
    template<typename TObj>
    SqlStatementUpdate& ref()
    {
        m_joins.push_back(TObj::staticMetaObject());
        return *this;
    }

    template<typename TObj1, typename TObj2, typename... TOthers>
    SqlStatementUpdate& ref()
    {
        ref<TObj1>();
        return ref<TObj2, TOthers...>();
    }

    template<typename TObj>
    SqlStatementUpdate& set(const SqlColumnAssignmentBase<TObj>& assignment1)
    {
        m_sets.push_back(assignment1.expression());
        return *this;
    }

    template<typename TObj>
    SqlStatementUpdate& set(const SqlColumnAssignmentBase<TObj>& assignment1, const SqlColumnAssignmentBase<TObj>& assignment2)
    {
        m_sets.push_back(assignment1.expression());
        m_sets.push_back(assignment2.expression());
        return *this;
    }

    // all assignments should be performed on same table
    template<typename TObj, typename... TRest>
    typename std::enable_if<sizeof...(TRest) != 0, SqlStatementUpdate>::type&
        set(const SqlColumnAssignmentBase<TObj>& assignment1, const SqlColumnAssignmentBase<TObj>& assignment2, TRest... rest)
    {
        m_sets.push_back(assignment1.expression());
        return set(assignment2, rest...);
    }

    SqlStatementUpdate& where(const WhereClauseBuilder& whereClause);
    int exec(SqlTransaction& transaction);
private:
    Array<const MetaObject *> m_joins;
    String m_whereClause;
    StringArray m_sets;
};

class SqlStatementDelete : public SqlStatementBase
{
public:
    explicit SqlStatementDelete(SqlStorable *storable);
    ~SqlStatementDelete();

    SqlStatementType type() const override;
    String buildQuery(SqlSyntax syntax) const override;

    /** reference other tables */
    template<typename TObj>
    SqlStatementDelete& ref()
    {
        m_joins.push_back(TObj::staticMetaObject());
        return *this;
    }

    template<typename TObj1, typename TObj2, typename... TOthers>
    SqlStatementDelete& ref()
    {
        ref<TObj1>();
        return ref<TObj2, TOthers...>();
    }
    SqlStatementDelete &where(const WhereClauseBuilder& whereClause);
    int exec(SqlTransaction& transaction);
private:
    Array<const MetaObject *> m_joins;
    String m_whereClause;

};


} // namespace sql
} // namespace metacpp

#endif // SQLSTATEMENT_H
