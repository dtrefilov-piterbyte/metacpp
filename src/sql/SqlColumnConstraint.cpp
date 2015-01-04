#include "SqlColumnConstraint.h"

namespace metacpp
{
namespace sql
{

SqlConstraintBase::~SqlConstraintBase()
{

}

const MetaField *SqlConstraintBase::metaField() const
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

const MetaField *SqlConstraintForeignKey::referenceMetaField() const
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
    return m_checkExpression;
}

} // namespace sql
} // namespace metacpp
