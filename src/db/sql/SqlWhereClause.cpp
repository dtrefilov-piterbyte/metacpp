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
#include "SqlWhereClause.h"

namespace metacpp
{
namespace db
{
namespace sql
{

DirectWhereClauseBuilder::DirectWhereClauseBuilder(const String &s)
    : m_string(s)
{
}

DirectWhereClauseBuilder::~DirectWhereClauseBuilder()
{
}

String DirectWhereClauseBuilder::expression() const
{
    return m_string;
}

ComplexWhereClauseBuilder::ComplexWhereClauseBuilder(ComplexWhereClauseBuilder::Operator op, const WhereClauseBuilder &left, const WhereClauseBuilder &right)
    : DirectWhereClauseBuilder(buildExpression(op, left, right))
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
    : DirectWhereClauseBuilder(buildExpression(inner))
{
}

String NegationWhereClauseBuilder::buildExpression(const WhereClauseBuilder &inner)
{
    return "NOT " + (inner.complex() ? ("(" + inner.expression() + ")") : inner.expression());
}


} // namespace sql
} // namespace db
} // namespace metacpp
