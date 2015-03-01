#include "ExpressionWhereClause.h"
#include "ExpressionNode.h"

namespace metacpp {
namespace db {

namespace detail
{

ExpressionWhereClauseImplBase::ExpressionWhereClauseImplBase()
{
}

ExpressionWhereClauseImplBase::~ExpressionWhereClauseImplBase()
{
}

ExpressionWhereClauseImplRelational::ExpressionWhereClauseImplRelational(RelationOperatorType op, const ExpressionNodeBase &lhs, const ExpressionNodeBase &rhs)
    : m_operator(op), m_lhs(lhs.impl()), m_rhs(rhs.impl())
{
}

ExpressionWhereClauseImplRelational::~ExpressionWhereClauseImplRelational()
{
}

String ExpressionWhereClauseImplRelational::sqlExpression() const
{
    String lbracketed = (m_lhs->complex() ? "(" : "") +
            m_lhs->sqlExpression() +
            (m_lhs->complex() ? ")" : "");
    String rbracketed = (m_rhs->complex() ? "(" : "") +
            m_rhs->sqlExpression() +
            (m_rhs->complex() ? ")" : "");
    switch (operatorType())
    {
    case eRelationalOperatorEqual:              return lbracketed + " = "  + rbracketed;
    case eRelationalOperatorNotEqual:           return lbracketed + " <> " + rbracketed;
    case eRelationalOperatorLess:               return lbracketed + " < "  + rbracketed;
    case eRelationalOperatorLessOrEqual:        return lbracketed + " <= " + rbracketed;
    case eRelationalOperatorGreater:            return lbracketed + " > "  + rbracketed;
    case eRelationalOperatorGreaterOrEqual:     return lbracketed + " >= " + rbracketed;
    case eRelationalOperatorIsNull:             return lbracketed + " IS " + rbracketed;
    case eRelationalOperatorIsNotNull:          return lbracketed + " IS NOT " + rbracketed;
    case eRelationalOperatorLike:               return lbracketed + " LIKE " + rbracketed;
    default:
        throw std::invalid_argument("Unknown operator type");
    }
}

bool ExpressionWhereClauseImplRelational::complex() const
{
    return false;
}

WhereClauseExpressionType ExpressionWhereClauseImplRelational::type() const
{
    return eWhereClauseRelational;
}

RelationOperatorType ExpressionWhereClauseImplRelational::operatorType() const
{
    return m_operator;
}

ExpressionWhereClauseImplComplex::ExpressionWhereClauseImplComplex(ConditionalOperatorType op, const ExpressionWhereClauseImplPtr &lhs, const ExpressionWhereClauseImplPtr &rhs)
    : m_operator(op), m_lhs(lhs), m_rhs(rhs)
{
}

ExpressionWhereClauseImplComplex::~ExpressionWhereClauseImplComplex()
{
}

String ExpressionWhereClauseImplComplex::sqlExpression() const
{
    String op;
    switch (operatorType())
    {
    case eConditionalOperatorAnd: op = " AND "; break;
    case eConditionalOperatorOr: op = " OR "; break;
    default:
        throw std::invalid_argument("Unknown operator type");
    }

    return (m_lhs->complex() ? "(" : "") +
           m_lhs->sqlExpression() +
           (m_lhs->complex() ? ")" : "") +
                    op +
           (m_rhs->complex() ? "(" : "") +
           m_rhs->sqlExpression() +
           (m_rhs->complex() ? ")" : "");
}

bool ExpressionWhereClauseImplComplex::complex() const
{
    return true;
}

WhereClauseExpressionType ExpressionWhereClauseImplComplex::type() const
{
    return eWhereClauseComplex;
}

ConditionalOperatorType ExpressionWhereClauseImplComplex::operatorType() const
{
    return m_operator;
}

ExpressionWhereClauseImplLogical::ExpressionWhereClauseImplLogical(UnaryLogicalOperatorType op, const ExpressionWhereClause &inner)
    : m_operator(op), m_inner(inner.impl())
{
}

ExpressionWhereClauseImplLogical::~ExpressionWhereClauseImplLogical()
{
}

String ExpressionWhereClauseImplLogical::sqlExpression() const
{
    String bracketed = (m_inner->complex() ? "(" : "") +
            m_inner->sqlExpression() + (m_inner->complex() ? ")" : "");
    switch (operatorType())
    {
    case eLogicalOperatorNot: return "NOT " + bracketed;
    }
    throw std::invalid_argument("Unknown operator type");
}

bool ExpressionWhereClauseImplLogical::complex() const
{
    return false;
}

WhereClauseExpressionType ExpressionWhereClauseImplLogical::type() const
{
    return eWhereClauseUnaryLogical;
}

UnaryLogicalOperatorType ExpressionWhereClauseImplLogical::operatorType() const
{
    return m_operator;
}

}

ExpressionWhereClause::ExpressionWhereClause(const detail::ExpressionWhereClauseImplPtr &impl)
    : m_impl(impl)
{
}

ExpressionWhereClause::~ExpressionWhereClause()
{
}

detail::ExpressionWhereClauseImplPtr ExpressionWhereClause::impl() const
{
    return m_impl;
}

ExpressionWhereClause operator&&(const ExpressionWhereClause &lhs, const ExpressionWhereClause &rhs)
{
    return ExpressionWhereClause(std::make_shared<detail::ExpressionWhereClauseImplComplex>(eConditionalOperatorAnd, lhs.impl(), rhs.impl()));
}

ExpressionWhereClause operator||(const ExpressionWhereClause &lhs, const ExpressionWhereClause &rhs)
{
    return ExpressionWhereClause(std::make_shared<detail::ExpressionWhereClauseImplComplex>(eConditionalOperatorOr, lhs.impl(), rhs.impl()));
}

ExpressionWhereClause operator!(const ExpressionWhereClause &inner)
{
    return ExpressionWhereClause(std::make_shared<detail::ExpressionWhereClauseImplLogical>(eLogicalOperatorNot, inner));
}

// namespace detail


} // namespace db
} // namespace metacpp

