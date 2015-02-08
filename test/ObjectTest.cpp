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

REFLECTIBLE_DESCRIPTOR_DECLARE(TestStruct)
REFLECTIBLE_DESCRIPTOR_DECLARE(TestBaseStruct)
REFLECTIBLE_DESCRIPTOR_DECLARE(TestSubStruct)

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

STRUCT_INFO_BEGIN(TestStruct)
    FIELD(TestStruct, enumValue, &ENUM_INFO(EEnumTest))
    FIELD(TestStruct, boolValue, true)
    FIELD(TestStruct, intValue, -1)
    FIELD(TestStruct, uintValue, 123154)
    FIELD(TestStruct, doubleValue, 25.0)
    FIELD(TestStruct, strValue, "testValue")
    FIELD(TestStruct, substruct)
    FIELD(TestStruct, arrValue)

    FIELD(TestStruct, optEnumValue, &ENUM_INFO(EEnumTest))
    FIELD(TestStruct, optBoolValue, true)
    FIELD(TestStruct, optIntValue, -1)
    FIELD(TestStruct, optUintValue, 123154)
    FIELD(TestStruct, optFloatValue, eOptional)

    FIELD(TestStruct, datetimeValue)
STRUCT_INFO_END(TestStruct)

REFLECTIBLE_DERIVED_F(TestStruct, TestBaseStruct)

META_INFO(TestStruct)

STRUCT_INFO_BEGIN(TestSubStruct)
    FIELD(TestSubStruct, name, "TestSubStruct")
STRUCT_INFO_END(TestSubStruct)

REFLECTIBLE_F(TestSubStruct)

META_INFO(TestSubStruct)


STRUCT_INFO_BEGIN(TestBaseStruct)
    FIELD(TestBaseStruct, id, eRequired)
STRUCT_INFO_END(TestBaseStruct)

REFLECTIBLE_F(TestBaseStruct)

META_INFO(TestBaseStruct)


void ObjectTest::testMetaInfo()
{
	TestStruct t;
    ASSERT_EQ(std::string(t.metaObject()->name()), "TestStruct");
    ASSERT_EQ(t.metaObject()->totalFields(), 15);
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
#ifdef HAVE_JSONCPP
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
#endif
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

TEST_F(ObjectTest, TestGetFieldProperty)
{
    TestStruct s;
    s.init();
    ASSERT_EQ(s.getProperty("uintValue").value<uint32_t>(), 123154);
}

TEST_F(ObjectTest, TestGetInvalidProperty)
{
    TestStruct s;
    s.init();
    ASSERT_FALSE(s.getProperty("asld;kasldk").valid());
}

TEST_F(ObjectTest, TestDynamicProperty)
{
    TestStruct s;
    s.init();
    s.setProperty("newProp", "value");
    ASSERT_EQ(s.getProperty("newProp").value<String>(), "value");
}

namespace
{
    using namespace metacpp;
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

        static void foo()
        {
        }

        float bar(int a, double b) const
        {
            return m_x * a * b;
        }

        void bar(int newX)
        {
            m_x = newX;
        }

        String bar(String arg)
        {
            return arg + String::fromValue(m_x);
        }

        void test(Variant)
        {
        }

        int x() const { return m_x; }

        META_INFO_DECLARE(MyObject)

    };

    METHOD_INFO_BEGIN(MyObject)
        SIGNATURE_METHOD(MyObject, foo, int (*)(int, float))
        SIGNATURE_METHOD(MyObject, foo, void (*)(void))
        SIGNATURE_METHOD(MyObject, bar, float (MyObject::*)(int, double) const)
        SIGNATURE_METHOD(MyObject, bar, void (MyObject::*)(int))
        SIGNATURE_METHOD(MyObject, bar, String (MyObject::*)(String))
        METHOD(MyObject, test)
    METHOD_INFO_END(MyObject)

    REFLECTIBLE_DERIVED_M(MyObject, Object)

    META_INFO(MyObject)

    TEST_F(ObjectTest, invokeStaticTest)
    {
        ASSERT_EQ(30, MyObject::staticMetaObject()->invoke<int>("foo", 12, 2.5f));
    }

    TEST_F(ObjectTest, invokeVoidStaticTest)
    {
        ASSERT_NO_THROW(MyObject::staticMetaObject()->invoke<void>("foo"));
    }

    TEST_F(ObjectTest, invalidArgumentTest)
    {
        ASSERT_THROW(MyObject::staticMetaObject()->invoke<int>("foo", 12, "2.5f"), MethodNotFoundException);
    }

    TEST_F(ObjectTest, invokeConstMethod)
    {
        const MyObject obj(2);
        ASSERT_EQ(60, obj.invoke<int>("bar", 12, 2.5));
    }

    TEST_F(ObjectTest, invokeMethod)
    {
        MyObject obj(2);
        ASSERT_NO_THROW(obj.invoke<void>("bar", 12));
        ASSERT_EQ(obj.x(), 12);
    }

    TEST_F(ObjectTest, invokeOverloadedMethod)
    {
        MyObject obj(2);
        ASSERT_EQ(obj.invoke<String>("bar", "prefix_"), "prefix_2");
    }

    TEST_F(ObjectTest, invokeFailureByConstness)
    {
        const MyObject obj(2);
        ASSERT_THROW(obj.invoke<void>("test", Variant()), MethodNotFoundException);
    }
}
