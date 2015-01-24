#include "ObjectTest.h"
#include "Object.h"
#include <string>
#include <memory>
#include "Variant.h"

using namespace metacpp;

enum EEnumTest
{
	eEnumValue0,
	eEnumValue1,
	eEnumValueUnk = -1
};

STRUCT_INFO_DECLARE(TestStruct)
STRUCT_INFO_DECLARE(TestBaseStruct)
STRUCT_INFO_DECLARE(TestSubStruct)

struct TestSubStruct : public Object
{
    String name;

    META_INFO_DECLARE(TestSubStruct)
};

struct TestBaseStruct : public Object
{
    int id;

    META_INFO_DECLARE(TestBaseStruct)
};

struct TestStruct : public TestBaseStruct
{
	EEnumTest enumValue;
	bool boolValue;
	int intValue;
	uint32_t uintValue;
    double doubleValue;
    String strValue;
	TestSubStruct substruct;
    Array<TestSubStruct> arrValue;
    DateTime datetimeValue;

    Nullable<EEnumTest> optEnumValue;
    Nullable<bool> optBoolValue;
    Nullable<int> optIntValue;
    Nullable<uint32_t> optUintValue;
    Nullable<float> optFloatValue;

    META_INFO_DECLARE(TestStruct)
};

ENUM_INFO_BEGIN(EEnumTest, eEnumSimple, eEnumValueUnk)
	VALUE_INFO(eEnumValue0)
	VALUE_INFO(eEnumValue1)
	VALUE_INFO(eEnumValueUnk)
ENUM_INFO_END(EEnumTest)

STRUCT_INFO_DERIVED_BEGIN(TestStruct, TestBaseStruct)
	FIELD_INFO(TestStruct, enumValue, &ENUM_INFO(EEnumTest))
	FIELD_INFO(TestStruct, boolValue, true)
	FIELD_INFO(TestStruct, intValue, -1)
	FIELD_INFO(TestStruct, uintValue, 123154)
    FIELD_INFO(TestStruct, doubleValue, 25.0)
	FIELD_INFO(TestStruct, strValue, "testValue")
	FIELD_INFO(TestStruct, substruct)
    FIELD_INFO(TestStruct, arrValue)

    FIELD_INFO(TestStruct, optEnumValue, &ENUM_INFO(EEnumTest))
    FIELD_INFO(TestStruct, optBoolValue, true)
    FIELD_INFO(TestStruct, optIntValue, -1)
    FIELD_INFO(TestStruct, optUintValue, 123154)
    FIELD_INFO(TestStruct, optFloatValue, eOptional)

    FIELD_INFO(TestStruct, datetimeValue)
STRUCT_INFO_END(TestStruct)

META_INFO(TestStruct)

STRUCT_INFO_BEGIN(TestSubStruct)
	FIELD_INFO(TestSubStruct, name, "TestSubStruct")
STRUCT_INFO_END(TestSubStruct)

META_INFO(TestSubStruct)


STRUCT_INFO_BEGIN(TestBaseStruct)
    FIELD_INFO(TestBaseStruct, id, eRequired)
STRUCT_INFO_END(TestBaseStruct)

META_INFO(TestBaseStruct)


void ObjectTest::testMetaInfo()
{
	TestStruct t;
    EXPECT_EQ(std::string(t.metaObject()->name()), "TestStruct");
    EXPECT_EQ(t.metaObject()->totalFields(), 15);
    ASSERT_EQ(t.metaObject()->superMetaObject()->name(), String("TestBaseStruct"));
    ASSERT_EQ(std::string(t.metaObject()->field(1)->name()), "enumValue");
    ASSERT_EQ(t.metaObject()->field(1)->type(), eFieldEnum);
    ASSERT_EQ(reinterpret_cast<const MetaFieldEnum *>(t.metaObject()->field(1))->defaultValue(), eEnumValueUnk);
    ASSERT_EQ(std::string(reinterpret_cast<const MetaFieldEnum *>(t.metaObject()->field(1))->enumName()), "EEnumTest");
    ASSERT_EQ(reinterpret_cast<const MetaFieldEnum *>(t.metaObject()->field(1))->enumType(), eEnumSimple);
    ASSERT_EQ(std::string(t.metaObject()->field(2)->name()), "boolValue");
    ASSERT_EQ(t.metaObject()->field(2)->type(), eFieldBool);
    ASSERT_EQ(std::string(t.metaObject()->field(3)->name()), "intValue");
    ASSERT_EQ(t.metaObject()->field(3)->type(), eFieldInt);
    ASSERT_EQ(std::string(t.metaObject()->field(4)->name()), "uintValue");
    ASSERT_EQ(t.metaObject()->field(4)->type(), eFieldUint);
    ASSERT_EQ(std::string(t.metaObject()->field(5)->name()), "doubleValue");
    ASSERT_EQ(t.metaObject()->field(5)->type(), eFieldDouble);
    ASSERT_EQ(std::string(t.metaObject()->field(6)->name()), "strValue");
    ASSERT_EQ(t.metaObject()->field(6)->type(), eFieldString);
    ASSERT_EQ(std::string(t.metaObject()->field(7)->name()), "substruct");
    ASSERT_EQ(t.metaObject()->field(7)->type(), eFieldObject);

}

void ObjectTest::testInitVisitor()
{
	TestStruct t;
	t.init();
    EXPECT_EQ(t.enumValue, eEnumValueUnk);
    EXPECT_EQ(t.boolValue, true);
    EXPECT_EQ(t.intValue, -1);
    EXPECT_EQ(t.uintValue, 123154);
    EXPECT_EQ(t.doubleValue, 25.0);
    EXPECT_EQ(t.strValue, "testValue");
    EXPECT_EQ(t.substruct.name, "TestSubStruct");
    EXPECT_TRUE(t.arrValue.empty());
    EXPECT_EQ(*t.optEnumValue, eEnumValueUnk);
    EXPECT_EQ(*t.optBoolValue, true);
    EXPECT_EQ(*t.optIntValue, -1);
    EXPECT_EQ(*t.optUintValue, 123154);
    EXPECT_FALSE(t.optFloatValue);
}

void ObjectTest::testSerialization()
{
	TestStruct t, t2;
    t.init();
    t.id = 123;
	t.enumValue = eEnumValue1;
	t.boolValue = false;
	t.intValue = 123414;
	t.uintValue = -1239;
    t.doubleValue = 1231.123f;
	t.strValue = "abdasdc";
	t.substruct.name = "1231";
    TestSubStruct item;
    item.name = "12"; t.arrValue.push_back(item);
    item.name = "asdj"; t.arrValue.push_back(item);
    item.name = ""; t.arrValue.push_back(item);
    t.optFloatValue = 2.5;
    t.datetimeValue = DateTime::fromString("0100-10-12 00:00:00");
    t2.fromString(t.toString());
    EXPECT_EQ(t.id, t2.id);
    EXPECT_EQ(t.enumValue, t2.enumValue);
    EXPECT_EQ(t.boolValue, t2.boolValue);
    EXPECT_EQ(t.intValue, t2.intValue);
    EXPECT_EQ(t.uintValue, t2.uintValue);
    EXPECT_EQ(t.doubleValue, t2.doubleValue);
    EXPECT_EQ(t.strValue, t2.strValue);
    EXPECT_EQ(t.substruct.name, t2.substruct.name);
    EXPECT_EQ(t.arrValue.size(), t2.arrValue.size());
	for (size_t i = 0; i < t.arrValue.size(); ++i)
        ASSERT_EQ(t.arrValue[i].name, t2.arrValue[i].name);

    EXPECT_EQ(*t2.optEnumValue, eEnumValueUnk);
    EXPECT_EQ(*t2.optBoolValue, true);
    EXPECT_EQ(*t2.optIntValue, -1);
    EXPECT_EQ(*t2.optUintValue, 123154);
    EXPECT_EQ(t.optFloatValue, t2.optFloatValue);
    EXPECT_EQ(t.datetimeValue, t2.datetimeValue);
}

TEST_F(ObjectTest, MetaInfoTest)
{
	testMetaInfo();
}

TEST_F(ObjectTest, InitVisitorTest)
{
	testInitVisitor();
}

TEST_F(ObjectTest, SerializationTest)
{
	testSerialization();
}

namespace metacpp
{
    class MyObject : public Object
    {
        int m_x;
    public:

        MyObject(int x = 0)
            : m_x(x)
        {

        }

        static int foo(int a, float b)
        {
            return a * b;
        }

        static void foo1()
        {
        }

        float bar(int a, double b) const
        {
            return m_x * a * b;
        }

        void bar1(int newX)
        {
            m_x = newX;
        }

        int x() const { return m_x; }

        META_INFO_DECLARE(MyObject)

    };

    STRUCT_INFO_DERIVED_BEGIN(MyObject, Object)
    STRUCT_INFO_END(MyObject)

    META_INFO(MyObject)

    class BindArgumentException : public std::invalid_argument
    {
    public:
        BindArgumentException(const char *errorMsg = "Cannot bind argument")
            : std::invalid_argument(errorMsg)
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
            static void unpack_arguments(std::tuple<TArgs...>& t, const VariantArray& argList)
            {
                typedef typename std::tuple_element<N, std::tuple<TArgs...>>::type ArgType;
                try
                {
                    std::get<N>(t) = argList[N].value<ArgType>();
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
            static void unpack_arguments(std::tuple<TArgs...>&, const VariantArray&)
            {
            }
        };
    }

    class InvokeHelperBase
    {
    public:
        virtual ~InvokeHelperBase() { }
        virtual Variant invoke(const VariantArray& argList) const = 0;
    };

    template<typename TRes, typename... TArgs>
    class FunctionInvokeHelper : public InvokeHelperBase
    {
    private:

        template<typename Q = TRes>
        typename std::enable_if<std::is_same<Q, void>::value, Variant>::type invokeHelper(const VariantArray &argList) const
        {
            doInvoke(argList);
            return Variant();
        }

        template<typename Q = TRes>
        typename std::enable_if<!std::is_same<Q, void>::value, Variant>::type invokeHelper(const VariantArray &argList) const
        {
            return Variant(doInvoke(argList));
        }

    public:
        typedef TRes (*TFunction)(TArgs...);

        explicit FunctionInvokeHelper(TFunction func)
            : m_function(func)
        {
        }

        TRes doInvoke(const VariantArray& argList) const
        {
            typedef std::tuple<TArgs...> ttype;
            ttype args;
            if (sizeof...(TArgs) != argList.size())
                throw BindArgumentException(String("Invalid number of arguments, " +
                                                   String::fromValue(sizeof...(TArgs)) + " expected").c_str());
            detail::unpack_impl<0, 0 == sizeof...(TArgs), TArgs...>::unpack_arguments(args, argList);
            return detail::fcall_impl<TFunction, TRes, ttype, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>
                    ::call(m_function, std::forward<ttype>(args));
        }

        Variant invoke(const VariantArray& argList) const override
        {
            return invokeHelper(argList);
        }

    private:
        TFunction m_function;
    };

    template<typename TRes, typename TObj, typename... TArgs>
    class MethodInvokeHelper : public InvokeHelperBase
    {
    private:

        template<typename Q = TRes>
        typename std::enable_if<std::is_same<Q, void>::value, Variant>::type invokeHelper(const VariantArray &argList) const
        {
            doInvoke(argList);
            return Variant();
        }

        template<typename Q = TRes>
        typename std::enable_if<!std::is_same<Q, void>::value, Variant>::type invokeHelper(const VariantArray &argList) const
        {
            return Variant(doInvoke(argList));
        }

    public:
        typedef TRes (TObj::*TFunction)(TArgs...);

        MethodInvokeHelper(TObj *obj, TFunction function)
            : m_obj(obj), m_method(function)
        {
        }

        TRes doInvoke(const VariantArray& argList) const
        {
            typedef std::tuple<TArgs...> ttype;
            ttype args;
            if (sizeof...(TArgs) != argList.size())
                throw BindArgumentException(String("Invalid number of arguments, " +
                                                   String::fromValue(sizeof...(TArgs)) + " expected").c_str());
            detail::unpack_impl<0, 0 == sizeof...(TArgs), TArgs...>::unpack_arguments(args, argList);
            return detail::mcall_impl<TFunction, TRes, TObj, ttype, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>
                    ::call(m_method, m_obj, std::forward<ttype>(args));
        }

        Variant invoke(const VariantArray& argList) const override
        {
            return invokeHelper(argList);
        }

    private:
        TObj *m_obj;
        TFunction m_method;
    };

    template<typename TRes, typename TObj, typename... TArgs>
    class ConstMethodInvokeHelper : public InvokeHelperBase
    {
    private:

        template<typename Q = TRes>
        typename std::enable_if<std::is_same<Q, void>::value, Variant>::type invokeHelper(const VariantArray &argList) const
        {
            doInvoke(argList);
            return Variant();
        }

        template<typename Q = TRes>
        typename std::enable_if<!std::is_same<Q, void>::value, Variant>::type invokeHelper(const VariantArray &argList) const
        {
            return Variant(doInvoke(argList));
        }

    public:
        typedef TRes (TObj::*TFunction)(TArgs...) const;

        ConstMethodInvokeHelper(const TObj *obj, TFunction function)
            : m_obj(obj), m_method(function)
        {
        }

        TRes doInvoke(const VariantArray& argList) const
        {
            typedef std::tuple<TArgs...> ttype;
            ttype args;
            if (sizeof...(TArgs) != argList.size())
                throw BindArgumentException(String("Invalid number of arguments, " +
                                                   String::fromValue(sizeof...(TArgs)) + " expected").c_str());
            detail::unpack_impl<0, 0 == sizeof...(TArgs), TArgs...>::unpack_arguments(args, argList);
            return detail::mcall_impl<TFunction, TRes, const TObj, ttype, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>
                    ::call(m_method, m_obj, std::forward<ttype>(args));
        }

        Variant invoke(const VariantArray& argList) const override
        {
            return invokeHelper(argList);
        }

    private:
        const TObj *m_obj;
        TFunction m_method;
    };

    template<typename TRes, typename... TArgs>
    std::unique_ptr<InvokeHelperBase> createInvokeHelper(TRes (*func)(TArgs...))
    {
        return std::move(std::unique_ptr<InvokeHelperBase>(new FunctionInvokeHelper<TRes, TArgs...>(func)));
    }

    template<typename TRes, typename TObj, typename... TArgs>
    std::unique_ptr<InvokeHelperBase> createInvokeHelper(TObj *obj, TRes (TObj::*function)(TArgs...))
    {
        return std::move(std::unique_ptr<InvokeHelperBase>(new MethodInvokeHelper<TRes, TObj, TArgs...>(obj, function)));
    }

    template<typename TRes, typename TObj, typename... TArgs>
    std::unique_ptr<InvokeHelperBase> createInvokeHelper(const TObj *obj, TRes (TObj::*function)(TArgs...) const)
    {
        return std::move(std::unique_ptr<InvokeHelperBase>(new ConstMethodInvokeHelper<TRes, TObj, TArgs...>(obj, function)));
    }

    TEST_F(ObjectTest, invokeStaticTest)
    {
        VariantArray argList = { Variant(12), Variant(2.5f) };
        ASSERT_EQ(30, createInvokeHelper(MyObject::foo)->invoke(argList).value<int>());
    }

    TEST_F(ObjectTest, invalidInvokeTest)
    {
        VariantArray argList;
        EXPECT_THROW(createInvokeHelper(MyObject::foo)->invoke(argList).value<int>(), BindArgumentException);
    }

    TEST_F(ObjectTest, invalidArgumentTest)
    {
        VariantArray argList = { Variant(12), Variant("2.5f") };
        EXPECT_THROW(createInvokeHelper(MyObject::foo)->invoke(argList).value<int>(), BindArgumentException);
    }

    TEST_F(ObjectTest, invokeVoidStaticTest)
    {
        ASSERT_FALSE(createInvokeHelper(MyObject::foo1)->invoke(VariantArray()).valid());
    }

    TEST_F(ObjectTest, invokeConstMethod)
    {
        VariantArray argList = { Variant(12), Variant(2.5) };
        MyObject obj(2);
        ASSERT_EQ(60, createInvokeHelper(&obj, &MyObject::bar)->invoke(argList).value<int>());
    }

    TEST_F(ObjectTest, invokeMethod)
    {
        VariantArray argList = { Variant(12) };
        MyObject obj;
        createInvokeHelper(&obj, &MyObject::bar1)->invoke(argList);
        ASSERT_EQ(12, obj.x());
    }
}
