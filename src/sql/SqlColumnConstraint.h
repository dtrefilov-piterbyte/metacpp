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
#ifndef SQLCOLUMNCONSTRAINT_H
#define SQLCOLUMNCONSTRAINT_H
#include "SqlColumnMatcher.h"
#include <memory>

namespace metacpp
{
namespace sql
{

/** \brief Type of sql constraint
 * \relates SqlConstraintBase
*/
enum SqlConstraintType
{
    SqlConstraintTypePrimaryKey,    /**< Primary key constraint */
    SqlConstraintTypeForeignKey,    /**< Foreign key constraint */
    SqlConstraintTypeIndex,         /**< Index constraint */
    SqlConstraintTypeCheck          /**< Check constraint */
};

/** \brief Base class for all single-column constraints */
class SqlConstraintBase
{
protected:
    /** \brief Constructs a new instance of SqlConstraintBase with given column */
    template<typename TObj, typename TField>
    SqlConstraintBase(const SqlColumnMatcherFieldBase<TObj, TField>& column)
        : m_metaField(column.metaField()), m_metaObject(TObj::staticMetaObject())
    {
    }
public:

    virtual ~SqlConstraintBase();
    /** \brief Returns type of this constraint */
    virtual SqlConstraintType type() = 0;
    /** \brief Returns column meta-information associated with this constraint */
    const MetaFieldBase *metaField() const;
    /** \brief Returns table meta-information associated with this constraint */
    const MetaObject *metaObject() const;
private:
    const MetaFieldBase *m_metaField;
    const MetaObject *m_metaObject;
};

/**
  \relates SqlConstraintBase
*/
typedef std::shared_ptr<SqlConstraintBase> SqlConstraintBasePtr;

/** \brief Represents primary key column constraint
 *
 * Should be created using macro PRIMARY_KEY
 */
class SqlConstraintPrimaryKey : public SqlConstraintBase
{
public:
    /** \brief Constructs a new instance of SqlConstraintPrimaryKey with given column */
    template<typename TObj, typename TField>
    SqlConstraintPrimaryKey(const SqlColumnMatcherFieldBase<TObj, TField>& column)
        : SqlConstraintBase(column)
    {
    }

    ~SqlConstraintPrimaryKey();
    /** \brief Overrides SqlConstraintBase::type */
    SqlConstraintType type() override;
};

/** \brief Creates shared instance of SqlConstraintPrimaryKey with given column */
#define PRIMARY_KEY(column) std::make_shared<SqlConstraintPrimaryKey>(column)

/** \brief Represents foreign key column constraint
 *
 * Should be created using macro REFERENCES
 */
class SqlConstraintForeignKey : public SqlConstraintBase
{
public:
    /** \brief Constructs a new instance of SqlConstraintForeignKey with given column and refColumn */
    template<typename TObj, typename TField,
             typename TObj1, typename TField1,
             typename = typename std::enable_if<!std::is_same<TObj, TObj1>::value>::type>
    SqlConstraintForeignKey(const SqlColumnMatcherFieldBase<TObj, TField>& column,
                                  const SqlColumnMatcherFieldBase<TObj1, TField1>& refColumn) :
          SqlConstraintBase(column),
          m_refMetaField(refColumn.metaField()),
          m_refMetaObject(TObj1::staticMetaObject())
    {
    }

    ~SqlConstraintForeignKey();

    /** \brief Overrides SqlConstraintBase::type */
    SqlConstraintType type() override;

    /** \brief Returns target column meta-information associated with this constraint */
    const MetaFieldBase *referenceMetaField() const;
    /** \brief Returns target table meta-information associated with this constraint */
    const MetaObject *referenceMetaObject() const;
private:
    const MetaFieldBase *m_refMetaField;
    const MetaObject *m_refMetaObject;
};

/** \brief Creates shared instance of SqlConstraintForeignKey with given target and reference column */
#define REFERENCES(column, refColumn)  std::make_shared<SqlConstraintForeignKey>(column, refColumn)

/** \brief Represents a single column index constraint
 *
 * Should be created using macros INDEX and UNIQUE_INDEX
 * TODO: multi-column indices
 */
class SqlConstraintIndex : public SqlConstraintBase
{
public:

    /** \brief Constructs a new instance of SqlConstraintIndex with given column and uniqueness flag */
    template<typename TObj, typename TField>
    SqlConstraintIndex(const SqlColumnMatcherFieldBase<TObj, TField>& matcher, bool unique)
        : SqlConstraintBase(matcher), m_unique(unique)
    {
    }

    ~SqlConstraintIndex();

    /** \brief Overrides SqlConstraintBase::type */
    SqlConstraintType type() override;

    /** Checks whether this is index is unique */
    bool unique() const;
private:
    bool m_unique;
};

/** \brief Creates shared instance of SqlConstraintIndex with given column */
#define INDEX(column) std::make_shared<SqlConstraintIndex>(column, false)
/** \brief Creates shared instance of SqlConstraintIndex with given column and uniqueness flag */
#define UNIQUE_INDEX(column) std::make_shared<SqlConstraintIndex>(column, true)

/** \brief Represents check constraint
 *
 * Should be created using macros CHECK
 */
class SqlConstraintCheck : public SqlConstraintBase
{
public:
    /** \brief Constructs a new instance of SqlConstraintCheck with given column and check condition */
    template<typename TObj, typename TField>
    SqlConstraintCheck(const SqlColumnMatcherFieldBase<TObj, TField>& matcher,
                       const WhereClauseBuilder& check)
        : SqlConstraintBase(matcher), m_checkExpression(check.expression())
    {
    }

    ~SqlConstraintCheck();

    /** \brief Overrides SqlConstraintBase::type */
    SqlConstraintType type() override;

    /** Returns check expression in Sql form */
    String checkExpression() const;
private:
    String m_checkExpression;
};

/** \brief Creates shared instance of SqlConstraintCheck with given column and check condition */
#define CHECK(column, check) std::make_shared<SqlConstraintCheck>(column, check)

} // namespace sql
} // namespace metacpp

#endif // SQLCOLUMNCONSTRAINT_H
