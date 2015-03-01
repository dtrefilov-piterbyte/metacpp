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

String ExpressionNodeImplColumn::sqlExpression(bool fullQualified) const
{
    if (!fullQualified) return m_metaField->name();
    return String(m_metaField->metaObject()->name()) + "." + m_metaField->name();
}

bool ExpressionNodeImplColumn::complex() const
{
    return false;
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

String ExpressionNodeImplUnaryOperator::sqlExpression(bool fullQualified) const
{
    switch (m_operator)
    {
    case eUnaryOperatorPlus:
        return m_innerNode->sqlExpression(fullQualified);
    case eUnaryOperatorMinus:
        return "-" + ((m_innerNode->complex() ? "(" : "" ) +
                m_innerNode->sqlExpression(fullQualified) +
                (m_innerNode->complex() ? ")" : ""));
    case eUnaryOperatorNegation:
        return "~" + ((m_innerNode->complex() ? "(" : "" ) +
                m_innerNode->sqlExpression(fullQualified) +
                (m_innerNode->complex() ? ")" : ""));
    default:
        throw std::invalid_argument("Unknown unary operator");
    }
}

bool ExpressionNodeImplUnaryOperator::complex() const
{
    return operatorType() == eUnaryOperatorPlus && m_innerNode->complex();
}

UnaryOperatorType ExpressionNodeImplUnaryOperator::operatorType() const
{
    return m_operator;
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

String ExpressionNodeImplBinaryOperator::sqlExpression(bool fullQualified) const
{
    String l = (m_lhs->complex() ? "(" : "") + m_lhs->sqlExpression(fullQualified) + (m_lhs->complex() ? ")" : "");
    String r = (m_rhs->complex() ? "(" : "") + m_rhs->sqlExpression(fullQualified) + (m_rhs->complex() ? ")" : "");
    switch (operatorType())
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
    }
    throw std::invalid_argument("Unknown operator type");
}

bool ExpressionNodeImplBinaryOperator::complex() const
{
    return true;
}

BinaryOperatorType ExpressionNodeImplBinaryOperator::operatorType() const
{
    return m_operator;
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

String ExpressionNodeImplFunctionCall::sqlExpression(bool fullQualified) const
{
    StringArray args = m_argumentNodes.map<String>([fullQualified](const ExpressionNodeImplPtr& node) { return node->sqlExpression(fullQualified); });
    return m_functionName + "(" + join(args, ", ") + ")";
}

bool ExpressionNodeImplFunctionCall::complex() const
{
    return false;
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

String ExpressionNodeImplNull::sqlExpression(bool fullQualified) const
{
    (void)fullQualified;
    return "NULL";
}

bool ExpressionNodeImplNull::complex() const
{
    return false;
}

}

ExpressionNodeBase::ExpressionNodeBase(detail::ExpressionNodeImplPtr impl)
    : m_impl(impl)
{
}

detail::ExpressionNodeImplPtr ExpressionNodeBase::impl() const
{
    return m_impl;
}

ExpressionNodeBase::~ExpressionNodeBase()
{
}

// namespace detail



} // namespace db
} // namespace metacpp

