#include "SqlWhereClause.h"

namespace metacpp
{
namespace sql
{

ExplicitWhereClauseBuilder::ExplicitWhereClauseBuilder(const String &s)
    : m_string(s)
{
}

ExplicitWhereClauseBuilder::~ExplicitWhereClauseBuilder()
{
}

String ExplicitWhereClauseBuilder::expression() const
{
    return m_string;
}

ComplexWhereClauseBuilder::ComplexWhereClauseBuilder(ComplexWhereClauseBuilder::Operator op, const WhereClauseBuilder &left, const WhereClauseBuilder &right)
    : ExplicitWhereClauseBuilder(buildExpression(op, left, right))
{
}

String ComplexWhereClauseBuilder::buildExpression(ComplexWhereClauseBuilder::Operator op, const WhereClauseBuilder &left, const WhereClauseBuilder &right)
{
    String res;
    if (left.complex()) res += "("; // parenthesis
    res += left.expression();
    if (left.complex()) res += ")"; // parenthesis

    switch (op)
    {
    case OperatorAnd:
        res += " AND ";
        break;
    case OperatorOr:
        res += " OR ";
        break;
    default:
        throw std::invalid_argument("Wrong operator");
    }

    if (right.complex()) res += "("; // parenthesis
    res += right.expression();
    if (right.complex()) res += ")"; // parenthesis
    return res;
}

NegationWhereClauseBuilder::NegationWhereClauseBuilder(const WhereClauseBuilder &inner)
    : ExplicitWhereClauseBuilder(buildExpression(inner))
{
}

String NegationWhereClauseBuilder::buildExpression(const WhereClauseBuilder &inner)
{
    return "NOT " + (inner.complex() ? ("(" + inner.expression() + ")") : inner.expression());
}


} // namespace sql
} // namespace metacpp
