/****************************************************************************
* Copyright 2014-2015 Trefilov Dmitrij                                      *
*                                                                           *
* Licensed under the Apache License, Version 2.0 (the "License");           *
* you may not use this file except in compliance with the License.          *
* You may obtain a copy of the License at                                   *
*                                                                           *
*    http://www.apache.org/licenses/LICENSE-2.0                             *
*                                                                           *
* Unless required by applicable law or agreed to in writing, software       *
* distributed under the License is distributed on an "AS IS" BASIS,         *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
* See the License for the specific language governing permissions and       *
* limitations under the License.                                            *
****************************************************************************/
#ifndef SQLSTATEMENT_H
#define SQLSTATEMENT_H
#include "SqlWhereClause.h"
#include "SqlResultSet.h"
#include "SqlColumnAssignment.h"
#include "SharedDataPointer.h"
#include <memory>

namespace metacpp
{
namespace sql
{

// TODO: think how to implement composite statements with agregate functions, i.e.
// select city.* from city where not (select avg(age) from person where cityid = city.id) > 12;

enum SqlStatementType
{
    SqlStatementTypeUnknown,
    SqlStatementTypeSelect,
    SqlStatementTypeInsert,
    SqlStatementTypeUpdate,
    SqlStatementTypeDelete,
};

class SqlTransaction;
class SqlStorable;

namespace connectors
{
    class SqlStatementImpl;
}

enum SqlSyntax
{
    SqlSyntaxUnknown,
    SqlSyntaxSqlite,
    SqlSyntaxPostgreSQL,
    SqlSyntaxMysql,
    SqlSyntaxMssql
};

/** \brief Base class for all common-types statement builders */
class SqlStatementBase
{
protected:
    SqlStatementBase();
    SqlStatementBase(const SqlStatementBase&)=default;
    SqlStatementBase& operator=(const SqlStatementBase&)=default;
public:
    virtual ~SqlStatementBase();
    virtual SqlStatementType type() const = 0;
protected:
    virtual String buildQuery(SqlSyntax syntax) const = 0;
    String fieldValue(const MetaField *field) const;
    std::shared_ptr<connectors::SqlStatementImpl> createImpl(SqlTransaction &transaction);
protected:
    std::shared_ptr<connectors::SqlStatementImpl> m_impl;
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

    template<typename TObj1, typename TField1, typename... TOthers>
    SqlStatementSelect& orderAsc(const SqlColumnMatcherFieldBase<TObj1, TField1>& column, TOthers... others)
    {
        return orderByHelper(true, column, others...);
    }

    template<typename TObj1, typename TField1, typename... TOthers>
    SqlStatementSelect& orderDesc(const SqlColumnMatcherFieldBase<TObj1, TField1>& column, TOthers... others)
    {
        return orderByHelper(false, column, others...);
    }
private:
    SqlStatementSelect& orderByHelper(bool)
    {
        return *this;
    }

    template<typename TObj1, typename TField1, typename... TOthers>
    SqlStatementSelect& orderByHelper(bool asc, const SqlColumnMatcherFieldBase<TObj1, TField1>& column, TOthers... others)
    {
        if (m_orderAsc && asc != *m_orderAsc)
            throw std::runtime_error("Cannot mix order modes");
        m_orderAsc = true;
        m_order.reserve(m_order.size() + 1 + sizeof...(others));
        m_order.push_back(column.expression());
        return orderByHelper(asc, others...);
    }
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
    SqlStorable *m_storable;
    StringArray m_order;
    Nullable<bool> m_orderAsc;
};


class SqlStatementInsert : public SqlStatementBase
{
public:
    explicit SqlStatementInsert(SqlStorable *storable);
    ~SqlStatementInsert();

    SqlStatementType type() const override;
    String buildQuery(SqlSyntax syntax) const override;

    int exec(SqlTransaction& transaction);
private:
    SqlStorable *m_storable;
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
    SqlStorable *m_storable;
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
    SqlStorable *m_storable;
};

class SqlStatementCustom : public SqlStatementBase
{
public:
    explicit SqlStatementCustom(const String& queryText);
    ~SqlStatementCustom();

    SqlStatementType type() const override;
    String buildQuery(SqlSyntax syntax) const override;

    void exec(SqlTransaction& transaction);
private:
    String m_queryText;
};


} // namespace sql
} // namespace metacpp

#endif // SQLSTATEMENT_H
