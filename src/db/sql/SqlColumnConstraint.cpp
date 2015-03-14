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
#include "SqlColumnConstraint.h"
#include "SqlExpressionTreeWalker.h"

namespace metacpp
{
namespace db
{
namespace sql
{

SqlConstraintBase::~SqlConstraintBase()
{

}

const MetaFieldBase *SqlConstraintBase::metaField() const
{
    return m_metaField;
}

const MetaObject *SqlConstraintBase::metaObject() const
{
    return m_metaObject;
}

SqlConstraintPrimaryKey::~SqlConstraintPrimaryKey()
{
}

SqlConstraintType SqlConstraintPrimaryKey::type()
{
    return SqlConstraintTypePrimaryKey;
}

SqlConstraintForeignKey::~SqlConstraintForeignKey()
{
}

SqlConstraintType SqlConstraintForeignKey::type()
{
    return SqlConstraintTypeForeignKey;
}

const MetaFieldBase *SqlConstraintForeignKey::referenceMetaField() const
{
    return m_refMetaField;
}

const MetaObject *SqlConstraintForeignKey::referenceMetaObject() const
{
    return m_refMetaObject;
}

SqlConstraintIndex::~SqlConstraintIndex()
{
}

SqlConstraintType SqlConstraintIndex::type()
{
    return SqlConstraintTypeIndex;
}

bool SqlConstraintIndex::unique() const
{
    return m_unique;
}

SqlConstraintCheck::~SqlConstraintCheck()
{
}

SqlConstraintType SqlConstraintCheck::type()
{
    return SqlConstraintTypeCheck;
}

String SqlConstraintCheck::checkExpression() const
{
    return detail::SqlExpressionTreeWalker(m_checkExpression.impl(), false).evaluate();
}

} // namespace sql
} // namespace db
} // namespace metacpp
