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

void ASTWalkerBase::doWalk()
{
    return visitNode(m_rootNode);
}

void ASTWalkerBase::visitNode(detail::ExpressionNodeImplPtr node)
{
    switch (node->nodeType())
    {
    case eNodeColumn: return visitColumn(std::dynamic_pointer_cast<detail::ExpressionNodeImplColumn>(node));
    case eNodeLiteral: return visitLiteral(std::dynamic_pointer_cast<detail::ExpressionNodeImplLiteral>(node));
    case eNodeNull: return visitNull(std::dynamic_pointer_cast<detail::ExpressionNodeImplNull>(node));
    case eNodeUnaryOperator: return visitUnaryOperator(std::dynamic_pointer_cast<detail::ExpressionNodeImplUnaryOperator>(node));
    case eNodeBinaryOperator: return visitBinaryOperator(std::dynamic_pointer_cast<detail::ExpressionNodeImplBinaryOperator>(node));
    case eNodeFunctionCall: return visitFunctionCall(std::dynamic_pointer_cast<detail::ExpressionNodeImplFunctionCall>(node));
    case eNodeWhereClauseRelational: return visitWhereClauseRelational(std::dynamic_pointer_cast<detail::ExpressionNodeImplWhereClauseRelational>(node));
    case eNodeWhereClauseLogical: return visitWhereClauseLogical(std::dynamic_pointer_cast<detail::ExpressionNodeImplWhereClauseLogical>(node));
    case eNodeWhereClauseComplex: return visitWhereClauseConditional(std::dynamic_pointer_cast<detail::ExpressionNodeImplWhereClauseConditional>(node));
    default: throw std::runtime_error("Unknown node type");
    }
}

void ASTWalkerBase::visitColumn(std::shared_ptr<ExpressionNodeImplColumn> column)
{
    (void)column;
}

void ASTWalkerBase::visitLiteral(std::shared_ptr<ExpressionNodeImplLiteral> literal)
{
    (void)literal;
}

void ASTWalkerBase::visitNull(std::shared_ptr<ExpressionNodeImplNull> null)
{
    (void)null;
}

void ASTWalkerBase::visitUnaryOperator(std::shared_ptr<ExpressionNodeImplUnaryOperator> unary)
{
    visitNode(unary->innerNode());
}

void ASTWalkerBase::visitBinaryOperator(std::shared_ptr<ExpressionNodeImplBinaryOperator> binary)
{
    visitNode(binary->leftNode());
    visitNode(binary->rightNode());
}

void ASTWalkerBase::visitFunctionCall(std::shared_ptr<ExpressionNodeImplFunctionCall> functionCall)
{
    for (auto node : functionCall->argumentNodes())
        visitNode(node);
}

void ASTWalkerBase::visitWhereClauseRelational(std::shared_ptr<ExpressionNodeImplWhereClauseRelational> whereClauseRelational)
{
    visitNode(whereClauseRelational->leftNode());
    visitNode(whereClauseRelational->rightNode());
}

void ASTWalkerBase::visitWhereClauseLogical(std::shared_ptr<ExpressionNodeImplWhereClauseLogical> whereClauseLogical)
{
    visitNode(whereClauseLogical->innerNode());
}

void ASTWalkerBase::visitWhereClauseConditional(std::shared_ptr<ExpressionNodeImplWhereClauseConditional> whereClauseComplex)
{
    visitNode(whereClauseComplex->leftNode());
    visitNode(whereClauseComplex->rightNode());
}

} // namespace detail
} // namespace db
} // namespace metacpp

