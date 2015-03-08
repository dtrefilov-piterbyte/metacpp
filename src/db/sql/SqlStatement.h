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
#include "SqlResultSet.h"
#include "SharedDataPointer.h"
#include "ExpressionAssignment.h"
#include "ExpressionNode.h"
#include "SqlExpressionTreeWalker.h"

namespace metacpp
{
namespace db
{
namespace sql
{

// TODO: think how to implement composite statements with agregate functions, i.e.
// select city.* from city where not (select avg(age) from person where cityid = city.id) > 12;

/** \brief Type of an sql staement
 * \relates SqlStatementBase
 */
enum SqlStatementType
{
    SqlStatementTypeUnknown,   /**< Custom non-standard statement */
    SqlStatementTypeSelect,    /**< Select query */
    SqlStatementTypeInsert,    /**< Insert query */
    SqlStatementTypeUpdate,    /**< Update query */
    SqlStatementTypeDelete,    /**< Delete query */
};

class SqlTransaction;
class SqlStorable;

namespace connectors
{
    class SqlStatementImpl;
}

/** \brief Base class for all types of SQL statements */
class SqlStatementBase
{
protected:
    /** \brief Constructs a new instance of SqlStatementBase */
    SqlStatementBase();
    SqlStatementBase(const SqlStatementBase&)=default;
    SqlStatementBase& operator=(const SqlStatementBase&)=default;
public:
    virtual ~SqlStatementBase();
    /** \brief Returns type of this statement */
    virtual SqlStatementType type() const = 0;
protected:
    /** \brief Constructs a query using specified syntax */
    virtual String buildQuery(SqlSyntax syntax) const = 0;
    /** \brief Create statement implementation */
    std::shared_ptr<connectors::SqlStatementImpl> createImpl(SqlTransaction &transaction);
protected:
    std::shared_ptr<connectors::SqlStatementImpl> m_impl;
};

/** \brief Class representing Select queries */
class SqlStatementSelect : public SqlStatementBase
{
public:
    /** \brief Constructs a new instance of SqlStatementSelect */
    explicit SqlStatementSelect(SqlStorable *storable);
    ~SqlStatementSelect();

    /** \brief Overridden from SqlStatementBase::type */
    SqlStatementType type() const override;

    /** \brief Overridden from SqlStatementBase::buildQuery */
    String buildQuery(SqlSyntax syntax) const override;

    /** \brief Specifies other table to be used in inner join with this statement */
    template<typename TObj>
    SqlStatementSelect& innerJoin()
    {
        if (m_joinType != JoinTypeInner && m_joinType != JoinTypeNone)
            throw std::logic_error("Invalid use: cannot mix joins");
        m_joinType = JoinTypeInner;
        m_joins.push_back(TObj::staticMetaObject());
        return *this;
    }

    /** \brief Specifies other tables to be used in inner join with this statement */
    template<typename TObj1, typename TObj2, typename... TOthers>
    SqlStatementSelect& innerJoin()
    {
        innerJoin<TObj1>();
        return innerJoin<TObj2, TOthers...>();
    }

    /** \brief Specifies other table to be used in keft outer join with this statement */
    template<typename TObj>
    SqlStatementSelect& outerJoin()
    {
        if (m_joinType != JoinTypeLeftOuter && m_joinType != JoinTypeNone)
            throw std::logic_error("Invalid use: cannot mix joins");
        m_joinType = JoinTypeLeftOuter;
        m_joins.push_back(TObj::staticMetaObject());
        return *this;
    }

    /** \brief Specifies other tables to be used in inner join with this statement */
    template<typename TObj1, typename TObj2, typename... TOthers>
    SqlStatementSelect& outerJoin()
    {
        outerJoin<TObj1>();
        return outerJoin<TObj2, TOthers...>();
    }

    /** \brief Specifies maximum number of rows returned by execution of this statement */
    SqlStatementSelect& limit(size_t lim);
    /** \brief Specifies number of skipped rows in result set */
    SqlStatementSelect& offset(size_t off);
    /** \brief Specifies where clause for this statement */
    SqlStatementSelect& where(const ExpressionNodeWhereClause& whereClause);
    /** \brief Executes statement and returns result set */
    SqlResultSet exec(SqlTransaction& transaction);

    /** \brief Specifies columns to be used for sorting in ascending order of result set */
    template<typename TObj1, typename TField1, typename... TOthers>
    SqlStatementSelect& orderAsc(const ExpressionNodeColumn<TObj1, TField1>& column, TOthers... others)
    {
        return orderByHelper(true, column, others...);
    }

    /** \brief Specifies columns to be used for sorting in descending order of result set */
    template<typename TObj1, typename TField1, typename... TOthers>
    SqlStatementSelect& orderDesc(const ExpressionNodeColumn<TObj1, TField1>& column, TOthers... others)
    {
        return orderByHelper(false, column, others...);
    }
private:
    SqlStatementSelect& orderByHelper(bool)
    {
        return *this;
    }

    template<typename TObj1, typename TField1, typename... TOthers>
    SqlStatementSelect& orderByHelper(bool asc, const ExpressionNodeColumn<TObj1, TField1>& column, TOthers... others)
    {
        m_orderAsc = true;
        m_order.reserve(m_order.size() + 1 + sizeof...(others));
        m_order.push_back(detail::SqlExpressionTreeWalker(column.impl()).doWalk() + (asc ? " ASC" : " DESC"));
        return orderByHelper(asc, others...);
    }
private:
    enum JoinType
    {
        JoinTypeNone,
        JoinTypeInner,
        JoinTypeLeftOuter
    };

    ExpressionNodeWhereClause m_whereClause;
    JoinType m_joinType;
    Nullable<size_t> m_limit;
    Nullable<size_t> m_offset;
    Array<const MetaObject *> m_joins;
    SqlStorable *m_storable;
    StringArray m_order;
    Nullable<bool> m_orderAsc;
};


/** \brief Class representing Insert queries */
class SqlStatementInsert : public SqlStatementBase
{
public:
    /** \brief Constructs a new instance of SqlStatementInsert */
    explicit SqlStatementInsert(SqlStorable *storable);
    ~SqlStatementInsert();

    /** \brief Overridden from SqlStatementBase::type */
    SqlStatementType type() const override;
    /** \brief Overridden from SqlStatementBase::buildQuery */
    String buildQuery(SqlSyntax syntax) const override;
    /** \brief Executes statement and returns number of rows inserted (on success should be equal to 1) */
    int exec(SqlTransaction& transaction);
private:
    SqlStorable *m_storable;
};

/** \brief Class representing Update queries */
class SqlStatementUpdate : public SqlStatementBase
{
public:
    /** \brief Constructs a new instance of SqlStatementUpdate */
    explicit SqlStatementUpdate(SqlStorable *storable);
    ~SqlStatementUpdate();

    /** \brief Overridden from SqlStatementBase::type */
    SqlStatementType type() const override;
    /** \brief Overridden from SqlStatementBase::buildQuery */
    String buildQuery(SqlSyntax syntax) const override;

    /** \brief Reference other tables */
    template<typename TObj>
    SqlStatementUpdate& ref()
    {
        m_joins.push_back(TObj::staticMetaObject());
        return *this;
    }

    /** \brief Reference other tables */
    template<typename TObj1, typename TObj2, typename... TOthers>
    SqlStatementUpdate& ref()
    {
        ref<TObj1>();
        return ref<TObj2, TOthers...>();
    }

    /** \brief Specifies a set clause with a given assignment */
    template<typename TObj>
    SqlStatementUpdate& set(const ExpressionAssignmentBase<TObj>& assignment)
    {
        m_sets.push_back(detail::SqlExpressionTreeWalker(assignment.lhs(), false).doWalk() + " = " +
                         detail::SqlExpressionTreeWalker(assignment.rhs(), false).doWalk());
        return *this;
    }

    /** \brief Specifies a set clause with a given assignments */
    template<typename TObj, typename... TRest>
    SqlStatementUpdate&
        set(const ExpressionAssignmentBase<TObj>& assignment1, const ExpressionAssignmentBase<TObj>& assignment2, TRest... rest)
    {
        m_sets.push_back(detail::SqlExpressionTreeWalker(assignment1.lhs(), false).doWalk() + " = " +
                         detail::SqlExpressionTreeWalker(assignment1.rhs(), false).doWalk());
        return set(assignment2, rest...);
    }

    /** \brief Specifies a where clause */
    SqlStatementUpdate& where(const ExpressionNodeWhereClause& whereClause);
    /** \brief Executes statement using given transaction and returns number of rows updated */
    int exec(SqlTransaction& transaction);
private:
    Array<const MetaObject *> m_joins;
    ExpressionNodeWhereClause m_whereClause;
    StringArray m_sets;
    SqlStorable *m_storable;
};

/** \brief Class representing Delete queries */
class SqlStatementDelete : public SqlStatementBase
{
public:
    /** Constructs a new instance of SqlStatementDelete */
    explicit SqlStatementDelete(SqlStorable *storable);
    ~SqlStatementDelete();

    /** \brief Overridden from SqlStatementBase::type */
    SqlStatementType type() const override;
    /** \brief Overridden from SqlStatementBase::buildQuery */
    String buildQuery(SqlSyntax syntax) const override;

    /** \brief Reference other tables */
    template<typename TObj>
    SqlStatementDelete& ref()
    {
        m_joins.push_back(TObj::staticMetaObject());
        return *this;
    }

    /** \brief Reference other tables */
    template<typename TObj1, typename TObj2, typename... TOthers>
    SqlStatementDelete& ref()
    {
        ref<TObj1>();
        return ref<TObj2, TOthers...>();
    }

    /** \brief Specifies a where clause */
    SqlStatementDelete &where(const ExpressionNodeWhereClause& whereClause);
    /** \brief Executes statement using given transaction and returns number of rows deleted */
    int exec(SqlTransaction& transaction);
private:
    Array<const MetaObject *> m_joins;
    ExpressionNodeWhereClause m_whereClause;
    SqlStorable *m_storable;
};

/** \brief Class representing custom user-defined query */
class SqlStatementCustom : public SqlStatementBase
{
public:
    /** \brief Constructs a new instance of SqlStatementCustom with given queryText */
    explicit SqlStatementCustom(const String& queryText);
    ~SqlStatementCustom();

    /** \brief Overridden from SqlStatementBase::type */
    SqlStatementType type() const override;
    /** \brief Overridden from SqlStatementBase::buildQuery */
    String buildQuery(SqlSyntax syntax) const override;
    /** \brief Executes statement using given transaction */
    void exec(SqlTransaction& transaction);
private:
    String m_queryText;
};


} // namespace sql
} // namespace db
} // namespace metacpp

#endif // SQLSTATEMENT_H
