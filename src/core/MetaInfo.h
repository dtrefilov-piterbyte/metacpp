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
#ifndef METAINFO_H
#define METAINFO_H
#include <string>
#include <cstddef>
#include <set>
#include <cstdint>
#include <functional>
#include <stack>
#include <mutex>
#include <ctime>
#include <memory>
#include "Array.h"
#include "StringBase.h"
#include "Nullable.h"
#include "Variant.h"
#include "DateTime.h"
#include "MetaType.h"

namespace metacpp
{
class Object;
class MetaObject;
}

#ifdef _MSC_VER
#define constexpr
#endif

/** \brief A structure describing enumeration element */
struct EnumValueInfoDescriptor
{
    const char		*m_pszValue;    /**< \brief name of the value */
    uint32_t		m_uValue;       /**< \brief value casted to uint32_t */
};

/** \brief A structure describing enumeration */
struct EnumInfoDescriptor
{
    EEnumType		m_type;                                 /**< \brief Type of enumeration */
    const char		*m_enumName;                            /**< \brief Name of enumeration */
    uint32_t		m_defaultValue;                         /**< \brief Default value for this enum */
    const EnumValueInfoDescriptor *m_valueDescriptors;      /**< \brief Pointer to the array of value descriptors (terminated with dummy descriptor) */
};

/** \brief A structure describing the reflection property of the class */
struct FieldInfoDescriptor
{
    const char *m_pszName;  /**< \brief field name */
    size_t		m_dwSize;   /**< \brief size of field in bytes */
    ptrdiff_t	m_dwOffset; /**< \brief offset of field in class */
    EFieldType	m_eType;    /**< \brief type of the field */
    bool        m_nullable; /**< \brief if this flag is set, field is in form Nullable<T> */

    /** \brief Type-specific extension */
	struct Extension
	{
        union UExtension
        {
            struct
			{
				const char	*defaultValue;
            } m_string;
            struct
			{
				bool		defaultValue;
            } m_bool;
            struct
			{
				int32_t		defaultValue;
            } m_int;
            struct
			{
                uint32_t	defaultValue;
            } m_uint;
            struct
            {
                int64_t     defaultValue;
            } m_int64;
            struct
            {
                uint64_t    defaultValue;
            } m_uint64;
            struct
			{
				float		defaultValue;
            } m_float;
            struct
            {
                double      defaultValue;
            } m_double;
            struct
			{
                const EnumInfoDescriptor *enumInfo;
            } m_enum;
            struct
			{
                EFieldType      elemType;
                size_t          elemSize;
            } m_array;
            struct
            {
                time_t defaultValue;
            } m_datetime;
            struct
            {
                const metacpp::MetaObject *metaObject;
            } m_obj;
        } ext;
        EMandatoriness	mandatoriness;  /**< \brief Mandatoriness of the field */

        /** \brief Constructs default Extension */
        explicit Extension()
		{
            mandatoriness = eOptional;
        }
        /** \brief Constructs bool Extension with default value v */
        explicit Extension(bool v)
		{
            ext.m_bool.defaultValue = v;
            mandatoriness = eDefaultable;
        }
        /** \brief Constructs int32_t Extension with default value v */
		explicit Extension(int32_t v)
		{
            ext.m_int.defaultValue = v;
            mandatoriness = eDefaultable;
        }
        /** \brief Constructs uint32_t Extension with default value v */
		explicit Extension(uint32_t v)
		{
            ext.m_uint.defaultValue = v;
            mandatoriness = eDefaultable;
        }
        /** \brief Constructs int64_t Extension with default value v */
        explicit Extension(int64_t v)
        {
            ext.m_int64.defaultValue = v;
            mandatoriness = eDefaultable;
        }
        /** \brief Constructs uint64_t Extension with default value v */
        explicit Extension(uint64_t v)
        {
            ext.m_uint64.defaultValue = v;
            mandatoriness = eDefaultable;
        }
        /** \brief Constructs float Extension with default value v */
		explicit Extension(float v)
		{
            ext.m_float.defaultValue = v;
            mandatoriness = eDefaultable;
        }
        /** \brief Constructs double Extension with default value v */
        explicit Extension(double v)
        {
            ext.m_double.defaultValue = v;
            mandatoriness = eDefaultable;
        }
        /** \brief Constructs metacpp::String Extension with default value v */
		explicit Extension(const char *v)
		{
            ext.m_string.defaultValue = v;
            mandatoriness = eDefaultable;
        }

        /** \brief Constructs metacpp::DateTime Extension with default value v */
        explicit Extension(const metacpp::DateTime& v)
        {
            ext.m_datetime.defaultValue = v.toStdTime();
            mandatoriness = eDefaultable;
        }

        /** \brief Constructs Extension with given mandatoriness m */
        explicit Extension(EMandatoriness m)
        {
            mandatoriness = m;
        }
        /** \brief Constructs enum Extension with given enumInfo */
        explicit Extension(const EnumInfoDescriptor *enumInfo)
		{
            ext.m_enum.enumInfo = enumInfo;
            mandatoriness = eDefaultable;
        }
        /** \brief Constructs metacpp::Array Extension with given elemType and elemSize */
        explicit Extension(EFieldType elemType, size_t elemSize)
		{
            ext.m_array.elemType = elemType;
            ext.m_array.elemSize = elemSize;
            mandatoriness = eDefaultable;
        }
        /** \brief Constructs metacpp::Object Extension with given metaObject */
        explicit Extension(const metacpp::MetaObject *metaObject)
        {
            ext.m_obj.metaObject = metaObject;
            mandatoriness = eDefaultable;
        }
	} valueInfo;
};

namespace detail
{
    template<typename T>
    struct PartialFieldInfoHelper;

    template<typename T, bool IsEnum = std::is_enum<T>::value, bool IsObject = std::is_base_of<metacpp::Object, T>::value>
    struct FullFieldInfoHelper;

    template<>
    struct PartialFieldInfoHelper<bool> {
        static constexpr EFieldType type() { return eFieldBool; }
        static FieldInfoDescriptor::Extension extension(bool v) { return FieldInfoDescriptor::Extension(v); }
        static FieldInfoDescriptor::Extension extension(EMandatoriness m = eOptional) { return FieldInfoDescriptor::Extension(m); }
    };

    template<>
    struct PartialFieldInfoHelper<int32_t> {
        static constexpr EFieldType type() { return eFieldInt; }
        static FieldInfoDescriptor::Extension extension(int32_t v) { return FieldInfoDescriptor::Extension(v); }
        static FieldInfoDescriptor::Extension extension(EMandatoriness m = eOptional) { return FieldInfoDescriptor::Extension(m); }
    };

    template<>
    struct PartialFieldInfoHelper<uint32_t> {
        static constexpr EFieldType type() { return eFieldUint; }
        static FieldInfoDescriptor::Extension extension(uint32_t v) { return FieldInfoDescriptor::Extension(v); }
        static FieldInfoDescriptor::Extension extension(EMandatoriness m = eOptional) { return FieldInfoDescriptor::Extension(m); }
    };

    template<>
    struct PartialFieldInfoHelper<int64_t> {
        static constexpr EFieldType type() { return eFieldInt64; }
        static FieldInfoDescriptor::Extension extension(int64_t v) { return FieldInfoDescriptor::Extension(v); }
        static FieldInfoDescriptor::Extension extension(EMandatoriness m = eOptional) { return FieldInfoDescriptor::Extension(m); }
    };

    template<>
    struct PartialFieldInfoHelper<uint64_t> {
        static constexpr EFieldType type() { return eFieldUint64; }
        static FieldInfoDescriptor::Extension extension(uint64_t v) { return FieldInfoDescriptor::Extension(v); }
        static FieldInfoDescriptor::Extension extension(EMandatoriness m = eOptional) { return FieldInfoDescriptor::Extension(m); }
    };

    template<>
    struct PartialFieldInfoHelper<float> {
        static constexpr EFieldType type() { return eFieldFloat; }
        static FieldInfoDescriptor::Extension extension(float v) { return FieldInfoDescriptor::Extension(v); }
        static FieldInfoDescriptor::Extension extension(EMandatoriness m = eOptional) { return FieldInfoDescriptor::Extension(m); }
    };

    template<>
    struct PartialFieldInfoHelper<double> {
        static constexpr EFieldType type() { return eFieldDouble; }
        static FieldInfoDescriptor::Extension extension(double v) { return FieldInfoDescriptor::Extension(v); }
        static FieldInfoDescriptor::Extension extension(EMandatoriness m = eOptional) { return FieldInfoDescriptor::Extension(m); }
    };

    template<>
    struct PartialFieldInfoHelper<metacpp::String> {
        static constexpr EFieldType type() { return eFieldString; }
        static FieldInfoDescriptor::Extension extension(const char *v) { return FieldInfoDescriptor::Extension(v); }
        static FieldInfoDescriptor::Extension extension(EMandatoriness m = eOptional) { return FieldInfoDescriptor::Extension(m); }
    };

    template<>
    struct PartialFieldInfoHelper<metacpp::DateTime> {
        static constexpr EFieldType type() { return eFieldDateTime; }
        static FieldInfoDescriptor::Extension extension(const metacpp::DateTime& v) { return FieldInfoDescriptor::Extension(v); }
        static FieldInfoDescriptor::Extension extension(EMandatoriness m = eOptional) { return FieldInfoDescriptor::Extension(m); }
    };

    template<typename T>
    struct PartialFieldInfoHelper<metacpp::Array<T> >
    {
        static constexpr EFieldType type() { return eFieldArray; }
        static constexpr FieldInfoDescriptor::Extension extension()
        {
            return FieldInfoDescriptor::Extension(FullFieldInfoHelper<T>::type(), sizeof(T));
        }
    };

    template<typename T>
    struct FullFieldInfoHelper<T, true, false>
    {
        static constexpr EFieldType type() { return eFieldEnum; }
        static constexpr FieldInfoDescriptor::Extension extension(const EnumInfoDescriptor *enumInfo) { return FieldInfoDescriptor::Extension(enumInfo); }
        static constexpr bool nullable() { return false; }
    };


    template<typename T>
    struct FullFieldInfoHelper<T, false, false> : public PartialFieldInfoHelper<T>
    {
        static constexpr bool nullable() { return false; }
    };

    template<typename T>
    struct FullFieldInfoHelper<T, false, true>
    {
        static constexpr EFieldType type() { return eFieldObject; }
        static constexpr FieldInfoDescriptor::Extension extension() { return FieldInfoDescriptor::Extension(T::staticMetaObject()); }
        static constexpr bool nullable() { return false; }
    };

    template<typename T>
    struct FullFieldInfoHelper<Nullable<T>, false, false> : public FullFieldInfoHelper<T>
    {
        static constexpr bool nullable() { return true; }
    };
} // namespace detail

/** \brief Exception thrown internally during invokation of reflection methods */
class BindArgumentException : public std::invalid_argument
{
public:
    BindArgumentException(const char *errorMsg = "Cannot bind argument")
        : std::invalid_argument(errorMsg)
    {
    }
};

/** \brief Exception thrown when a method with specified name and signature is not found in metainformation */
class MethodNotFoundException : public std::invalid_argument
{
public:
    MethodNotFoundException(const char *methodName)
        : std::invalid_argument(std::string("Method not found: ") + methodName)
    {
    }
};

namespace
{
    template<typename Func, typename TRes, typename Tuple, bool Done, int Total, int... N>
    struct fcall_impl
    {
        static TRes call(Func f, Tuple && t)
        {
            return fcall_impl<Func, TRes, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(f, std::forward<Tuple>(t));
        }
    };

    template<typename Func, typename TRes, typename Tuple, int Total, int... N>
    struct fcall_impl<Func, TRes, Tuple, true, Total, N...>
    {
        static TRes call(Func f, Tuple && t)
        {
            return f(std::get<N>(std::forward<Tuple>(t))...);
        }
    };

    template<typename Func, typename TRes, typename TObj, typename Tuple, bool Done, int Total, int... N>
    struct mcall_impl
    {
        static TRes call(Func f, TObj *obj, Tuple && t)
        {
            return mcall_impl<Func, TRes, TObj, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(f, obj, std::forward<Tuple>(t));
        }
    };

    template<typename Func, typename TRes, typename TObj, typename Tuple, int Total, int... N>
    struct mcall_impl<Func, TRes, TObj, Tuple, true, Total, N...>
    {
        static TRes call(Func f, TObj *obj, Tuple && t)
        {
            return (*obj.*f)(std::get<N>(std::forward<Tuple>(t))...);
        }
    };

    template<typename THead, typename TTail>
    struct argtuple_impl;

	template<typename...THeadArgs>
	struct argtuple_impl<std::tuple<THeadArgs...>, std::tuple<> >
	{
		typedef std::tuple<THeadArgs...> type;
	};

	template<typename... TArgs>
	struct argtuple
	{
		typedef typename argtuple_impl<std::tuple<>, std::tuple<TArgs...> >::type type;
	};

    template<template<typename... > class THead, typename...THeadArgs,
        template<typename... > class TTail, typename TCurrent, typename...TTailArgs>
    struct argtuple_impl<THead<THeadArgs...>, TTail<TCurrent, TTailArgs...> >
    {
        typedef typename argtuple_impl<THead<THeadArgs..., typename std::remove_cv<typename std::remove_reference<TCurrent>::type>::type>, TTail<TTailArgs...> >::type type;
    };

    template<size_t N, bool Done, typename TType>
    struct unpack_impl;


    template<size_t N, bool Done, template<typename... > class TType, typename...TArgs>
    struct unpack_impl<N, Done, TType<TArgs...> >
    {
        static void unpack_arguments(TType<TArgs...>& t, const metacpp::Array<metacpp::Variant>& argList)
        {
            typedef typename std::tuple_element<N, TType<TArgs...> >::type ArgType;
            try
            {
                std::get<N>(t) = metacpp::variant_cast<ArgType>(argList[N]);
            }
            catch (const std::invalid_argument& e)
            {
                throw BindArgumentException(e.what());
            }
            unpack_impl<N + 1, N + 1 == sizeof...(TArgs), TType<TArgs...> >::unpack_arguments(t, argList);
        }
    };

    template<size_t N, template<typename... > class TType, typename...TArgs>
    struct unpack_impl<N, true, TType<TArgs...> >
    {
        static void unpack_arguments(TType<TArgs...>&, const metacpp::Array<metacpp::Variant>&)
        {
        }
    };
}

/** \brief Base helper class for invokation of reflection methods */
class MetaInvokerBase
{
public:
    virtual ~MetaInvokerBase() { }
    /** \brief Invokes method with the given context and arguments */
    virtual metacpp::Variant invoke(const void *contextObj, const metacpp::Array<metacpp::Variant>& argList) const = 0;
};

/** \brief Helper class for invokation of static methods
 */
template<typename TRes, typename... TArgs>
class FunctionInvoker : public MetaInvokerBase
{
private:

    template<typename Q = TRes>
    typename std::enable_if<std::is_same<Q, void>::value, metacpp::Variant>::type doInvoke(const metacpp::Array<metacpp::Variant> &argList) const
    {
        invokeImpl(argList);
        return metacpp::Variant();
    }

    template<typename Q = TRes>
    typename std::enable_if<!std::is_same<Q, void>::value, metacpp::Variant>::type doInvoke(const metacpp::Array<metacpp::Variant> &argList) const
    {
        return metacpp::Variant(invokeImpl(argList));
    }


    TRes invokeImpl(const metacpp::Array<metacpp::Variant>& argList) const
    {
        typedef typename argtuple<TArgs...>::type ttype;
        ttype args;
        if (sizeof...(TArgs) != argList.size())
            throw BindArgumentException(metacpp::String("Invalid number of arguments, " +
                                               metacpp::String::fromValue(sizeof...(TArgs)) + " expected").c_str());
        unpack_impl<0, 0 == sizeof...(TArgs), ttype>::unpack_arguments(args, argList);
        return fcall_impl<TFunction, TRes, ttype, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>
                ::call(m_function, std::forward<ttype>(args));
    }

public:
    /** \brief Type of the static method */
    typedef TRes (*TFunction)(TArgs...);

    /** \brief Construct a new instance of FunctionInvoker with given func */
    explicit FunctionInvoker(TFunction func)
        : m_function(func)
    {
    }
    /** \brief Overrides MetaInvokerBase::invoke */
    metacpp::Variant invoke(const void *metaObject, const metacpp::Array<metacpp::Variant>& argList) const override
    {
        (void)metaObject;
        return doInvoke(argList);
    }

private:
    TFunction m_function;
};

/** \brief Helper class for invokation of non-const own methods
 */
template<typename TRes, typename TObj, typename... TArgs>
class MethodInvoker : public MetaInvokerBase
{
private:

    template<typename Q = TRes>
    typename std::enable_if<std::is_same<Q, void>::value, metacpp::Variant>::type doInvoke(TObj *obj, const metacpp::Array<metacpp::Variant> &argList) const
    {
        invokeImpl(obj, argList);
        return metacpp::Variant();
    }

    template<typename Q = TRes>
    typename std::enable_if<!std::is_same<Q, void>::value, metacpp::Variant>::type doInvoke(TObj *obj, const metacpp::Array<metacpp::Variant> &argList) const
    {
        return metacpp::Variant(invokeImpl(obj, argList));
    }

    TRes invokeImpl(TObj *obj, const metacpp::Array<metacpp::Variant>& argList) const
    {
        typedef typename argtuple<TArgs...>::type ttype;
        ttype args;
        if (sizeof...(TArgs) != argList.size())
            throw BindArgumentException(metacpp::String("Invalid number of arguments, " +
                                               metacpp::String::fromValue(sizeof...(TArgs)) + " expected").c_str());
        unpack_impl<0, 0 == sizeof...(TArgs), ttype>::unpack_arguments(args, argList);
        return mcall_impl<TFunction, TRes, TObj, ttype, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>
                ::call(m_method, obj, std::forward<ttype>(args));
    }
public:
    /** \brief Type of the own method */
    typedef TRes (TObj::*TFunction)(TArgs...);

    /** \brief Construct a new instance of MethodInvoker with given func */
    explicit MethodInvoker(TFunction function)
        : m_method(function)
    {
    }


    /** \brief Overrides MetaInvokerBase::invoke */
    metacpp::Variant invoke(const void *obj, const metacpp::Array<metacpp::Variant>& argList) const override
    {
        return doInvoke(reinterpret_cast<TObj *>(const_cast<void *>(obj)), argList);
    }

private:
    TFunction m_method;
};

/** \brief Helper class for invokation of const own methods
 */
template<typename TRes, typename TObj, typename... TArgs>
class ConstMethodInvoker : public MetaInvokerBase
{
private:

    template<typename Q = TRes>
    typename std::enable_if<std::is_same<Q, void>::value, metacpp::Variant>::type doInvoke(const TObj *obj, const metacpp::Array<metacpp::Variant> &argList) const
    {
        invokeImpl(obj, argList);
        return metacpp::Variant();
    }

    template<typename Q = TRes>
    typename std::enable_if<!std::is_same<Q, void>::value, metacpp::Variant>::type doInvoke(const TObj *obj, const metacpp::Array<metacpp::Variant> &argList) const
    {
        return metacpp::Variant(invokeImpl(obj, argList));
    }

    TRes invokeImpl(const TObj *obj, const metacpp::Array<metacpp::Variant>& argList) const
    {
        typedef typename argtuple<TArgs...>::type ttype;
        ttype args;
        if (sizeof...(TArgs) != argList.size())
            throw BindArgumentException(metacpp::String("Invalid number of arguments, " +
                                               metacpp::String::fromValue(sizeof...(TArgs)) + " expected").c_str());
        unpack_impl<0, 0 == sizeof...(TArgs), ttype>::unpack_arguments(args, argList);
        return mcall_impl<TFunction, TRes, const TObj, ttype, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>
                ::call(m_method, obj, std::forward<ttype>(args));
    }

public:
    /** \brief Type of the own method */
    typedef TRes (TObj::*TFunction)(TArgs...) const;

    /** \brief Construct a new instance of ConstMethodInvoker with given func */
    ConstMethodInvoker(TFunction function)
        : m_method(function)
    {
    }

    /** \brief Overrides MetaInvokerBase::invoke */
    metacpp::Variant invoke(const void *obj, const metacpp::Array<metacpp::Variant>& argList) const override
    {
        return doInvoke(reinterpret_cast<const TObj *>(obj), argList);
    }

private:
    TFunction m_method;
};

namespace detail
{
    template<typename TRes, typename... TArgs>
    std::unique_ptr<MetaInvokerBase> createInvokeHelper(TRes (*func)(TArgs...))
    {
        return std::move(std::unique_ptr<MetaInvokerBase>(new FunctionInvoker<TRes, TArgs...>(func)));
    }

    template<typename TRes, typename TObj, typename... TArgs>
    std::unique_ptr<MetaInvokerBase> createInvokeHelper(TRes (TObj::*function)(TArgs...))
    {
        return std::move(std::unique_ptr<MetaInvokerBase>(new MethodInvoker<TRes, TObj, TArgs...>(function)));
    }

    template<typename TRes, typename TObj, typename... TArgs>
    std::unique_ptr<MetaInvokerBase> createInvokeHelper(TRes (TObj::*function)(TArgs...) const)
    {
        return std::move(std::unique_ptr<MetaInvokerBase>(new ConstMethodInvoker<TRes, TObj, TArgs...>(function)));
    }
} // namespace detail

/** \brief Type of the reflection method.
 * Static methods are called by providing a pointer to metacpp::MetaObject as first argument
 * to the invoker. Own methods are called by providing a pointer to metacpp::Object.
*/
enum EMethodType
{
    eMethodNone,    /**< \brief Invalid type */
    eMethodStatic,  /**< \brief Static method */
    eMethodOwn      /**< \brief Own method */
};

/** \brief Structure describing reflection method and providing a way for it's invokation */
struct MethodInfoDescriptor
{
    const char *m_pszName;                          /**< \brief name of the method */
    EMethodType m_eType;                            /**< \brief method type */
    bool m_bConstness;                              /**< \brief constness of self-reference for own methods. \see ConstMethodInvoker, MethodInvoker */
    size_t m_nArgs;                                 /**< \brief number of arguments passing to the method (self-reference is not counted here) */
    std::unique_ptr<MetaInvokerBase> m_pInvoker;    /**< \brief invoker helper */
};

namespace detail
{
    template<typename TFunction>
    struct MethodInfoHelper;

    template<typename TRes, typename... TArgs>
    struct MethodInfoHelper<TRes (*)(TArgs...)>
    {
        static constexpr EMethodType type() { return eMethodStatic; }
        static constexpr bool constness() { return false; }
        static constexpr size_t numArguments() { return sizeof...(TArgs); }
        static std::unique_ptr<MetaInvokerBase> createInvoker(TRes (*func)(TArgs...))
        {
            return std::move(std::unique_ptr<MetaInvokerBase>(new FunctionInvoker<TRes, TArgs...>(func)));
        }
    };

    template<typename TRes, typename TObj, typename... TArgs>
    struct MethodInfoHelper<TRes (TObj::*)(TArgs...)>
    {
        static constexpr EMethodType type() { return eMethodOwn; }
        static constexpr bool constness() { return false; }
        static constexpr size_t numArguments() { return sizeof...(TArgs); }
        static std::unique_ptr<MetaInvokerBase> createInvoker(TRes (TObj::*function)(TArgs...))
        {
            return std::move(std::unique_ptr<MetaInvokerBase>(new MethodInvoker<TRes, TObj, TArgs...>(function)));
        }
    };

    template<typename TRes, typename TObj, typename... TArgs>
    struct MethodInfoHelper<TRes (TObj::*)(TArgs...) const>
    {
        static constexpr EMethodType type() { return eMethodOwn; }
        static constexpr bool constness() { return true; }
        static constexpr size_t numArguments() { return sizeof...(TArgs); }
        static std::unique_ptr<MetaInvokerBase> createInvoker(TRes (TObj::*function)(TArgs...) const)
        {
            return std::move(std::unique_ptr<MetaInvokerBase>(new ConstMethodInvoker<TRes, TObj, TArgs...>(function)));
        }
    };
} // namespace detail

/**
  * \brief Structure describing class internals
*/
struct MetaInfoDescriptor
{
    const char                      *m_strucName;           /**< \brief Name of the struct */
    size_t                          m_dwSize;               /**< \brief Size of the struct */
    const MetaInfoDescriptor        *m_superDescriptor;     /**< \brief Pointer to the super (base) struct descriptor */
    const FieldInfoDescriptor		*m_fieldDescriptors;    /**< \brief Pointer to the array of property descriptors (terminated with dummy descriptor) */
    const MethodInfoDescriptor      *m_methodDescriptors;   /**< \brief Pointer to the array of method descriptors (terminated with dummy descriptor) */
};

/** \brief Starts a list of method descriptors
 * \relates MetaInfoDescriptor
 */
#define METHOD_INFO_BEGIN(obj) \
    const MethodInfoDescriptor _methodInfos_##obj[] = {

/** \brief Terminates a list of method descriptors
 * \relates MetaInfoDescriptor
 */
#define METHOD_INFO_END(obj) \
        { nullptr, eMethodNone, false, 0, nullptr } \
    };

/** \brief Puts a named method descriptor into the list
 * \relates MetaInfoDescriptor
 * \see METHOD_INFO_BEGIN, METHOD_INFO_END, METHOD, SIGNATURE_METHOD
 */
#define NAMED_METHOD(name, pMethod) \
    { \
        /* name     */   name, \
        /* type */       ::detail::MethodInfoHelper<decltype(pMethod)>::type(), \
        /* constness */  ::detail::MethodInfoHelper<decltype(pMethod)>::constness(), \
        /* num args */   ::detail::MethodInfoHelper<decltype(pMethod)>::numArguments(), \
        /* invoker */    ::detail::MethodInfoHelper<decltype(pMethod)>::createInvoker(pMethod) \
    },

/** \brief Puts a method descriptor into the list
 * \relates MetaInfoDescriptor
 * \see METHOD_INFO_BEGIN, METHOD_INFO_END, NAMED_METHOD, SIGNATURE_METHOD
 */
#define METHOD(obj, method) NAMED_METHOD(#method, &obj::method)

/** \brief Puts a method descriptor with given signature into the list
 * \relates MetaInfoDescriptor
 * \see METHOD_INFO_BEGIN, METHOD_INFO_END, NAMED_METHOD, METHOD
 */
#define SIGNATURE_METHOD(obj, method, signature) NAMED_METHOD(#method, static_cast<signature>(&obj::method))

/** \brief Starts a list of property descriptors
 * \relates MetaInfoDescriptor
 */
#define STRUCT_INFO_BEGIN(struc) \
    const FieldInfoDescriptor _fieldInfos_##struc[] = {

/** \brief Terminates a list of property descriptors
 * \relates MetaInfoDescriptor
 */
#define STRUCT_INFO_END(struc) \
        { 0, 0, 0, eFieldVoid, false, FieldInfoDescriptor::Extension() } \
    };

/** \brief Puts a property descriptor into the list
 * \relates MetaInfoDescriptor
 * \see STRUCT_INFO_BEGIN, STRUCT_INFO_END
 */
#define FIELD(struc, field, ...) { \
    /* name */      #field, \
    /* size */      sizeof(decltype(struc::field)), \
    /* offset */    offsetof(struc, field), \
    /* type */      ::detail::FullFieldInfoHelper<std::remove_cv<decltype(struc::field)>::type>::type(), \
    /* nullable */  ::detail::FullFieldInfoHelper<std::remove_cv<decltype(struc::field)>::type>::nullable(), \
    /* extension */ ::detail::FullFieldInfoHelper<std::remove_cv<decltype(struc::field)>::type>::extension(__VA_ARGS__) \
    },

/** \brief Macro used for accessing previously declared MetaInfoDescriptor for the struc
  \relates MetaInfoDescriptor
 */
#define REFLECTIBLE_DESCRIPTOR(struc) _descriptor_##struc

/** \brief Declares MetaInfoDescriptor for the struc
  \relates MetaInfoDescriptor
 */
#define REFLECTIBLE_DESCRIPTOR_DECLARE(struc) extern const MetaInfoDescriptor _descriptor_##struc;

/** \brief Defines MetaInfoDescriptor for the struc with property (field) reflection info
  \relates MetaInfoDescriptor
 */
#define REFLECTIBLE_F(struc) \
    const MetaInfoDescriptor _descriptor_##struc = { #struc, sizeof(struc), nullptr, _fieldInfos_##struc, nullptr }; \

/** \brief Defines MetaInfoDescriptor for the struc with given super class and property (field) reflection info
  \relates MetaInfoDescriptor
 */
#define REFLECTIBLE_DERIVED_F(struc, superStruc) \
    const MetaInfoDescriptor _descriptor_##struc = { #struc, sizeof(struc), &REFLECTIBLE_DESCRIPTOR(superStruc), _fieldInfos_##struc, nullptr  }; \

/** \brief Defines MetaInfoDescriptor for the struc with method reflection info
  \relates MetaInfoDescriptor
 */
#define REFLECTIBLE_M(struc) \
    const MetaInfoDescriptor _descriptor_##struc = { #struc, sizeof(struc), nullptr, nullptr, _methodInfos_##struc }; \

/** \brief Defines MetaInfoDescriptor for the struc with given super class and method reflection info
  \relates MetaInfoDescriptor
 */
#define REFLECTIBLE_DERIVED_M(struc, superStruc) \
    const MetaInfoDescriptor _descriptor_##struc = { #struc, sizeof(struc), &REFLECTIBLE_DESCRIPTOR(superStruc), nullptr, _methodInfos_##struc }; \

/** \brief Defines MetaInfoDescriptor for the struc with field and method reflection info
  \relates MetaInfoDescriptor
 */
#define REFLECTIBLE_FM(struc) \
    const MetaInfoDescriptor _descriptor_##struc = { #struc, sizeof(struc), nullptr, _fieldInfos_##struc, _methodInfos_##struc }; \

/** \brief Defines MetaInfoDescriptor for the struc with given super class and field and method reflection info
  \relates MetaInfoDescriptor
 */
#define REFLECTIBLE_DERIVED_FM(struc, superStruc) \
    const MetaInfoDescriptor _descriptor_##struc = { #struc, sizeof(struc), &REFLECTIBLE_DESCRIPTOR(superStruc), _fieldInfos_##struc, _methodInfos_##struc }; \

/** \brief Starts a list of enumeration value descriptors
 * \relates EnumInfoDescriptor
 */
#define ENUM_INFO_BEGIN(_enum, type, def) \
    extern const EnumValueInfoDescriptor _enumValueInfos_##_enum[]; \
    const EnumInfoDescriptor _enumInfo_##_enum = { type, #_enum, (uint32_t)def, _enumValueInfos_##_enum }; \
    const EnumValueInfoDescriptor _enumValueInfos_##_enum[] = {

/** \brief Terminates a list of enumeration value descriptors
 * \relates EnumInfoDescriptor
 */
#define ENUM_INFO_END(_enum) \
		{ nullptr, 0 } \
	};

/** \brief Macro used for accessing previously declared EnumInfoDescriptor
  \relates EnumInfoDescriptor
 */
#define ENUM_INFO(_enum) _enumInfo_##_enum

/** \brief Declares EnumInfoDescriptor
  \relates EnumInfoDescriptor
 */
#define ENUM_INFO_DECLARE(_enum) extern const EnumInfoDescriptor _enumInfo_##_enum;

/** \brief Puts a enum value descriptor into the list
 * \relates EnumInfoDescriptor
 * \see ENUM_INFO_BEGIN, ENUM_INFO_END
 */
#define VALUE_INFO(name) \
	{ #name, (uint32_t)name },

#endif // METAINFO_H
