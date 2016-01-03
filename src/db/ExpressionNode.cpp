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
#include "ExpressionNode.h"

namespace metacpp {
namespace db {

namespace detail
{

ExpressionNodeImplBase::ExpressionNodeImplBase()
{

}

ExpressionNodeImplBase::~ExpressionNodeImplBase()
{

}

ExpressionNodeImplColumn::ExpressionNodeImplColumn(const MetaFieldBase *metaField)
    : m_metaField(metaField)
{
}

ExpressionNodeImplColumn::~ExpressionNodeImplColumn()
{
}

EFieldType ExpressionNodeImplColumn::type() const
{
    return m_metaField->type();
}

ExpressionNodeType ExpressionNodeImplColumn::nodeType() const
{
    return eNodeColumn;
}

bool ExpressionNodeImplColumn::isLeaf() const
{
    return true;
}

const MetaFieldBase *ExpressionNodeImplColumn::metaField() const
{
    return m_metaField;
}

ExpressionNodeImplUnaryOperator::ExpressionNodeImplUnaryOperator(UnaryOperatorType op, ExpressionNodeImplPtr innerNode)
    : m_operator(op), m_innerNode(innerNode)
{
}

ExpressionNodeImplUnaryOperator::~ExpressionNodeImplUnaryOperator()
{
}

EFieldType ExpressionNodeImplUnaryOperator::type() const
{
    return m_innerNode->type();
}

ExpressionNodeType ExpressionNodeImplUnaryOperator::nodeType() const
{
    return eNodeUnaryOperator;
}

bool ExpressionNodeImplUnaryOperator::isLeaf() const
{
    return operatorType() == eUnaryOperatorPlus;
}

UnaryOperatorType ExpressionNodeImplUnaryOperator::operatorType() const
{
    return m_operator;
}

ExpressionNodeImplPtr ExpressionNodeImplUnaryOperator::innerNode() const
{
    return m_innerNode;
}

ExpressionNodeImplBinaryOperator::ExpressionNodeImplBinaryOperator(EFieldType type, BinaryOperatorType op,
                                                                   const ExpressionNodeImplPtr &lhs, const ExpressionNodeImplPtr &rhs)
    : m_type(type), m_operator(op), m_lhs(lhs), m_rhs(rhs)
{
}

ExpressionNodeImplBinaryOperator::~ExpressionNodeImplBinaryOperator()
{
}

EFieldType ExpressionNodeImplBinaryOperator::type() const
{
    return m_type;
}

ExpressionNodeType ExpressionNodeImplBinaryOperator::nodeType() const
{
    return eNodeBinaryOperator;
}

bool ExpressionNodeImplBinaryOperator::isLeaf() const
{
    return false;
}

BinaryOperatorType ExpressionNodeImplBinaryOperator::operatorType() const
{
    return m_operator;
}

ExpressionNodeImplPtr ExpressionNodeImplBinaryOperator::leftNode() const
{
    return m_lhs;
}

ExpressionNodeImplPtr ExpressionNodeImplBinaryOperator::rightNode() const
{
    return m_rhs;
}

ExpressionNodeImplFunctionCall::ExpressionNodeImplFunctionCall(EFieldType type, const char *funcName, std::initializer_list<ExpressionNodeImplPtr> &&argumentNodes)
    : m_type(type), m_functionName(funcName), m_argumentNodes(argumentNodes)
{
}

ExpressionNodeImplFunctionCall::~ExpressionNodeImplFunctionCall()
{
}

EFieldType ExpressionNodeImplFunctionCall::type() const
{
    return m_type;
}

ExpressionNodeType ExpressionNodeImplFunctionCall::nodeType() const
{
    return eNodeFunctionCall;
}

bool ExpressionNodeImplFunctionCall::isLeaf() const
{
    return false;
}

const String &ExpressionNodeImplFunctionCall::functionName() const
{
    return m_functionName;
}

const Array<ExpressionNodeImplPtr> &ExpressionNodeImplFunctionCall::argumentNodes() const
{
    return m_argumentNodes;
}

ExpressionNodeImplNull::ExpressionNodeImplNull(EFieldType inferType)
    : m_type(inferType)
{
}

ExpressionNodeImplNull::~ExpressionNodeImplNull()
{

}

EFieldType ExpressionNodeImplNull::type() const
{
    return m_type;
}

ExpressionNodeType ExpressionNodeImplNull::nodeType() const
{
    return eNodeNull;
}

bool ExpressionNodeImplNull::isLeaf() const
{
    return true;
}

EFieldType ExpressionNodeImplWhereClauseBase::type() const
{
    return eFieldVoid;
}

bool ExpressionNodeImplWhereClauseBase::isLeaf() const
{
    return false;
}

ExpressionNodeImplWhereClauseRelational::ExpressionNodeImplWhereClauseRelational(RelationOperatorType op, const ExpressionNodeImplPtr &lhs, const ExpressionNodeImplPtr &rhs)
    : m_operator(op), m_lhs(lhs), m_rhs(rhs)
{

}

ExpressionNodeImplWhereClauseRelational::~ExpressionNodeImplWhereClauseRelational()
{
}

bool ExpressionNodeImplWhereClauseRelational::complex() const
{
    return false;
}

ExpressionNodeType ExpressionNodeImplWhereClauseRelational::nodeType() const
{
    return eNodeWhereClauseRelational;
}

RelationOperatorType ExpressionNodeImplWhereClauseRelational::operatorType() const
{
    return m_operator;
}

ExpressionNodeImplPtr ExpressionNodeImplWhereClauseRelational::leftNode() const
{
    return m_lhs;
}

ExpressionNodeImplPtr ExpressionNodeImplWhereClauseRelational::rightNode() const
{
    return m_rhs;
}

ExpressionNodeImplWhereClauseLogical::ExpressionNodeImplWhereClauseLogical(UnaryLogicalOperatorType op, const ExpressionNodeImplWhereClausePtr &inner)
    : m_operator(op), m_inner(inner)
{

}

ExpressionNodeImplWhereClauseLogical::~ExpressionNodeImplWhereClauseLogical()
{
}

bool ExpressionNodeImplWhereClauseLogical::complex() const
{
    return false;
}

ExpressionNodeType ExpressionNodeImplWhereClauseLogical::nodeType() const
{
    return eNodeWhereClauseLogical;
}

UnaryLogicalOperatorType ExpressionNodeImplWhereClauseLogical::operatorType() const
{
    return m_operator;
}

ExpressionNodeImplWhereClausePtr ExpressionNodeImplWhereClauseLogical::innerNode() const
{
    return m_inner;
}

ExpressionNodeImplWhereClauseConditional::ExpressionNodeImplWhereClauseConditional(ConditionalOperatorType op, const ExpressionNodeImplWhereClausePtr &lhs,
                                                                           const ExpressionNodeImplWhereClausePtr &rhs)
    : m_operator(op), m_lhs(lhs), m_rhs(rhs)
{
}

ExpressionNodeImplWhereClauseConditional::~ExpressionNodeImplWhereClauseConditional()
{
}

bool ExpressionNodeImplWhereClauseConditional::complex() const
{
    return true;
}

ExpressionNodeType ExpressionNodeImplWhereClauseConditional::nodeType() const
{
    return eNodeWhereClauseComplex;
}

ConditionalOperatorType ExpressionNodeImplWhereClauseConditional::operatorType() const
{
    return m_operator;
}

ExpressionNodeImplWhereClausePtr ExpressionNodeImplWhereClauseConditional::leftNode() const
{
    return m_lhs;
}

ExpressionNodeImplWhereClausePtr ExpressionNodeImplWhereClauseConditional::rightNode() const
{
    return m_rhs;
}

ExpressionNodeImplCastOperator::ExpressionNodeImplCastOperator(EFieldType type, ExpressionNodeImplPtr innerNode)
    : m_type(type), m_innerNode(innerNode)
{
}

ExpressionNodeImplCastOperator::~ExpressionNodeImplCastOperator()
{
}

EFieldType ExpressionNodeImplCastOperator::type() const
{
    return m_type;
}

ExpressionNodeType ExpressionNodeImplCastOperator::nodeType() const
{
    return eNodeCastOperator;
}

bool ExpressionNodeImplCastOperator::isLeaf() const
{
    return false;
}

ExpressionNodeImplPtr ExpressionNodeImplCastOperator::innerNode() const
{
    return m_innerNode;
}

} // namespace detail


ExpressionNodeBase::ExpressionNodeBase(detail::ExpressionNodeImplPtr impl)
    : m_impl(impl)
{
}

ExpressionNodeBase::~ExpressionNodeBase()
{
}

detail::ExpressionNodeImplPtr ExpressionNodeBase::impl() const { return m_impl; }
bool ExpressionNodeBase::empty() const { return !m_impl; }
EFieldType ExpressionNodeBase::type() const { return impl()->type(); }
ExpressionNodeType ExpressionNodeBase::nodeType() const { return impl()->nodeType(); }
bool ExpressionNodeBase::isLeaf() const { return impl()->isLeaf(); }

ExpressionNodeWhereClause operator&&(const ExpressionNodeWhereClause &lhs, const ExpressionNodeWhereClause &rhs)
{
    return ExpressionNodeWhereClause(std::make_shared<detail::ExpressionNodeImplWhereClauseConditional>(eConditionalOperatorAnd,
        std::dynamic_pointer_cast<detail::ExpressionNodeImplWhereClauseBase>(lhs.impl()),
        std::dynamic_pointer_cast<detail::ExpressionNodeImplWhereClauseBase>(rhs.impl())));
}

ExpressionNodeWhereClause operator||(const ExpressionNodeWhereClause &lhs, const ExpressionNodeWhereClause &rhs)
{
    return ExpressionNodeWhereClause(std::make_shared<detail::ExpressionNodeImplWhereClauseConditional>(eConditionalOperatorOr,
        std::dynamic_pointer_cast<detail::ExpressionNodeImplWhereClauseBase>(lhs.impl()),
        std::dynamic_pointer_cast<detail::ExpressionNodeImplWhereClauseBase>(rhs.impl())));
}

ExpressionNodeWhereClause operator!(const ExpressionNodeWhereClause &inner)
{
    return ExpressionNodeWhereClause(std::make_shared<detail::ExpressionNodeImplWhereClauseLogical>(eLogicalOperatorNot,
                                                                                                    std::dynamic_pointer_cast<detail::ExpressionNodeImplWhereClauseBase>(inner.impl())));
}

ExpressionNodeWhereClause metacpp::db::ExpressionNode<String>::like(const String &val)
{
    return ExpressionNodeWhereClause(std::make_shared<detail::ExpressionNodeImplWhereClauseRelational>
                                     (eRelationalOperatorLike, this->impl(), std::make_shared<detail::ExpressionNodeImplLiteral>(val)));
}

ExpressionNodeWhereClause metacpp::db::ExpressionNode<String>::like(const ExpressionNode<String> &other)
{
    return ExpressionNodeWhereClause(std::make_shared<detail::ExpressionNodeImplWhereClauseRelational>
                                 (eRelationalOperatorLike, this->impl(), other.impl()));
}



} // namespace db
} // namespace metacpp

