#ifndef METACPP_DB_EXPRESSIONASSIGNMENT_H
#define METACPP_DB_EXPRESSIONASSIGNMENT_H
#include "StringBase.h"
#include <memory>

namespace metacpp {
namespace db {

template<typename T>
class ExpressionNode;

template<typename TObj, typename TField>
class ExpressionNodeColumn;

namespace detail
{
class ExpressionNodeImplBase;
}

/** \brief Base template class representing assignement expressions */
template<typename TObj>
class ExpressionAssignmentBase
{
protected:
    ExpressionAssignmentBase() { }
public:
    virtual ~ExpressionAssignmentBase() { }

    /** \brief Gets private implementation of the left-hand side expression */
    virtual std::shared_ptr<detail::ExpressionNodeImplBase> lhs() const = 0;
    /** \brief Gets private implementation of the right-hand side expression */
    virtual std::shared_ptr<detail::ExpressionNodeImplBase> rhs() const = 0;
};

/** \brief General implementation for assignments */
template<typename TObj, typename TField1, typename TField2>
class ExpressionAssignment : public ExpressionAssignmentBase<TObj>
{
public:
    /** \brief Constructs a new instance of SqlColumnAssignment with given left hand side and right hand side */
    ExpressionAssignment(const ExpressionNodeColumn<TObj, TField1>& lhs, const ExpressionNode<TField2>& rhs)
        : m_lhs(lhs.impl()), m_rhs(rhs.impl())
    {

    }

    ~ExpressionAssignment()
    {
    }

    /** \brief Gets private implementation of the left-hand side expression */
    std::shared_ptr<detail::ExpressionNodeImplBase> lhs() const { return m_lhs; }
    /** \brief Gets private implementation of the right-hand side expression */
    std::shared_ptr<detail::ExpressionNodeImplBase> rhs() const { return m_rhs; }
private:
    std::shared_ptr<detail::ExpressionNodeImplBase> m_lhs, m_rhs;
};

} // namespace db
} // namespace metacpp

#endif // METACPP_DB_EXPRESSIONASSIGNMENT_H
