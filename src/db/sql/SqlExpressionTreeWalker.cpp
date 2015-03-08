#include "SqlExpressionTreeWalker.h"

namespace metacpp
{
namespace db
{
namespace sql
{
namespace detail
{

SqlExpressionTreeWalker::SqlExpressionTreeWalker(const db::detail::ExpressionNodeImplPtr &rootNode, bool fullQualified, SqlSyntax sqlSyntax)
    : db::detail::ASTWalkerBase(rootNode), m_fullQualified(fullQualified), m_sqlSyntax(sqlSyntax)
{
}

SqlExpressionTreeWalker::~SqlExpressionTreeWalker()
{
}

String SqlExpressionTreeWalker::evaluateColumn(std::shared_ptr<db::detail::ExpressionNodeImplColumn> column)
{
    if (m_fullQualified)
        return String() + column->metaField()->metaObject()->name() + "." + column->metaField()->name();
    return column->metaField()->name();
}

String SqlExpressionTreeWalker::evaluateLiteral(std::shared_ptr<db::detail::ExpressionNodeImplLiteral> literal)
{
    switch (literal->type())
    {
    case eFieldDateTime:
    case eFieldString:
        return "\'" + variant_cast<String>(literal->value()).replace("'", "\'\'") + "\'";
    default:
        return variant_cast<String>(literal->value());
    }
}

String SqlExpressionTreeWalker::evaluateNull(std::shared_ptr<db::detail::ExpressionNodeImplNull> null)
{
    (void)null;
    return "NULL";
}

String SqlExpressionTreeWalker::evaluateUnaryOperator(std::shared_ptr<db::detail::ExpressionNodeImplUnaryOperator> unary)
{
    auto inner = unary->innerNode();
    switch (unary->operatorType())
    {
    case eUnaryOperatorPlus:
        return evaluateNode(unary->innerNode());
    case eUnaryOperatorMinus:
        return "-" + evaluateSubnode(inner, !inner->isLeaf());
    case eUnaryOperatorNegation:
        return "~" + evaluateSubnode(inner, !inner->isLeaf());
    default:
        throw std::invalid_argument("Unknown unary operator");
    }
}

String SqlExpressionTreeWalker::evaluateBinaryOperator(std::shared_ptr<db::detail::ExpressionNodeImplBinaryOperator> binary)
{
    auto left = binary->leftNode();
    auto right = binary->rightNode();
    String l = evaluateSubnode(left, !left->isLeaf());
    String r = evaluateSubnode(right, !right->isLeaf());
    switch (binary->operatorType())
    {
    case eBinaryOperatorPlus: return l + " + " + r;
    case eBinaryOperatorConcatenate: return l + " || " + r;
    case eBinaryOperatorMinus: return l + " - " + r;
    case eBinaryOperatorMultiply: return l + " * " + r;
    case eBinaryOperatorDivide: return l + " / " + r;
    case eBinaryOperatorReminder: return l + " % " + r;
    case eBinaryOperatorAnd: return l + " & " + r;
    case eBinaryOperatorOr: return l + " | " + r;
    case eBinaryOperatorShiftLeft: return l + " << " + r;
    case eBinaryOperatorShiftRight: return l + " >> " + r;
    case eBinaryOperatorXor: return "(" + l + " & ~" + r + ") | (~" + l + " & " + r + ")";
    default:
        throw std::invalid_argument("Unknown binary operator");
    }
}

String SqlExpressionTreeWalker::evaluateFunctionCall(std::shared_ptr<db::detail::ExpressionNodeImplFunctionCall> functionCall)
{
    StringArray args = functionCall->argumentNodes().map<String>([this](const db::detail::ExpressionNodeImplPtr node) { return evaluateNode(node); });
    return functionCall->functionName() + "(" + join(args, ", ") + ")";
}

String SqlExpressionTreeWalker::evaluateWhereClauseRelational(std::shared_ptr<db::detail::ExpressionNodeImplWhereClauseRelational> whereClauseRelational)
{
    auto left = whereClauseRelational->left();
    auto right = whereClauseRelational->right();
    String l = evaluateSubnode(left, !left->isLeaf());
    String r = evaluateSubnode(right, !right->isLeaf());
    switch (whereClauseRelational->operatorType())
    {
    case eRelationalOperatorEqual:              return l + " = "  + r;
    case eRelationalOperatorNotEqual:           return l + " <> " + r;
    case eRelationalOperatorLess:               return l + " < "  + r;
    case eRelationalOperatorLessOrEqual:        return l + " <= " + r;
    case eRelationalOperatorGreater:            return l + " > "  + r;
    case eRelationalOperatorGreaterOrEqual:     return l + " >= " + r;
    case eRelationalOperatorIsNull:             return l + " IS " + r;
    case eRelationalOperatorIsNotNull:          return l + " IS NOT " + r;
    case eRelationalOperatorLike:               return l + " LIKE " + r;
    default:
        throw std::invalid_argument("Unknown relational operator type");
}
}

String SqlExpressionTreeWalker::evaluateWhereClauseLogical(std::shared_ptr<db::detail::ExpressionNodeImplWhereClauseLogical> whereClauseLogical)
{
    auto inner = whereClauseLogical->inner();
    switch (whereClauseLogical->operatorType())
    {
    case eLogicalOperatorNot: return "NOT " + evaluateSubnode(inner, inner->complex());
    }
    throw std::invalid_argument("Unknown logical operator type");
}

String SqlExpressionTreeWalker::evaluateWhereClauseConditional(std::shared_ptr<db::detail::ExpressionNodeImplWhereClauseConditional> whereClauseConditional)
{
    String op;
    switch (whereClauseConditional->operatorType())
    {
    case eConditionalOperatorAnd: op = " AND "; break;
    case eConditionalOperatorOr: op = " OR "; break;
    default:
        throw std::invalid_argument("Unknown conditional operator type");
    }
    auto left = whereClauseConditional->left();
    auto right = whereClauseConditional->right();
    String l = evaluateSubnode(left, left->complex());
    String r = evaluateSubnode(right, right->complex());
    return l + op + r;
}


String SqlExpressionTreeWalker::evaluateSubnode(const db::detail::ExpressionNodeImplPtr &node, bool bracesRequired)
{
    if (!bracesRequired) return evaluateNode(node);
    return "(" + evaluateNode(node) + ")";
}

} // namespace detail
} // namespace sql
} // namespace db
} // namespace metacpp
