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
#ifndef SQLWHERECLAUSE_H
#define SQLWHERECLAUSE_H
#include <type_traits>
#include "Object.h"

namespace metacpp
{
namespace sql
{

/** \brief Basic builder for sql conditional expressions (usually in where clause) */
class WhereClauseBuilder
{
public:
    virtual ~WhereClauseBuilder() { }

    /** \brief print sql subexpression into a stream */
    virtual String expression() const = 0;
    virtual bool complex() const { return false; }
};

/** \brief Builder with prepared conditional expression */
class DirectWhereClauseBuilder : public WhereClauseBuilder
{
public:
    DirectWhereClauseBuilder(const String& s);

    ~DirectWhereClauseBuilder();

    String expression() const override;

private:
    String m_string;
};

/** Where clause compined from two others using 'and' or 'or' operators */
class ComplexWhereClauseBuilder : public DirectWhereClauseBuilder
{
public:
    enum Operator
    {
        OperatorAnd,
        OperatorOr
    };

    ComplexWhereClauseBuilder(Operator op, const WhereClauseBuilder& left,
                              const WhereClauseBuilder& right);

    bool complex() const override
    {
        return true;
    }
private:
    String buildExpression(Operator op, const WhereClauseBuilder& left,
                              const WhereClauseBuilder& right);
};

class NegationWhereClauseBuilder : public DirectWhereClauseBuilder
{
public:
    NegationWhereClauseBuilder(const WhereClauseBuilder& inner);

    String buildExpression(const WhereClauseBuilder& inner);
};

inline ComplexWhereClauseBuilder operator &&(const WhereClauseBuilder& left,
                                      const WhereClauseBuilder& right)
{
    return ComplexWhereClauseBuilder(ComplexWhereClauseBuilder::OperatorAnd, left, right);
}

inline ComplexWhereClauseBuilder operator ||(const WhereClauseBuilder& left,
                                      const WhereClauseBuilder& right)
{
    return ComplexWhereClauseBuilder(ComplexWhereClauseBuilder::OperatorOr, left, right);
}

inline NegationWhereClauseBuilder operator !(const WhereClauseBuilder& inner)
{
    return NegationWhereClauseBuilder(inner);
}

} // namespace sql
} // namespace metacpp

#endif // SQLWHERECLAUSE_H
