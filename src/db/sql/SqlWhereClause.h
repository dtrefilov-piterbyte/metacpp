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
namespace db
{
namespace sql
{

/** \brief Basic builder for sql conditional expressions (usually in where clause) */
class WhereClauseBuilder
{
public:
    virtual ~WhereClauseBuilder() { }

    /** \brief print an sql string representation of this where clause */
    virtual String expression() const = 0;
    /** \brief checks whether this expression is complex, i.e. should be wraped in
     * parenthesis while embedding into more complex statements
    */
    virtual bool complex() const { return false; }
};

/** \brief Builder with prepared conditional expression */
class DirectWhereClauseBuilder : public WhereClauseBuilder
{
public:
    /** \brief Constructs new instance of DirectWhereClauseBuilder
     * \param s - final sql expression
    */
    DirectWhereClauseBuilder(const String& s);

    ~DirectWhereClauseBuilder();

    /** \brief Overriden from WhereClauseBuilder::expression */
    String expression() const override;

private:
    String m_string;
};

/** \brief Where clause compined from two others using 'and' or 'or' operators */
class ComplexWhereClauseBuilder : public DirectWhereClauseBuilder
{
public:
    enum Operator
    {
        OperatorAnd,
        OperatorOr
    };

    /** Constructs a new instance of ComplexWhereClauseBuilder
     * \param op - an operator used to combine two expressions
     * \param left - left hand side of the expression
     * \param right - right hand side of the expression
    */
    ComplexWhereClauseBuilder(Operator op, const WhereClauseBuilder& left,
                              const WhereClauseBuilder& right);

    /** \brief Overriden from WhereClauseBuilder::complex
     * \returns true
    */
    bool complex() const override
    {
        return true;
    }
private:
    String buildExpression(Operator op, const WhereClauseBuilder& left,
                              const WhereClauseBuilder& right);
};

/** \brief Represents negated where clause expression */
class NegationWhereClauseBuilder : public DirectWhereClauseBuilder
{
public:
    /** \brief Constructs new instance of NegationWhereClauseBuilder
     * \param inner - inner expression
    */
    NegationWhereClauseBuilder(const WhereClauseBuilder& inner);

    String buildExpression(const WhereClauseBuilder& inner);
};

/** \relates WhereClauseBuilder
 * Combine two where clause expressions using logical AND operator
 */
inline ComplexWhereClauseBuilder operator &&(const WhereClauseBuilder& left,
                                      const WhereClauseBuilder& right)
{
    return ComplexWhereClauseBuilder(ComplexWhereClauseBuilder::OperatorAnd, left, right);
}

/** \relates WhereClauseBuilder
 * Combine two where clause expressions using logical OR operator
 */
inline ComplexWhereClauseBuilder operator ||(const WhereClauseBuilder& left,
                                      const WhereClauseBuilder& right)
{
    return ComplexWhereClauseBuilder(ComplexWhereClauseBuilder::OperatorOr, left, right);
}

/** \relates WhereClauseBuilder
 * Negates the specified where clause expression
 */
inline NegationWhereClauseBuilder operator !(const WhereClauseBuilder& inner)
{
    return NegationWhereClauseBuilder(inner);
}

} // namespace sql
} // namespace db
} // namespace metacpp

#endif // SQLWHERECLAUSE_H
