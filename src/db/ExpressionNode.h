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
#ifndef METACPP_DB_EXPRESSIONNODE_H
#define METACPP_DB_EXPRESSIONNODE_H
#include <memory>
#include "Utils.h"
#include "StringBase.h"
#include "MetaObject.h"
#include "TypePromotion.h"
#include "ExpressionAssignment.h"

namespace metacpp {
namespace db {

/** \brief Type of the AST node of a query expression */
enum ExpressionNodeType
{
    eNodeColumn,                    /**< \brief Node reffered to column */
    eNodeLiteral,                   /**< \brief SQL-literal (computed from C++ expression) */
    eNodeNull,                      /**< \brief Null-valued node */
    eNodeUnaryOperator,             /**< \brief Unary operator */
    eNodeCastOperator,              /**< \brief Type cast operator */
    eNodeBinaryOperator,            /**< \brief Binary operator */
    eNodeFunctionCall,              /**< \brief Function call */
    eNodeWhereClauseRelational,
    eNodeWhereClauseLogical,
    eNodeWhereClauseComplex
};

/** \brief Types of binary operator nodes */
enum BinaryOperatorType
{
    eBinaryOperatorPlus,        /**< \brief Addition operator */
    eBinaryOperatorConcatenate, /**< \brief String concatenation operator */
    eBinaryOperatorMinus,       /**< \brief Subtraction operator */
    eBinaryOperatorMultiply,    /**< \brief Multiplication operator */
    eBinaryOperatorDivide,      /**< \brief Division operator */
    eBinaryOperatorReminder,    /**< \brief Reminder from division operator */
    eBinaryOperatorAnd,         /**< \brief Bitwise conjunction operator */
    eBinaryOperatorOr,          /**< \brief Bitwise disjunction operator */
    eBinaryOperatorXor,         /**< \brief Bitwise exclusive disjunction operator */
    eBinaryOperatorShiftLeft,   /**< \brief Bitwise left shift operator */
    eBinaryOperatorShiftRight   /**< \brief Bitwise right shift operator */
};

/** \brief Types of unary operator nodes */
enum UnaryOperatorType
{
    eUnaryOperatorPlus,         /**< \brief Unary plus operator (has no effect) */
    eUnaryOperatorMinus,        /**< \brief Unary minus operator (taking of twos complement) */
    eUnaryOperatorNegation      /**< \brief Unary bitwise negation operator (taking of ones complement) */
};

/** \brief Types of binary relational operators used for building where clauses from two ExpressionNode objects */
enum RelationOperatorType
{
    eRelationalOperatorEqual,
    eRelationalOperatorNotEqual,
    eRelationalOperatorLess,
    eRelationalOperatorLessOrEqual,
    eRelationalOperatorGreater,
    eRelationalOperatorGreaterOrEqual,
    eRelationalOperatorLike,
    eRelationalOperatorIsNull,
    eRelationalOperatorIsNotNull
};

/** \brief Types of binary operators acting on where clause subexpressions */
enum UnaryLogicalOperatorType
{
    eLogicalOperatorNot,
};

/** \brief Types of binary operators used for combination of two ExpressionWhereClause objects */
enum ConditionalOperatorType
{
    eConditionalOperatorAnd,
    eConditionalOperatorOr
};

/** \brief Type represeting a null value in query expressions */
typedef struct
{
} null_t;

static const null_t null = null_t();

namespace detail
{
    class ExpressionNodeImplBase;
    typedef std::shared_ptr<ExpressionNodeImplBase> ExpressionNodeImplPtr;
} // namespace detail

/** \brief Base class representing query expression node */
class ExpressionNodeBase
{
protected:
    /** \brief Constructs new instance with given private implementation */
    ExpressionNodeBase(detail::ExpressionNodeImplPtr impl);
public:
    /** \brief Gets the private implementation of this node */
    detail::ExpressionNodeImplPtr impl() const;
    bool empty() const;
    virtual ~ExpressionNodeBase();

    EFieldType type() const;
    ExpressionNodeType nodeType() const;
    String sqlExpression(bool fullQualified = true) const;
    bool isLeaf() const;
private:
    detail::ExpressionNodeImplPtr m_impl;
};

namespace detail
{

class ExpressionNodeImplBase : public std::enable_shared_from_this<ExpressionNodeImplBase>
{
public:
    ExpressionNodeImplBase();
    virtual ~ExpressionNodeImplBase();
    virtual EFieldType type() const = 0;
    virtual ExpressionNodeType nodeType() const = 0;
    virtual bool isLeaf() const = 0;
};

class ExpressionNodeImplColumn : public ExpressionNodeImplBase
{
public:
    explicit ExpressionNodeImplColumn(const MetaFieldBase *metaField);
    ~ExpressionNodeImplColumn();
    EFieldType type() const override;
    ExpressionNodeType nodeType() const override;
    bool isLeaf() const override;
    const MetaFieldBase *metaField() const;
private:
    const MetaFieldBase *m_metaField;
};

class ExpressionNodeImplLiteral : public ExpressionNodeImplBase
{
public:
    template<typename T>
    explicit ExpressionNodeImplLiteral(const T& value)
        : m_value(value)
    {
    }

    ~ExpressionNodeImplLiteral()
    {
    }

    EFieldType type() const override
    {
        return m_value.type();
    }

    ExpressionNodeType nodeType() const override
    {
        return eNodeLiteral;
    }

    bool isLeaf() const override
    {
        return true;
    }

    const Variant& value() const { return m_value; }
private:
    Variant m_value;
};

class ExpressionNodeImplNull : public ExpressionNodeImplBase
{
public:
    explicit ExpressionNodeImplNull(EFieldType inferType);
    ~ExpressionNodeImplNull();

    EFieldType type() const override;
    ExpressionNodeType nodeType() const override;
    bool isLeaf() const override;
private:
    EFieldType m_type;
};

class ExpressionNodeImplUnaryOperator : public ExpressionNodeImplBase
{
public:
    ExpressionNodeImplUnaryOperator(UnaryOperatorType op, ExpressionNodeImplPtr innerNode);
    ~ExpressionNodeImplUnaryOperator();
    EFieldType type() const override;
    ExpressionNodeType nodeType() const override;
    bool isLeaf() const override;
    UnaryOperatorType operatorType() const;
    ExpressionNodeImplPtr innerNode() const;
private:
    UnaryOperatorType m_operator;
    ExpressionNodeImplPtr m_innerNode;
};

class ExpressionNodeImplCastOperator : public ExpressionNodeImplBase
{
public:
    ExpressionNodeImplCastOperator(EFieldType type, ExpressionNodeImplPtr innerNode);
    ~ExpressionNodeImplCastOperator();
    EFieldType type() const override;
    ExpressionNodeType nodeType() const override;
    bool isLeaf() const override;
    ExpressionNodeImplPtr innerNode() const;
private:
    EFieldType m_type;
    ExpressionNodeImplPtr m_innerNode;
};

class ExpressionNodeImplBinaryOperator : public ExpressionNodeImplBase
{
public:
    ExpressionNodeImplBinaryOperator(EFieldType type, BinaryOperatorType op,
                                     const ExpressionNodeImplPtr& lhs, const ExpressionNodeImplPtr& rhs);
    ~ExpressionNodeImplBinaryOperator();
    EFieldType type() const override;
    ExpressionNodeType nodeType() const override;
    bool isLeaf() const override;
    BinaryOperatorType operatorType() const;
    ExpressionNodeImplPtr leftNode() const;
    ExpressionNodeImplPtr rightNode() const;
private:
    EFieldType m_type;
    BinaryOperatorType m_operator;
    ExpressionNodeImplPtr m_lhs, m_rhs;
};

class ExpressionNodeImplFunctionCall : public ExpressionNodeImplBase
{
public:
    ExpressionNodeImplFunctionCall(EFieldType type, const char *funcName, std::initializer_list<ExpressionNodeImplPtr> &&argumentNodes);

    template<typename... TNodes>
    ExpressionNodeImplFunctionCall(EFieldType type, const char *funcName, TNodes... argumentNodes)
        : ExpressionNodeImplFunctionCall(type, funcName, { argumentNodes... })
    {
    }

    ~ExpressionNodeImplFunctionCall();

    EFieldType type() const override;
    ExpressionNodeType nodeType() const override;
    bool isLeaf() const override;
    const String& functionName() const;
    const Array<ExpressionNodeImplPtr>& argumentNodes() const;
private:
    EFieldType m_type;
    String m_functionName;
    Array<ExpressionNodeImplPtr> m_argumentNodes;
};

class ExpressionNodeImplWhereClauseBase : public ExpressionNodeImplBase
{
public:
    EFieldType type() const override;
    bool isLeaf() const override;
    virtual bool complex() const = 0;
};
typedef std::shared_ptr<ExpressionNodeImplWhereClauseBase> ExpressionNodeImplWhereClausePtr;

class ExpressionNodeImplWhereClauseRelational : public ExpressionNodeImplWhereClauseBase
{
public:
     ExpressionNodeImplWhereClauseRelational(RelationOperatorType op, const ExpressionNodeImplPtr& lhs, const ExpressionNodeImplPtr& rhs);
     ExpressionNodeImplWhereClauseRelational(const ExpressionNodeImplWhereClauseRelational&)=default;
    ~ExpressionNodeImplWhereClauseRelational();

    bool complex() const override;
    ExpressionNodeType nodeType() const override;

    RelationOperatorType operatorType() const;
    ExpressionNodeImplPtr leftNode() const;
    ExpressionNodeImplPtr rightNode() const;
private:
    RelationOperatorType m_operator;
    ExpressionNodeImplPtr m_lhs, m_rhs;
};

class ExpressionNodeImplWhereClauseLogical : public ExpressionNodeImplWhereClauseBase
{
public:
    ExpressionNodeImplWhereClauseLogical(UnaryLogicalOperatorType op, const ExpressionNodeImplWhereClausePtr& innerNode);
    ~ExpressionNodeImplWhereClauseLogical();

    bool complex() const override;
    ExpressionNodeType nodeType() const override;

    UnaryLogicalOperatorType operatorType() const;
    ExpressionNodeImplWhereClausePtr innerNode() const;
private:
    UnaryLogicalOperatorType m_operator;
    ExpressionNodeImplWhereClausePtr m_inner;
};

class ExpressionNodeImplWhereClauseConditional : public ExpressionNodeImplWhereClauseBase
{
public:
    ExpressionNodeImplWhereClauseConditional(ConditionalOperatorType op, const ExpressionNodeImplWhereClausePtr& lhs, const ExpressionNodeImplWhereClausePtr& rhs);
    ~ExpressionNodeImplWhereClauseConditional();

    bool complex() const override;
    ExpressionNodeType nodeType() const override;

    ConditionalOperatorType operatorType() const;
    ExpressionNodeImplWhereClausePtr leftNode() const;
    ExpressionNodeImplWhereClausePtr rightNode() const;
private:
    ConditionalOperatorType m_operator;
    ExpressionNodeImplWhereClausePtr m_lhs, m_rhs;
};
} // namespace detail

/** \brief Class representing final conditional expression which usually appears in where clause part of an SQL statement */
class ExpressionNodeWhereClause final : public ExpressionNodeBase
{
public:
    ExpressionNodeWhereClause()
        : ExpressionNodeBase(detail::ExpressionNodeImplPtr())
    {
    }

    /** \brief Constructs new instance of ExpressionWhereClause with given private implementation */
    explicit ExpressionNodeWhereClause(const detail::ExpressionNodeImplWhereClausePtr& impl)
        : ExpressionNodeBase(impl)
    {
    }
};

/** \brief Typed expression node */
template<typename T>
class ExpressionNode : public ExpressionNodeBase
{
public:
    typedef T Type;

    /** \brief Constructs new instance with given private implementation */
    explicit ExpressionNode(detail::ExpressionNodeImplPtr impl)
        : ExpressionNodeBase(impl)
    {
    }
    ~ExpressionNode() { }
};

/** \brief Typed query literal node */
template<typename T>
class ExpressionNodeLiteral : public ExpressionNode<T>
{
public:
    /** \brief Constructs new instance from given value */
    explicit ExpressionNodeLiteral(const T& value)
        : ExpressionNode<T>(std::make_shared<detail::ExpressionNodeImplLiteral>(value))
    {
    }
};

template<>
class ExpressionNode<String> : public ExpressionNodeBase
{
public:
    typedef String Type;

    /** \brief Constructs new instance with given private implementation */
    explicit ExpressionNode(detail::ExpressionNodeImplPtr impl)
        : ExpressionNodeBase(impl)
    {
    }
    ~ExpressionNode() { }

    ExpressionNodeWhereClause like(const String& val);
    ExpressionNodeWhereClause like(const ExpressionNode<String>& other);
};

template<typename T>
class ExpressionNodeNull : public ExpressionNode<T>
{
public:
    /** \brief Constructs new instance of null-valued node of the given infer type */
    explicit ExpressionNodeNull()
        : ExpressionNode<T>(std::make_shared<detail::ExpressionNodeImplNull>(::detail::FullFieldInfoHelper<T>::type()))
    {
    }
};

class ExpressionNodeColumnBase : public ExpressionNodeBase
{
public:
    ExpressionNodeColumnBase(const MetaFieldBase *metaField)
        : ExpressionNodeBase(std::make_shared<detail::ExpressionNodeImplColumn>(metaField))
    {

    }
};

/** \brief Partially specialized expression node reffered to a column of a table */
template<typename TObj, typename TField, typename Enable = void>
class ExpressionNodeColumnPartial;

/** \brief Partial specialization for arithmetic fields */
template<typename TObj, typename TField>
class ExpressionNodeColumnPartial<TObj, TField, typename std::enable_if<std::is_arithmetic<TField>::value>::type>
        : public ExpressionNode<TField>
{
protected:
    /** \brief Constructs new instance from given field meta-information */
    explicit ExpressionNodeColumnPartial(const MetaFieldBase *metaField)
        : ExpressionNode<TField>(std::make_shared<detail::ExpressionNodeImplColumn>(metaField))
    {
    }
public:
    const MetaFieldBase *metaField() const
    {
        return std::dynamic_pointer_cast<detail::ExpressionNodeImplColumn>(this->impl())->metaField();
    }
};

/** \brief Partial specialization for text fields */
template<typename TObj>
class ExpressionNodeColumnPartial<TObj, String>
        : public ExpressionNode<String>
{
protected:
    /** \brief Constructs new instance from given field meta-information */
    explicit ExpressionNodeColumnPartial(const MetaFieldBase *metaField)
        : ExpressionNode<String>(std::make_shared<detail::ExpressionNodeImplColumn>(metaField))
    {
    }
public:
    const MetaFieldBase *metaField() const
    {
        return std::dynamic_pointer_cast<detail::ExpressionNodeImplColumn>(this->impl())->metaField();
    }
};

/** \brief Partial specialization for timestamp fields */
template<typename TObj>
class ExpressionNodeColumnPartial<TObj, DateTime>
        : public ExpressionNode<DateTime>
{
protected:
    /** \brief Constructs new instance from given field meta-information */
    explicit ExpressionNodeColumnPartial(const MetaFieldBase *metaField)
        : ExpressionNode<DateTime>(std::make_shared<detail::ExpressionNodeImplColumn>(metaField))
    {
    }
public:
    const MetaFieldBase *metaField() const
    {
        return std::dynamic_pointer_cast<detail::ExpressionNodeImplColumn>(this->impl())->metaField();
    }
};

// TODO: Object and Array field nodes to represent mongodb queries

/** \brief Fully specialized expression node reffered to a column */
template<typename TObj, typename TField>
class ExpressionNodeColumn;

template<typename TObj, typename TField>
class ExpressionNodeColumn : public ExpressionNodeColumnPartial<TObj, TField>
{
public:
    /** \brief Constructs new instance from given field meta-information */
    explicit ExpressionNodeColumn(const MetaFieldBase *metaField)
        : ExpressionNodeColumnPartial<TObj, TField>(metaField)
    {
    }

    /** \brief Creates expression assignment of literal value */
    template<typename T>
    ExpressionAssignment<TObj, TField, T> operator=(const T& rhs)
    {
        return ExpressionAssignment<TObj, TField, T>(*this, ExpressionNodeLiteral<T>(rhs));
    }

    /** \brief Creates expression assignment of any other node */
    template<typename T>
    ExpressionAssignment<TObj, TField, T> operator=(const ExpressionNode<T>& rhs)
    {
        return ExpressionAssignment<TObj, TField, T>(*this, rhs);
    }
};

/** \brief Specialization of ExpressionNodeColumn for nullable columns */
template<typename TObj, typename TField>
class ExpressionNodeColumn<TObj, Nullable<TField> > : public ExpressionNodeColumn<TObj, TField>
{
public:
    /** \brief Constructs new instance from given field meta-information */
    explicit ExpressionNodeColumn(const MetaFieldBase *metaField)
        : ExpressionNodeColumn<TObj, TField>(metaField)
    {
    }

    ExpressionNodeWhereClause isNull() const
    {
        return ExpressionNodeWhereClause(std::make_shared<detail::ExpressionNodeImplWhereClauseRelational>
                                         (eRelationalOperatorIsNull, this->impl(),
                                          std::make_shared<detail::ExpressionNodeImplNull>(::detail::FullFieldInfoHelper<TField>::type())));
    }

    ExpressionNodeWhereClause isNotNull() const
    {
        return ExpressionNodeWhereClause(std::make_shared<detail::ExpressionNodeImplWhereClauseRelational>
                                         (eRelationalOperatorIsNotNull, this->impl(),
                                          std::make_shared<detail::ExpressionNodeImplNull>(::detail::FullFieldInfoHelper<TField>::type())));
    }

    ExpressionNodeWhereClause operator==(const null_t&) const
    {
        return this->isNull();
    }

    ExpressionNodeWhereClause operator!=(const null_t&) const
    {
        return this->isNotNull();
    }

    ExpressionAssignment<TObj, TField, TField> operator=(const null_t&)
    {
        return ExpressionAssignment<TObj, TField, TField>(*this, ExpressionNodeNull<TField>());
    }

    template<typename T>
    ExpressionAssignment<TObj, TField, T> operator=(const T& rhs)
    {
        return ExpressionAssignment<TObj, TField, T>(*this, ExpressionNodeLiteral<T>(rhs));
    }

    template<typename T>
    ExpressionAssignment<TObj, TField, T> operator=(const ExpressionNode<T>& rhs)
    {
        return ExpressionAssignment<TObj, TField, T>(*this, rhs);
    }
};

/** \brief Creates new column expression node reffered to the given member of the object
 * \relates ExpressionNodeColumn
 */
template<typename TObj, typename TField>
typename std::enable_if<std::is_base_of<metacpp::Object, TObj>::value, ExpressionNodeColumn<TObj, TField> >::type
GetExpressionNodeColumn(const TField TObj::*member)
{
    return ExpressionNodeColumn<TObj, TField>(getMetaField(member));
}

#define COL(ColumnSpec) ::metacpp::db::GetExpressionNodeColumn(&ColumnSpec)

template<typename T>
class ExpressionNodeUnaryOperator : public ExpressionNode<T>
{
public:
    ExpressionNodeUnaryOperator(UnaryOperatorType operatorType, const ExpressionNode<T>& innerNode)
        : ExpressionNode<T>(std::make_shared<detail::ExpressionNodeImplUnaryOperator>(operatorType, innerNode.impl()))
    {
    }
};

template<typename T>
class ExpressionNodeCastOperator : public ExpressionNode<T>
{
public:
    ExpressionNodeCastOperator(EFieldType type, const ExpressionNodeBase& innerNode)
        : ExpressionNode<T>(std::make_shared<detail::ExpressionNodeImplCastOperator>(type, innerNode.impl()))
    {
    }
};

template<typename T>
class ExpressionNodeBinaryOperator : public ExpressionNode<T>
{
public:
    ExpressionNodeBinaryOperator(BinaryOperatorType operatorType, const ExpressionNodeBase& lhs, const ExpressionNodeBase& rhs)
        : ExpressionNode<T>(std::make_shared<detail::ExpressionNodeImplBinaryOperator>(::detail::FullFieldInfoHelper<T>::type(),
                                                                                       operatorType, lhs.impl(), rhs.impl()))
    {

    }
};

template<typename T>
class ExpressionNodeFunctionCall : public ExpressionNode<T>
{
public:
    template<typename... TNodes>
    ExpressionNodeFunctionCall(const char *funcName, TNodes... nodes)
        : ExpressionNode<T>(std::make_shared<detail::ExpressionNodeImplFunctionCall>(::detail::FullFieldInfoHelper<T>::type(),
                                                                                     funcName, getImplHelper(nodes)...))
    {
    }
private:
    static detail::ExpressionNodeImplPtr getImplHelper(const ExpressionNodeBase& node)
    {
        return node.impl();
    }
    static detail::ExpressionNodeImplPtr getImplHelper(const null_t&)
    {
        return std::make_shared<detail::ExpressionNodeImplNull>();
    }
    template<typename TField>
    static typename std::enable_if<MayBeField<TField>::value, detail::ExpressionNodeImplPtr>::type
        getImplHelper(const TField& value)
    {
        return std::make_shared<detail::ExpressionNodeImplLiteral>(value);
    }
};

template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
ExpressionNodeUnaryOperator<T> operator+(const ExpressionNode<T>& innerNode)
{
    return ExpressionNodeUnaryOperator<T>(eUnaryOperatorPlus, innerNode);
}

template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
ExpressionNodeUnaryOperator<T> operator-(const ExpressionNode<T>& innerNode)
{
    return ExpressionNodeUnaryOperator<T>(eUnaryOperatorMinus, innerNode);
}

template<typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
ExpressionNodeUnaryOperator<T> operator~(const ExpressionNode<T>& innerNode)
{
    return ExpressionNodeUnaryOperator<T>(eUnaryOperatorNegation, innerNode);
}

namespace detail
{
template<typename T>
struct CastHelper;

template<> struct CastHelper<bool> { static const EFieldType type = eFieldBool; };
template<> struct CastHelper<int32_t> { static const EFieldType type = eFieldInt; };
template<> struct CastHelper<uint32_t> { static const EFieldType type = eFieldUint; };
template<> struct CastHelper<int64_t> { static const EFieldType type = eFieldInt64; };
template<> struct CastHelper<uint64_t> { static const EFieldType type = eFieldUint64; };
template<> struct CastHelper<float> { static const EFieldType type = eFieldFloat; };
template<> struct CastHelper<double> { static const EFieldType type = eFieldDouble; };
template<> struct CastHelper<DateTime> { static const EFieldType type = eFieldDateTime; };
template<> struct CastHelper<String> { static const EFieldType type = eFieldString; };

} // namespace detail

template<typename TRes>
ExpressionNodeCastOperator<TRes> cast(const ExpressionNodeBase& innerNode)
{
    return ExpressionNodeCastOperator<TRes>(detail::CastHelper<TRes>::type, innerNode);
}

namespace detail
{
    template<typename TField1, typename TField2,
             typename = typename std::enable_if<std::is_arithmetic<TField1>::value && std::is_arithmetic<TField2>::value>::type>
    struct BinaryOperatorHelper
    {
        typedef ExpressionNodeBinaryOperator<typename TypePromotion<TField1, TField2>::Type> TRes;

        TRes operator()(BinaryOperatorType op, const ExpressionNode<TField1>& lhs,
                                                      const ExpressionNode<TField2>& rhs)
        {
            return TRes(op, lhs, rhs);
        }
    };
} // namespace detail

#define _INST_BINARY_OPERATOR(op, opType) \
template<typename TField1, typename TField2, typename = typename std::enable_if<std::is_arithmetic<TField1>::value && std::is_arithmetic<TField2>::value>::type> \
typename detail::BinaryOperatorHelper<TField1, TField2>::TRes operator op(const ExpressionNode<TField1>& lhs, const TField2& rhs) \
{ \
    detail::BinaryOperatorHelper<TField1, TField2> helper; \
    return helper(opType, lhs, ExpressionNodeLiteral<TField2>(rhs)); \
} \
    template<typename TField1, typename TField2, typename = typename std::enable_if<std::is_arithmetic<TField1>::value && std::is_arithmetic<TField2>::value>::type> \
typename detail::BinaryOperatorHelper<TField1, TField2>::TRes operator op(const TField1& lhs, const ExpressionNode<TField2>& rhs) \
{ \
    detail::BinaryOperatorHelper<TField1, TField2> helper; \
    return helper(opType, ExpressionNodeLiteral<TField1>(lhs), rhs); \
} \
    template<typename TField1, typename TField2, typename = typename std::enable_if<std::is_arithmetic<TField1>::value && std::is_arithmetic<TField2>::value>::type> \
typename detail::BinaryOperatorHelper<TField1, TField2>::TRes operator op(const ExpressionNode<TField1>& lhs, const ExpressionNode<TField2>& rhs) \
{ \
    detail::BinaryOperatorHelper<TField1, TField2> helper; \
    return helper(opType, lhs, rhs); \
}

_INST_BINARY_OPERATOR(+, eBinaryOperatorPlus)
_INST_BINARY_OPERATOR(-, eBinaryOperatorMinus)
_INST_BINARY_OPERATOR(*, eBinaryOperatorMultiply)
_INST_BINARY_OPERATOR(/, eBinaryOperatorDivide)
_INST_BINARY_OPERATOR(%, eBinaryOperatorReminder)
_INST_BINARY_OPERATOR(&, eBinaryOperatorAnd)
_INST_BINARY_OPERATOR(|, eBinaryOperatorOr)
_INST_BINARY_OPERATOR(^, eBinaryOperatorXor)
_INST_BINARY_OPERATOR(<<, eBinaryOperatorShiftLeft)
_INST_BINARY_OPERATOR(>>, eBinaryOperatorShiftRight)

#undef _INST_BINARY_OPERATOR

inline ExpressionNodeBinaryOperator<String> operator +(const ExpressionNode<String>& lhs, const String& rhs)
{
    return ExpressionNodeBinaryOperator<String>(eBinaryOperatorConcatenate, lhs, ExpressionNodeLiteral<String>(rhs));
}

inline ExpressionNodeBinaryOperator<String> operator +(const String& lhs, const ExpressionNode<String>& rhs)
{
    return ExpressionNodeBinaryOperator<String>(eBinaryOperatorConcatenate, ExpressionNodeLiteral<String>(lhs), rhs);
}

inline ExpressionNodeBinaryOperator<String> operator +(const ExpressionNode<String>& lhs, const ExpressionNode<String>& rhs)
{
    return ExpressionNodeBinaryOperator<String>(eBinaryOperatorConcatenate, lhs, rhs);
}

ExpressionNodeWhereClause operator&&(const ExpressionNodeWhereClause& lhs, const ExpressionNodeWhereClause& rhs);
ExpressionNodeWhereClause operator||(const ExpressionNodeWhereClause& lhs, const ExpressionNodeWhereClause& rhs);
ExpressionNodeWhereClause operator!(const ExpressionNodeWhereClause& inner);

#define _INST_RELATIONAL_OPERATOR(op, opType) \
template<typename T1, typename T2> \
typename std::enable_if<std::is_convertible<T1, T2>::value, ExpressionNodeWhereClause>::type \
    operator op(const ExpressionNode<T1>& lhs, const ExpressionNode<T2>& rhs) \
{ \
    return ExpressionNodeWhereClause(std::make_shared<detail::ExpressionNodeImplWhereClauseRelational>(opType, lhs.impl(), rhs.impl())); \
} \
 \
template<typename T1, typename T2> \
typename std::enable_if<std::is_convertible<T1, T2>::value, ExpressionNodeWhereClause>::type \
    operator op(const T1& lhs, const ExpressionNode<T2>& rhs) \
{ \
    return ExpressionNodeWhereClause(std::make_shared<detail::ExpressionNodeImplWhereClauseRelational>(opType, std::make_shared<detail::ExpressionNodeImplLiteral>(lhs), \
        rhs.impl())); \
} \
 \
template<typename T1, typename T2> \
typename std::enable_if<std::is_convertible<T1, T2>::value, ExpressionNodeWhereClause>::type \
    operator op(const ExpressionNode<T1>& lhs, const T2& rhs) \
{ \
    return ExpressionNodeWhereClause(std::make_shared<detail::ExpressionNodeImplWhereClauseRelational>(opType, lhs.impl(), std::make_shared<detail::ExpressionNodeImplLiteral>(rhs))); \
}

_INST_RELATIONAL_OPERATOR(==, eRelationalOperatorEqual)
_INST_RELATIONAL_OPERATOR(!=, eRelationalOperatorNotEqual)
_INST_RELATIONAL_OPERATOR(<,  eRelationalOperatorLess)
_INST_RELATIONAL_OPERATOR(<=, eRelationalOperatorLessOrEqual)
_INST_RELATIONAL_OPERATOR(>,  eRelationalOperatorGreater)
_INST_RELATIONAL_OPERATOR(>=, eRelationalOperatorGreaterOrEqual)

#undef _INST_RELATIONAL_OPERATOR

/*** String functions ****/

inline ExpressionNodeFunctionCall<String> upper(const ExpressionNode<String>& column)
{
    return ExpressionNodeFunctionCall<String>("upper", column);
}

inline ExpressionNodeFunctionCall<String> lower(const ExpressionNode<String>& column)
{
    return ExpressionNodeFunctionCall<String>("lower", column);
}

inline ExpressionNodeFunctionCall<String> trim(const ExpressionNode<String>& column)
{
    return ExpressionNodeFunctionCall<String>("trim", column);
}

inline ExpressionNodeFunctionCall<String> ltrim(const ExpressionNode<String>& column)
{
    return ExpressionNodeFunctionCall<String>("ltrim", column);
}

inline ExpressionNodeFunctionCall<String> rtrim(const ExpressionNode<String>& column)
{
    return ExpressionNodeFunctionCall<String>("rtrim", column);
}

inline ExpressionNodeFunctionCall<uint64_t> length(const ExpressionNode<String>& column)
{
    return ExpressionNodeFunctionCall<uint64_t>("length", column);
}

/*** Arihmetic functions  ***/

template<typename TField, typename = typename std::enable_if<std::is_arithmetic<TField>::value>::type>
inline ExpressionNodeFunctionCall<TField> abs(const ExpressionNode<TField>& column)
{
    return ExpressionNodeFunctionCall<TField>("abs", column);
}

template<typename TField, typename = typename std::enable_if<std::is_floating_point<TField>::value>::type>
inline ExpressionNodeFunctionCall<TField> round(const ExpressionNode<TField>& column)
{
    return ExpressionNodeFunctionCall<TField>("round", column);
}

inline ExpressionNodeFunctionCall<int64_t> random()
{
    return ExpressionNodeFunctionCall<int64_t>("random");
}

/*** Miscellaneous functions ***/
namespace detail
{
    template<typename T, bool IsNode = std::is_base_of<ExpressionNodeBase, T>::value>
    struct TypeDeductorHelper;

    template<typename TField>
    struct TypeDeductorHelper<TField, false>
    {
        typedef typename std::remove_reference<TField>::type Type;
    };

    template<typename TNode>
    struct TypeDeductorHelper<TNode, true>
    {
        typedef typename TNode::Type Type;
    };

    /** Helper type used for retrieval of the result type of coalesce function */
    template<typename... TFields>
    struct CoalesceHelper;

    template<typename T1, typename T2>
    struct CoalesceHelper<T1, T2>
    {
        typedef typename TypePromotion<typename TypeDeductorHelper<T1>::Type, typename TypeDeductorHelper<T2>::Type>::Type TRes;
    };

    template<typename T1, typename T2, typename T3, typename... TRest>
    struct CoalesceHelper<T1, T2, T3, TRest...>
    {
        typedef typename TypePromotion<typename CoalesceHelper<T1, T2>::TRes, typename TypeDeductorHelper<T3>::Type, TRest...>::Type TRes;
    };
}

template<typename... TArgs>
ExpressionNodeFunctionCall<typename detail::CoalesceHelper<TArgs...>::TRes> coalesce(TArgs... args)
{
    return ExpressionNodeFunctionCall<typename detail::CoalesceHelper<TArgs...>::TRes>("coalesce", args...);
}

// postgresql-specific function calls
namespace postgres {
    inline ExpressionNodeFunctionCall<DateTime> now()
    {
        return ExpressionNodeFunctionCall<DateTime>("now");
    }
}

// sqlite-specific function calls
namespace sqlite {
    inline ExpressionNodeFunctionCall<String> strftime(const String& fmt, const ExpressionNode<DateTime>& dt)
    {
        return ExpressionNodeFunctionCall<String>("strftime", fmt, dt);
    }
}

} // namespace db
} // namespace metacpp

#endif // METACPP_DB_EXPRESSIONNODE_H
