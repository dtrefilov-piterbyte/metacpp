#ifndef SQLCOLUMNASSIGNMENT_H
#define SQLCOLUMNASSIGNMENT_H
#include "String.h"
#include "Array.h"
#include "SqlColumnMatcher.h"

namespace metacpp
{
namespace sql
{

template<typename TObj>
class SqlColumnAssignmentBase
{
public:
    virtual String expression() const = 0;
};

template<typename TObj, typename TField1, typename TField2>
class SqlColumnAssignment : public SqlColumnAssignmentBase<TObj>
{
public:
    SqlColumnAssignment(const SqlColumnMatcherFieldBase<TObj, TField1>& lhs, const SqlColumnMatcherBase<TField2>& rhs)
    {
        m_expr = String(lhs.metaField()->name()) + " = " + rhs.expression();
    }

    String expression() const override
    {
        return m_expr;
    }
private:
    String m_expr;
};

} // namespace sql
} // namespace metacpp

#endif // SQLCOLUMNASSIGNMENT_H
