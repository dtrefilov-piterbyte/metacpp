#ifndef SQLCOLUMNCONSTRAINT_H
#define SQLCOLUMNCONSTRAINT_H
#include "SqlColumnMatcher.h"
#include <memory>

namespace metacpp
{
namespace sql
{

enum SqlConstraintType
{
    SqlConstraintTypePrimaryKey,
    SqlConstraintTypeForeignKey,
    SqlConstraintTypeIndex,
    SqlConstraintTypeCheck
};

class SqlConstraintBase
{
public:
    template<typename TObj, typename TField>
    SqlConstraintBase(const SqlColumnMatcherFieldBase<TObj, TField>& matcher)
        : m_metaField(matcher.metaField()), m_metaObject(TObj::staticMetaObject())
    {

    }

    virtual ~SqlConstraintBase();
    virtual SqlConstraintType type() = 0;

    const MetaField *metaField() const;
    const MetaObject *metaObject() const;
private:
    const MetaField *m_metaField;
    const MetaObject *m_metaObject;
};

typedef std::shared_ptr<SqlConstraintBase> SqlConstraintBasePtr;

class SqlConstraintPrimaryKey : public SqlConstraintBase
{
public:
    template<typename TObj, typename TField>
    SqlConstraintPrimaryKey(const SqlColumnMatcherFieldBase<TObj, TField>& matcher)
        : SqlConstraintBase(matcher)
    {
    }

    ~SqlConstraintPrimaryKey();
    SqlConstraintType type() override;
};

#define PRIMARY_KEY(column) std::make_shared<SqlConstraintPrimaryKey>(column)

class SqlConstraintForeignKey : public SqlConstraintBase
{
public:
    template<typename TObj, typename TField,
             typename TObj1, typename TField1,
             typename = typename std::enable_if<!std::is_same<TObj, TObj1>::value>::type>
    SqlConstraintForeignKey(const SqlColumnMatcherFieldBase<TObj, TField>& matcher,
                                  const SqlColumnMatcherFieldBase<TObj1, TField1>& refMatcher) :
          SqlConstraintBase(matcher),
          m_refMetaField(refMatcher.metaField()),
          m_refMetaObject(TObj1::staticMetaObject())
    {
    }

    ~SqlConstraintForeignKey();

    SqlConstraintType type() override;

    const MetaField *referenceMetaField() const;
    const MetaObject *referenceMetaObject() const;
private:
    const MetaField *m_refMetaField;
    const MetaObject *m_refMetaObject;
};

#define REFERENCES(column, refColumn)  std::make_shared<SqlConstraintForeignKey>(column, refColumn)

// TODO: Multi-column indices
class SqlConstraintIndex : public SqlConstraintBase
{
public:
    template<typename TObj, typename TField>
    SqlConstraintIndex(const SqlColumnMatcherFieldBase<TObj, TField>& matcher, bool unique)
        : SqlConstraintBase(matcher), m_unique(unique)
    {
    }

    ~SqlConstraintIndex();

    SqlConstraintType type() override;

    bool unique() const;
private:
    bool m_unique;
};

#define INDEX(column) std::make_shared<SqlConstraintIndex>(column, false)
#define UNIQUE_INDEX(column) std::make_shared<SqlConstraintIndex>(column, true)

class SqlConstraintCheck : public SqlConstraintBase
{
public:
    template<typename TObj, typename TField>
    SqlConstraintCheck(const SqlColumnMatcherFieldBase<TObj, TField>& matcher,
                       const WhereClauseBuilder& check)
        : SqlConstraintBase(matcher), m_checkExpression(check.expression())
    {
    }

    ~SqlConstraintCheck();

    SqlConstraintType type() override;

    String checkExpression() const;
private:
    String m_checkExpression;
};

#define CHECK(column, check) std::make_shared<SqlConstraintCheck>(column, check)

} // namespace sql
} // namespace metacpp

#endif // SQLCOLUMNCONSTRAINT_H
