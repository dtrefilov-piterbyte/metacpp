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
#include "SqlExpressionTreeWalker.h"

namespace metacpp
{
namespace db
{
namespace sql
{
namespace detail
{

SqlExpressionTreeWalker::SqlExpressionTreeWalker(const db::detail::ExpressionNodeImplPtr &rootNode, 
	bool fullQualified, SqlSyntax sqlSyntax, size_t startLiteralIndex)
    : db::detail::ASTWalkerBase(rootNode), m_fullQualified(fullQualified), m_sqlSyntax(sqlSyntax),
	m_startLiteralIndex(startLiteralIndex)
{
}

SqlExpressionTreeWalker::~SqlExpressionTreeWalker()
{
}

String SqlExpressionTreeWalker::evaluate()
{
    doWalk();
    assert(m_stack.size() == 1);
    String result = m_stack.back();
    m_stack.pop_back();
    return result;
}

const VariantArray &SqlExpressionTreeWalker::literals() const
{
    return m_bindValues;
}

void SqlExpressionTreeWalker::visitColumn(std::shared_ptr<db::detail::ExpressionNodeImplColumn> column)
{
    String eval;
    if (m_fullQualified)
        eval = String() + column->metaField()->metaObject()->name() + "." + column->metaField()->name();
    else
        eval = column->metaField()->name();
    m_stack.push_back(eval);
}

void SqlExpressionTreeWalker::visitLiteral(std::shared_ptr<db::detail::ExpressionNodeImplLiteral> literal)
{
    m_bindValues.push_back(literal->value());
    if (m_fullQualified)
    {
        if (m_sqlSyntax == SqlSyntaxPostgreSQL)
            m_stack.push_back("$" + String::fromValue(m_startLiteralIndex + m_bindValues.size()));
        else
            m_stack.push_back("?");
    }
    else
    {
        String eval;
        switch (literal->type())
        {
        case eFieldDateTime:
        case eFieldString:
            eval = "\'" + variant_cast<String>(literal->value()).replace("'", "\'\'") + "\'";
            break;
        default:
            eval = variant_cast<String>(literal->value());
            break;
        }
        m_stack.push_back(eval);
    }
}

void SqlExpressionTreeWalker::visitNull(std::shared_ptr<db::detail::ExpressionNodeImplNull> null)
{
    (void)null;
    String eval = "NULL";
    m_stack.push_back(eval);
}

void SqlExpressionTreeWalker::visitUnaryOperator(std::shared_ptr<db::detail::ExpressionNodeImplUnaryOperator> unary)
{
    auto inner = unary->innerNode();
    String eval;
    switch (unary->operatorType())
    {
    case eUnaryOperatorPlus:
        eval = evaluateSubnode(inner);
        break;
    case eUnaryOperatorMinus:
        eval = "-" + evaluateSubnode(inner, !inner->isLeaf());
        break;
    case eUnaryOperatorNegation:
        eval = "~" + evaluateSubnode(inner, !inner->isLeaf());
        if (m_sqlSyntax == SqlSyntaxMySql)
        {
            switch (inner->type())
            {
            case eFieldInt:
            case eFieldUint:
                eval = "CAST(" + eval + " as INTEGER)";
                break;
            default:
                break;
            }
        }
        break;
    default:
        throw std::invalid_argument("Unknown unary operator");
    }
    m_stack.push_back(eval);
}

void SqlExpressionTreeWalker::visitBinaryOperator(std::shared_ptr<db::detail::ExpressionNodeImplBinaryOperator> binary)
{
    auto left = binary->leftNode();
    auto right = binary->rightNode();
    String l = evaluateSubnode(left, !left->isLeaf());
    String r = evaluateSubnode(right, !right->isLeaf());
    String eval;
    switch (binary->operatorType())
    {
    case eBinaryOperatorPlus:
        eval = l + " + " + r;
        break;
    case eBinaryOperatorConcatenate:
        if (m_sqlSyntax == SqlSyntaxMySql)
            eval = "CONCAT(" + l + ", " + r + ")";
        else
            eval = l + " || " + r;
        break;
    case eBinaryOperatorMinus:
        eval = l + " - " + r;
        break;
    case eBinaryOperatorMultiply:
        eval = l + " * " + r;
        break;
    case eBinaryOperatorDivide:
        if (m_sqlSyntax == SqlSyntaxMySql)
            eval = l + " DIV " + r;
        else
            eval = l + " / " + r;
        break;
    case eBinaryOperatorReminder:
        eval = l + " % " + r;
        break;
    case eBinaryOperatorAnd:
        eval = l + " & " + r;
        break;
    case eBinaryOperatorOr:
        eval = l + " | " + r;
        break;
    case eBinaryOperatorShiftLeft:
        eval = l + " << " + r;
        break;
    case eBinaryOperatorShiftRight:
        eval = l + " >> " + r;
        break;
    case eBinaryOperatorXor:
        eval = "(" + l + " & ~" + r + ") | (~" + l + " & " + r + ")";
        break;
    default:
        throw std::invalid_argument("Unknown binary operator");
    }
    m_stack.push_back(eval);
}

void SqlExpressionTreeWalker::visitFunctionCall(std::shared_ptr<db::detail::ExpressionNodeImplFunctionCall> functionCall)
{
    StringArray args = functionCall->argumentNodes().map<String>([this](const db::detail::ExpressionNodeImplPtr node) { return evaluateSubnode(node); });
    String eval = functionCall->functionName() + "(" + join(args, ", ") + ")";
    m_stack.push_back(eval);
}

void SqlExpressionTreeWalker::visitWhereClauseRelational(std::shared_ptr<db::detail::ExpressionNodeImplWhereClauseRelational> whereClauseRelational)
{
    auto left = whereClauseRelational->leftNode();
    auto right = whereClauseRelational->rightNode();
    String l = evaluateSubnode(left, !left->isLeaf());
    String r = evaluateSubnode(right, !right->isLeaf());
    String eval;
    switch (whereClauseRelational->operatorType())
    {
    case eRelationalOperatorEqual:
        eval = l + " = "  + r;
        break;
    case eRelationalOperatorNotEqual:
        eval = l + " <> " + r;
        break;
    case eRelationalOperatorLess:
        eval = l + " < "  + r;
        break;
    case eRelationalOperatorLessOrEqual:
        eval = l + " <= " + r;
        break;
    case eRelationalOperatorGreater:
        eval = l + " > "  + r;
        break;
    case eRelationalOperatorGreaterOrEqual:
        eval = l + " >= " + r;
        break;
    case eRelationalOperatorIsNull:
        eval = l + " IS NULL";
        break;
    case eRelationalOperatorIsNotNull:
        eval = l + " IS NOT NULL";
        break;
    case eRelationalOperatorLike:
        eval = l + " LIKE " + r;
        break;
    default:
        throw std::invalid_argument("Unknown relational operator type");
    }
    m_stack.push_back(eval);
}

void SqlExpressionTreeWalker::visitWhereClauseLogical(std::shared_ptr<db::detail::ExpressionNodeImplWhereClauseLogical> whereClauseLogical)
{
    auto inner = whereClauseLogical->innerNode();
    String eval;
    switch (whereClauseLogical->operatorType())
    {
    case eLogicalOperatorNot:
        eval = "NOT " + evaluateSubnode(inner, inner->complex());
        break;
    default:
        throw std::invalid_argument("Unknown logical operator type");
    }
    m_stack.push_back(eval);
}

void SqlExpressionTreeWalker::visitWhereClauseConditional(std::shared_ptr<db::detail::ExpressionNodeImplWhereClauseConditional> whereClauseConditional)
{
    String op;
    switch (whereClauseConditional->operatorType())
    {
    case eConditionalOperatorAnd: op = " AND "; break;
    case eConditionalOperatorOr: op = " OR "; break;
    default:
        throw std::invalid_argument("Unknown conditional operator type");
    }
    auto left = whereClauseConditional->leftNode();
    auto right = whereClauseConditional->rightNode();
    String l = evaluateSubnode(left, left->complex());
    String r = evaluateSubnode(right, right->complex());
    String eval = l + op + r;
    m_stack.push_back(eval);
}


String SqlExpressionTreeWalker::evaluateSubnode(const db::detail::ExpressionNodeImplPtr &node, bool bracesRequired)
{
    visitNode(node);
    String eval = m_stack.back();
    m_stack.pop_back();
    if (!bracesRequired)
        return eval;
    return "(" + eval + ")";
}

} // namespace detail
} // namespace sql
} // namespace db
} // namespace metacpp
