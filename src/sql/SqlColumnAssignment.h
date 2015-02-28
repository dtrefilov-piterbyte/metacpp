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
#ifndef SQLCOLUMNASSIGNMENT_H
#define SQLCOLUMNASSIGNMENT_H
#include "StringBase.h"
#include "Array.h"
#include "SqlColumnMatcher.h"

namespace metacpp
{
namespace sql
{

/** \brief Base template class representing assignement expressions */
template<typename TObj>
class SqlColumnAssignmentBase
{
public:
    /** \brief Returns sql-formed expression */
    virtual String expression() const = 0;
};

/** \brief General implementation for assignments */
template<typename TObj, typename TField1, typename TField2, typename>
class SqlColumnAssignment : public SqlColumnAssignmentBase<TObj>
{
public:
    /** \brief Constructs a new instance of SqlColumnAssignment with given left hand side and right hand side */
    SqlColumnAssignment(const SqlColumnMatcherFieldBase<TObj, TField1>& lhs, const SqlColumnMatcherBase<TField2>& rhs)
    {
        m_expr = String(lhs.metaField()->name()) + " = " + rhs.expression();
    }

    /** \brief Overrides SqlColumnAssignmentBase::expression */
    String expression() const override
    {
        return m_expr;
    }
private:
    String m_expr;
};

} // namespace sql
} // namespace metacpp

#endif // SQLCOLUMNASSIGNMENT_H
