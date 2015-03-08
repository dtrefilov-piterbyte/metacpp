#include "ASTWalkerBase.h"

namespace metacpp {
namespace db {
namespace detail {

ASTWalkerBase::ASTWalkerBase(const detail::ExpressionNodeImplPtr &rootNode)
    : m_rootNode(rootNode)
{
}

ASTWalkerBase::~ASTWalkerBase()
{
}

String ASTWalkerBase::doWalk()
{
    return evaluateNode(m_rootNode);
}

String ASTWalkerBase::evaluateNode(detail::ExpressionNodeImplPtr node)
{
    switch (node->nodeType())
    {
    case eNodeColumn: return evaluateColumn(std::dynamic_pointer_cast<detail::ExpressionNodeImplColumn>(node));
    case eNodeLiteral: return evaluateLiteral(std::dynamic_pointer_cast<detail::ExpressionNodeImplLiteral>(node));
    case eNodeNull: return evaluateNull(std::dynamic_pointer_cast<detail::ExpressionNodeImplNull>(node));
    case eNodeUnaryOperator: return evaluateUnaryOperator(std::dynamic_pointer_cast<detail::ExpressionNodeImplUnaryOperator>(node));
    case eNodeBinaryOperator: return evaluateBinaryOperator(std::dynamic_pointer_cast<detail::ExpressionNodeImplBinaryOperator>(node));
    case eNodeFunctionCall: return evaluateFunctionCall(std::dynamic_pointer_cast<detail::ExpressionNodeImplFunctionCall>(node));
    case eNodeWhereClauseRelational: return evaluateWhereClauseRelational(std::dynamic_pointer_cast<detail::ExpressionNodeImplWhereClauseRelational>(node));
    case eNodeWhereClauseLogical: return evaluateWhereClauseLogical(std::dynamic_pointer_cast<detail::ExpressionNodeImplWhereClauseLogical>(node));
    case eNodeWhereClauseComplex: return evaluateWhereClauseConditional(std::dynamic_pointer_cast<detail::ExpressionNodeImplWhereClauseConditional>(node));
    default: throw std::runtime_error("Unknown node type");
    }
}

} // namespace detail
} // namespace db
} // namespace metacpp

