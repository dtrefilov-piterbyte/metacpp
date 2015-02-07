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
#include "String.h"
#include "Nullable.h"
#include "DateTime.h"
#include "Variant.h"
#include "MetaType.h"

namespace metacpp
{
class Object;
class MetaObject;
}

struct EnumValueInfoDescriptor
{
	const char		*m_pszValue;
	uint32_t		m_uValue;
};

struct EnumInfoDescriptor
{
	EEnumType		m_type;
	const char		*m_enumName;
	uint32_t		m_defaultValue;
    const EnumValueInfoDescriptor *m_valueDescriptors;
};

struct FieldInfoDescriptor
{
	const char *m_pszName;
	size_t		m_dwSize;
	ptrdiff_t	m_dwOffset;
	EFieldType	m_eType;
    bool        m_nullable;

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
        EMandatoriness	mandatoriness;

        explicit Extension()
		{
            mandatoriness = eOptional;
        }
        explicit Extension(bool v)
		{
            ext.m_bool.defaultValue = v;
            mandatoriness = eDefaultable;
        }
		explicit Extension(int32_t v)
		{
            ext.m_int.defaultValue = v;
            mandatoriness = eDefaultable;
        }
		explicit Extension(uint32_t v)
		{
            ext.m_uint.defaultValue = v;
            mandatoriness = eDefaultable;
        }
        explicit Extension(int64_t v)
        {
            ext.m_int64.defaultValue = v;
            mandatoriness = eDefaultable;
        }
        explicit Extension(uint64_t v)
        {
            ext.m_uint64.defaultValue = v;
            mandatoriness = eDefaultable;
        }
		explicit Extension(float v)
		{
            ext.m_float.defaultValue = v;
            mandatoriness = eDefaultable;
        }
        explicit Extension(double v)
        {
            ext.m_double.defaultValue = v;
            mandatoriness = eDefaultable;
        }

		explicit Extension(const char *v)
		{
            ext.m_string.defaultValue = v;
            mandatoriness = eDefaultable;
        }

        explicit Extension(const metacpp::DateTime& v)
        {
            ext.m_datetime.defaultValue = v.toStdTime();
            mandatoriness = eDefaultable;
        }

        explicit Extension(EMandatoriness m)
        {
            mandatoriness = m;
        }
        explicit Extension(const EnumInfoDescriptor *enumInfo)
		{
            ext.m_enum.enumInfo = enumInfo;
            mandatoriness = eDefaultable;
        }
        explicit Extension(EFieldType elemType, size_t elemSize)
		{
            ext.m_array.elemType = elemType;
            ext.m_array.elemSize = elemSize;
            mandatoriness = eDefaultable;
        }
        explicit Extension(const metacpp::MetaObject *metaObject)
        {
            ext.m_obj.metaObject = metaObject;
            mandatoriness = eDefaultable;
        }
	} valueInfo;
};

#if defined(_MSC_VER) && !defined(constexpr)
#define constexpr
#endif

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

class BindArgumentException : public std::invalid_argument
{
public:
    BindArgumentException(const char *errorMsg = "Cannot bind argument")
        : std::invalid_argument(errorMsg)
    {
    }
};

class MethodNotFoundException : public std::invalid_argument
{
public:
    MethodNotFoundException(const char *methodName)
        : std::invalid_argument(std::string("Method not found: ") + methodName)
    {
    }
};

namespace detail
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


    template<size_t N, bool Done, typename... TArgs>
    struct unpack_impl
    {
        static void unpack_arguments(std::tuple<TArgs...>& t, const metacpp::Array<metacpp::Variant>& argList)
        {
            typedef typename std::tuple_element<N, std::tuple<TArgs...>>::type ArgType;
            try
            {
                std::get<N>(t) = metacpp::variant_cast<ArgType>(argList[N]);
            }
            catch (const std::invalid_argument& e)
            {
                throw BindArgumentException(e.what());
            }
            unpack_impl<N + 1, N + 1 == sizeof...(TArgs), TArgs...>::unpack_arguments(t, argList);
        }
    };

    template<size_t N, typename... TArgs>
    struct unpack_impl<N, true, TArgs...>
    {
        static void unpack_arguments(std::tuple<TArgs...>&, const metacpp::Array<metacpp::Variant>&)
        {
        }
    };
}

class MetaInvokerBase
{
public:
    virtual ~MetaInvokerBase() { }
    virtual metacpp::Variant invoke(const void *contextObj, const metacpp::Array<metacpp::Variant>& argList) const = 0;
};

template<typename TRes, typename... TArgs>
class FunctionInvoker : public MetaInvokerBase
{
private:

    template<typename Q = TRes>
    typename std::enable_if<std::is_same<Q, void>::value, metacpp::Variant>::type invokeHelper(const metacpp::Array<metacpp::Variant> &argList) const
    {
        doInvoke(argList);
        return metacpp::Variant();
    }

    template<typename Q = TRes>
    typename std::enable_if<!std::is_same<Q, void>::value, metacpp::Variant>::type invokeHelper(const metacpp::Array<metacpp::Variant> &argList) const
    {
        return metacpp::Variant(doInvoke(argList));
    }

public:
    typedef TRes (*TFunction)(TArgs...);

    explicit FunctionInvoker(TFunction func)
        : m_function(func)
    {
    }

    TRes doInvoke(const metacpp::Array<metacpp::Variant>& argList) const
    {
        typedef std::tuple<TArgs...> ttype;
        ttype args;
        if (sizeof...(TArgs) != argList.size())
            throw BindArgumentException(metacpp::String("Invalid number of arguments, " +
                                               metacpp::String::fromValue(sizeof...(TArgs)) + " expected").c_str());
        detail::unpack_impl<0, 0 == sizeof...(TArgs), TArgs...>::unpack_arguments(args, argList);
        return detail::fcall_impl<TFunction, TRes, ttype, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>
                ::call(m_function, std::forward<ttype>(args));
    }

    metacpp::Variant invoke(const void *metaObject, const metacpp::Array<metacpp::Variant>& argList) const override
    {
        (void)metaObject;
        return invokeHelper(argList);
    }

private:
    TFunction m_function;
};

template<typename TRes, typename TObj, typename... TArgs>
class MethodInvoker : public MetaInvokerBase
{
private:

    template<typename Q = TRes>
    typename std::enable_if<std::is_same<Q, void>::value, metacpp::Variant>::type invokeHelper(TObj *obj, const metacpp::Array<metacpp::Variant> &argList) const
    {
        doInvoke(obj, argList);
        return metacpp::Variant();
    }

    template<typename Q = TRes>
    typename std::enable_if<!std::is_same<Q, void>::value, metacpp::Variant>::type invokeHelper(TObj *obj, const metacpp::Array<metacpp::Variant> &argList) const
    {
        return metacpp::Variant(doInvoke(obj, argList));
    }

public:
    typedef TRes (TObj::*TFunction)(TArgs...);

    explicit MethodInvoker(TFunction function)
        : m_method(function)
    {
    }

    TRes doInvoke(TObj *obj, const metacpp::Array<metacpp::Variant>& argList) const
    {
        typedef std::tuple<TArgs...> ttype;
        ttype args;
        if (sizeof...(TArgs) != argList.size())
            throw BindArgumentException(metacpp::String("Invalid number of arguments, " +
                                               metacpp::String::fromValue(sizeof...(TArgs)) + " expected").c_str());
        detail::unpack_impl<0, 0 == sizeof...(TArgs), TArgs...>::unpack_arguments(args, argList);
        return detail::mcall_impl<TFunction, TRes, TObj, ttype, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>
                ::call(m_method, obj, std::forward<ttype>(args));
    }

    metacpp::Variant invoke(const void *obj, const metacpp::Array<metacpp::Variant>& argList) const override
    {
        return invokeHelper(reinterpret_cast<TObj *>(const_cast<void *>(obj)), argList);
    }

private:
    TFunction m_method;
};

template<typename TRes, typename TObj, typename... TArgs>
class ConstMethodInvoker : public MetaInvokerBase
{
private:

    template<typename Q = TRes>
    typename std::enable_if<std::is_same<Q, void>::value, metacpp::Variant>::type invokeHelper(const TObj *obj, const metacpp::Array<metacpp::Variant> &argList) const
    {
        doInvoke(obj, argList);
        return metacpp::Variant();
    }

    template<typename Q = TRes>
    typename std::enable_if<!std::is_same<Q, void>::value, metacpp::Variant>::type invokeHelper(const TObj *obj, const metacpp::Array<metacpp::Variant> &argList) const
    {
        return metacpp::Variant(doInvoke(obj, argList));
    }

public:
    typedef TRes (TObj::*TFunction)(TArgs...) const;

    ConstMethodInvoker(TFunction function)
        : m_method(function)
    {
    }

    TRes doInvoke(const TObj *obj, const metacpp::Array<metacpp::Variant>& argList) const
    {
        typedef std::tuple<TArgs...> ttype;
        ttype args;
        if (sizeof...(TArgs) != argList.size())
            throw BindArgumentException(metacpp::String("Invalid number of arguments, " +
                                               metacpp::String::fromValue(sizeof...(TArgs)) + " expected").c_str());
        detail::unpack_impl<0, 0 == sizeof...(TArgs), TArgs...>::unpack_arguments(args, argList);
        return detail::mcall_impl<TFunction, TRes, const TObj, ttype, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>
                ::call(m_method, obj, std::forward<ttype>(args));
    }

    metacpp::Variant invoke(const void *obj, const metacpp::Array<metacpp::Variant>& argList) const override
    {
        return invokeHelper(reinterpret_cast<const TObj *>(obj), argList);
    }

private:
    TFunction m_method;
};

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

enum EMethodType
{
    eMethodNone,
    eMethodStatic,
    eMethodOwn
};

struct MethodInfoDescriptor
{
    const char *m_pszName;
    EMethodType m_eType;
    bool m_bConstness;
    size_t m_nArgs;
    std::unique_ptr<MetaInvokerBase> m_pInvoker;
};

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

struct MetaInfoDescriptor
{
    const char                      *m_strucName;
    size_t                          m_dwSize;
    const MetaInfoDescriptor        *m_superDescriptor;
    const FieldInfoDescriptor		*m_fieldDescriptors;
    const MethodInfoDescriptor      *m_methodDescriptors;
};

#define METHOD_INFO_BEGIN(obj) \
    const MethodInfoDescriptor _methodInfos_##obj[] = {

#define METHOD_INFO_END(obj) \
        { nullptr, eMethodNone, false, 0, nullptr } \
    };

#define NAMED_METHOD(name, pMethod) \
    { \
        /* name     */   name, \
        /* type */       MethodInfoHelper<decltype(pMethod)>::type(), \
        /* constness */  MethodInfoHelper<decltype(pMethod)>::constness(), \
        /* num args */   MethodInfoHelper<decltype(pMethod)>::numArguments(), \
        /* invoker */    MethodInfoHelper<decltype(pMethod)>::createInvoker(pMethod) \
    },

#define METHOD(obj, method) NAMED_METHOD(#method, &obj::method)
#define SIGNATURE_METHOD(obj, method, signature) NAMED_METHOD(#method, static_cast<signature>(&obj::method))

#define STRUCT_INFO_BEGIN(struc) \
    const FieldInfoDescriptor _fieldInfos_##struc[] = {

// field info sequence terminator
#define STRUCT_INFO_END(struc) \
        { 0, 0, 0, eFieldVoid, false, FieldInfoDescriptor::Extension() } \
	};

#define FIELD(struc, field, ...) { \
    /* name */      #field, \
    /* size */      sizeof(decltype(struc::field)), \
    /* offset */    offsetof(struc, field), \
    /* type */      FullFieldInfoHelper<std::remove_cv<decltype(struc::field)>::type>::type(), \
    /* nullable */  FullFieldInfoHelper<std::remove_cv<decltype(struc::field)>::type>::nullable(), \
    /* extension */ FullFieldInfoHelper<std::remove_cv<decltype(struc::field)>::type>::extension(__VA_ARGS__) \
    },

#define REFLECTIBLE_DESCRIPTOR(struc) _descriptor_##struc
#define REFLECTIBLE_DESCRIPTOR_DECLARE(struc) extern const MetaInfoDescriptor _descriptor_##struc;

#define REFLECTIBLE_F(struc) \
    const MetaInfoDescriptor _descriptor_##struc = { #struc, sizeof(struc), nullptr, _fieldInfos_##struc, nullptr }; \

#define REFLECTIBLE_DERIVED_F(struc, superStruc) \
    const MetaInfoDescriptor _descriptor_##struc = { #struc, sizeof(struc), &REFLECTIBLE_DESCRIPTOR(superStruc), _fieldInfos_##struc, nullptr  }; \

#define REFLECTIBLE_M(struc) \
    const MetaInfoDescriptor _descriptor_##struc = { #struc, sizeof(struc), nullptr, nullptr, _methodInfos_##struc }; \

#define REFLECTIBLE_DERIVED_M(struc, superStruc) \
    const MetaInfoDescriptor _descriptor_##struc = { #struc, sizeof(struc), &REFLECTIBLE_DESCRIPTOR(superStruc), nullptr, _methodInfos_##struc }; \

#define REFLECTIBLE_FM(struc) \
    const MetaInfoDescriptor _descriptor_##struc = { #struc, sizeof(struc), nullptr, _fieldInfos_##struc, _methodInfos_##struc }; \

#define REFLECTIBLE_DERIVED_FM(struc, superStruc) \
    const MetaInfoDescriptor _descriptor_##struc = { #struc, sizeof(struc), &REFLECTIBLE_DESCRIPTOR(superStruc), _fieldInfos_##struc, _methodInfos_##struc }; \

#define ENUM_INFO_BEGIN(_enum, type, def) \
    extern const EnumValueInfoDescriptor _enumValueInfos_##_enum[]; \
    const EnumInfoDescriptor _enumInfo_##_enum = { type, #_enum, (uint32_t)def, _enumValueInfos_##_enum }; \
    const EnumValueInfoDescriptor _enumValueInfos_##_enum[] = {

#define ENUM_INFO_END(_enum) \
		{ nullptr, 0 } \
	};

#define ENUM_INFO(_enum) _enumInfo_##_enum
#define ENUM_INFO_DECLARE(_enum) extern const EnumInfoDescriptor _enumInfo_##_enum;

#define VALUE_INFO(name) \
	{ #name, (uint32_t)name },

#endif // METAINFO_H
