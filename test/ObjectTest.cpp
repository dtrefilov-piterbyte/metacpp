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

    TestSubStruct(const String& name = String())
        : name(name)
    {
    }

    TestSubStruct(const TestSubStruct&)=default;

    TestSubStruct& operator=(const TestSubStruct&)=default;

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
    Variant variantValue;

    Nullable<EEnumTest> optEnumValue;
    Nullable<bool> optBoolValue;
    Nullable<int> optIntValue;
    Nullable<uint32_t> optUintValue;
    Nullable<float> optFloatValue;
    Nullable<DateTime> optDateTimeValue;
    Nullable<Variant> optVariantValue;

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
    FIELD(TestStruct, datetimeValue)
    FIELD(TestStruct, variantValue)

    FIELD(TestStruct, optEnumValue, &ENUM_INFO(EEnumTest))
    FIELD(TestStruct, optBoolValue, true)
    FIELD(TestStruct, optIntValue, -1)
    FIELD(TestStruct, optUintValue, 123154)
    FIELD(TestStruct, optFloatValue, eOptional)
    FIELD(TestStruct, optDateTimeValue)
    FIELD(TestStruct, optVariantValue)

STRUCT_INFO_END(TestStruct)

REFLECTIBLE_DERIVED_F(TestStruct, TestBaseStruct)

META_INFO(TestStruct)

STRUCT_INFO_BEGIN(TestSubStruct)
    FIELD(TestSubStruct, name, "TestSubStruct")
STRUCT_INFO_END(TestSubStruct)

METHOD_INFO_BEGIN(TestSubStruct)
    CONSTRUCTOR(TestSubStruct)
    CONSTRUCTOR(TestSubStruct, String)
METHOD_INFO_END(TestSubStruct)

REFLECTIBLE_FM(TestSubStruct)

META_INFO(TestSubStruct)


STRUCT_INFO_BEGIN(TestBaseStruct)
    FIELD(TestBaseStruct, id, eRequired)
STRUCT_INFO_END(TestBaseStruct)

REFLECTIBLE_F(TestBaseStruct)

META_INFO(TestBaseStruct)

TEST_F(ObjectTest, MetaInfoTest)
{
    TestStruct t;

    auto testField = [&](int i, const String& name, EFieldType type, bool nullable,
            size_t size, std::ptrdiff_t offset) {
        EXPECT_EQ(t.metaObject()->field(i)->name(), name);
        EXPECT_EQ(t.metaObject()->field(i)->type(), type);
        EXPECT_EQ(t.metaObject()->field(i)->nullable(), nullable);
        EXPECT_EQ(t.metaObject()->field(i)->size(), size);
        EXPECT_EQ(t.metaObject()->field(i)->offset(), offset);
    };

    EXPECT_EQ(String(t.metaObject()->name()), "TestStruct");
    EXPECT_EQ(t.metaObject()->totalFields(), 18);
    EXPECT_EQ(String(t.metaObject()->superMetaObject()->name()), "TestBaseStruct");

    testField(0, "id", eFieldInt, false, sizeof(int), offsetof(TestStruct, id));
    testField(1, "enumValue", eFieldEnum, false, sizeof(EEnumTest), offsetof(TestStruct, enumValue));
    EXPECT_EQ(reinterpret_cast<const MetaFieldEnum *>(t.metaObject()->field(1))->defaultValue(), eEnumValueUnk);
    EXPECT_EQ(String(reinterpret_cast<const MetaFieldEnum *>(t.metaObject()->field(1))->enumName()), "EEnumTest");
    EXPECT_EQ(reinterpret_cast<const MetaFieldEnum *>(t.metaObject()->field(1))->enumType(), eEnumSimple);
    testField(2, "boolValue", eFieldBool, false, sizeof(bool), offsetof(TestStruct, boolValue));
    testField(3, "intValue", eFieldInt, false, sizeof(int32_t), offsetof(TestStruct, intValue));
    testField(4, "uintValue", eFieldUint, false, sizeof(uint32_t), offsetof(TestStruct, uintValue));
    testField(5, "doubleValue", eFieldDouble, false, sizeof(double), offsetof(TestStruct, doubleValue));
    testField(6, "strValue", eFieldString, false, sizeof(String), offsetof(TestStruct, strValue));
    testField(7, "substruct", eFieldObject, false, sizeof(TestSubStruct), offsetof(TestStruct, substruct));
    testField(8, "arrValue", eFieldArray, false, sizeof(Array<TestSubStruct>), offsetof(TestStruct, arrValue));
    testField(9, "datetimeValue", eFieldDateTime, false, sizeof(DateTime), offsetof(TestStruct, datetimeValue));
    testField(10, "variantValue", eFieldVariant, false, sizeof(Variant), offsetof(TestStruct, variantValue));
    testField(11, "optEnumValue", eFieldEnum, true, sizeof(Nullable<EEnumTest>), offsetof(TestStruct, optEnumValue));
    EXPECT_EQ(reinterpret_cast<const MetaFieldEnum *>(t.metaObject()->field(11))->defaultValue(), eEnumValueUnk);
    EXPECT_EQ(String(reinterpret_cast<const MetaFieldEnum *>(t.metaObject()->field(11))->enumName()), "EEnumTest");
    EXPECT_EQ(reinterpret_cast<const MetaFieldEnum *>(t.metaObject()->field(11))->enumType(), eEnumSimple);
    testField(12, "optBoolValue", eFieldBool, true, sizeof(Nullable<bool>), offsetof(TestStruct, optBoolValue));
    testField(13, "optIntValue", eFieldInt, true, sizeof(Nullable<int>), offsetof(TestStruct, optIntValue));
    testField(14, "optUintValue", eFieldUint, true, sizeof(Nullable<uint32_t>), offsetof(TestStruct, optUintValue));
    testField(15, "optFloatValue", eFieldFloat, true, sizeof(Nullable<float>), offsetof(TestStruct, optFloatValue));
    testField(16, "optDateTimeValue", eFieldDateTime, true, sizeof(Nullable<DateTime>), offsetof(TestStruct, optDateTimeValue));
    testField(17, "optVariantValue", eFieldVariant, true, sizeof(Nullable<Variant>), offsetof(TestStruct, optVariantValue));

}

TEST_F(ObjectTest, InitVisitorTest)
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
    EXPECT_FALSE(t.variantValue.valid());
    EXPECT_FALSE(t.datetimeValue.valid());

    EXPECT_EQ(*t.optEnumValue, eEnumValueUnk);
    EXPECT_EQ(*t.optBoolValue, true);
    EXPECT_EQ(*t.optIntValue, -1);
    EXPECT_EQ(*t.optUintValue, 123154);
    EXPECT_FALSE(t.optFloatValue);
    EXPECT_FALSE(t.optDateTimeValue);
    EXPECT_FALSE(t.optVariantValue);
}

#ifdef HAVE_JSONCPP
TEST_F(ObjectTest, SerializationTest)
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
    t.arrValue.emplace_back("12");
    t.arrValue.emplace_back("asdj");
    t.arrValue.emplace_back("");
    t.optFloatValue = 2.5;
    t.datetimeValue = DateTime::fromString("1970-10-12 00:00:00");
    t.optVariantValue = 12;
    t2.fromJson(t.toJson());
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
    EXPECT_FALSE(t.variantValue.valid());

    EXPECT_EQ(*t2.optEnumValue, eEnumValueUnk);
    EXPECT_EQ(*t2.optBoolValue, true);
    EXPECT_EQ(*t2.optIntValue, -1);
    EXPECT_EQ(*t2.optUintValue, 123154);
    EXPECT_EQ(t.optFloatValue, t2.optFloatValue);
    EXPECT_EQ(t.datetimeValue, t2.datetimeValue);
    EXPECT_EQ(variant_cast<int>(*t.optVariantValue), variant_cast<int>(*t2.optVariantValue));
}

TEST_F(ObjectTest, SerializationTestVariantArray)
{
    TestStruct t, t2;
    t.init();
    t.variantValue = VariantArray { 12, "Test", DateTime::fromString("2021-10-5 00:12:42") };
    t2.fromJson(t.toJson());
    ASSERT_EQ(t2.variantValue.type(), eFieldArray);
    auto array = variant_cast<VariantArray>(t2.variantValue);
    ASSERT_EQ(array.size(), 3);
    EXPECT_EQ(variant_cast<int>(array[0]), 12);
    EXPECT_EQ(variant_cast<String>(array[1]), "Test");
    EXPECT_EQ(variant_cast<DateTime>(array[2]), DateTime::fromString("2021-10-5 00:12:42"));
}

TEST_F(ObjectTest, SerializationTestVariantObject)
{
    TestStruct t, t2;
    t.init();
    t.variantValue = TestSubStruct::staticMetaObject()->createInstance("Test");
    t2.fromJson(t.toJson(), { TestSubStruct::staticMetaObject() });
    ASSERT_EQ(t2.variantValue.type(), eFieldObject);
    EXPECT_EQ(variant_cast<TestSubStruct *>(t2.variantValue)->name, "Test");
}

TEST_F(ObjectTest, SerializationTestVariantObjectArray)
{
    TestStruct t, t2;
    t.init();
    t.variantValue = VariantArray {
            TestSubStruct::staticMetaObject()->createInstance("Object 1"),
            TestSubStruct::staticMetaObject()->createInstance("Object 2"),
            "String item"
    };
    t2.fromJson(t.toJson(), { TestSubStruct::staticMetaObject() });
    ASSERT_EQ(t2.variantValue.type(), eFieldArray);
    auto array = variant_cast<VariantArray>(t2.variantValue);
    ASSERT_EQ(array.size(), 3);
    ASSERT_EQ(array[0].type(), eFieldObject);
    ASSERT_EQ(array[1].type(), eFieldObject);
    ASSERT_EQ(array[2].type(), eFieldString);
    EXPECT_EQ(variant_cast<TestSubStruct *>(array[0])->name, "Object 1");
    EXPECT_EQ(variant_cast<TestSubStruct *>(array[1])->name, "Object 2");
    EXPECT_EQ(variant_cast<String>(array[2]), "String item");
}

TEST_F(ObjectTest, SerializationTestVariantNestedArray)
{
    TestStruct t;
    t.init();
    t.variantValue = VariantArray { VariantArray {1, "Test"} };
    EXPECT_ANY_THROW(t.toJson());
}

TEST_F(ObjectTest, SerializationTestVariantInvalid)
{
    TestStruct t, t2;
    t.init();
    t.variantValue = Variant();
    t2.fromJson(t.toJson());
    EXPECT_FALSE(t2.variantValue.valid());
}

TEST_F(ObjectTest, SerializationTestDateTimeValid)
{
    TestStruct t, t2;
    t.init();
    t.datetimeValue = DateTime::fromString("2012-12-12 01:21:21");
    t2.fromJson(t.toJson());
    EXPECT_EQ(t2.datetimeValue, t.datetimeValue);
}

TEST_F(ObjectTest, SerializationTestDateTimeInvalid)
{
    TestStruct t, t2;
    t.init();
    t.datetimeValue = DateTime();
    t2.fromJson(t.toJson());
    EXPECT_FALSE(t2.datetimeValue.valid());
}

#endif

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

        static int foo(int a, const float& b)
        {
            return static_cast<int>(a * b);
        }

        static void foo()
        {
        }

        float bar(int a, const double& b) const
        {
            return static_cast<float>(m_x * a * b);
        }

        void bar(int newX)
        {
            m_x = newX;
        }

        String bar(const String& arg)
        {
            return arg + String::fromValue(m_x);
        }

        String test(const Variant& v)
        {
            return String::fromValue(v);
        }

        static String objName(const MyObject *obj)
        {
            return obj->metaObject()->name();
        }

        int x() const { return m_x; }

        META_INFO_DECLARE(MyObject)

    };

    METHOD_INFO_BEGIN(MyObject)
        CONSTRUCTOR(MyObject)
        CONSTRUCTOR(MyObject, int)
        SIGNATURE_METHOD(MyObject, foo, int (*)(int, const float&))
        SIGNATURE_METHOD(MyObject, foo, void (*)(void))
        SIGNATURE_METHOD(MyObject, bar, float (MyObject::*)(int, const double&) const)
        SIGNATURE_METHOD(MyObject, bar, void (MyObject::*)(int))
        SIGNATURE_METHOD(MyObject, bar, String (MyObject::*)(const String&))
        METHOD(MyObject, test)
        METHOD(MyObject, objName)
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
        ASSERT_THROW(obj.invoke<String>("test", Variant()), MethodNotFoundException);
    }

    TEST_F(ObjectTest, invokeSuccessByMutable)
    {
        MyObject obj(2);
        ASSERT_EQ(obj.invoke<String>("test", 123), "123");
    }

    TEST_F(ObjectTest, invokeSuccessWithObjectParam)
    {
        ASSERT_EQ("MyObject", MyObject::staticMetaObject()->invoke<String>("objName", new MyObject()));
    }

    TEST_F(ObjectTest, createInstanceDefault)
    {
        MyObject *obj = dynamic_cast<MyObject *>(MyObject::staticMetaObject()->createInstance());
        EXPECT_EQ(obj->x(), 0);
        MyObject::staticMetaObject()->destroyInstance(obj);
    }

    TEST_F(ObjectTest, createInstanceParameter)
    {
        MyObject *obj = dynamic_cast<MyObject *>(MyObject::staticMetaObject()->createInstance(42));
        EXPECT_EQ(obj->x(), 42);
        MyObject::staticMetaObject()->destroyInstance(obj);
    }

    TEST_F(ObjectTest, createInstanceFailure)
    {
        EXPECT_THROW(MyObject::staticMetaObject()->createInstance("a string"),
                     MethodNotFoundException);
    }
}
