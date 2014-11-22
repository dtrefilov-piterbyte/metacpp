#include "ObjectTest.h"
#include "Object.h"
#include <string>
#include <memory>
#include "JsonSerializerVisitor.h"

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
	float floatValue;
    String strValue;
	TestSubStruct substruct;
    Array<String> arrValue;

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
	FIELD_INFO(TestStruct, floatValue, 25.0f)
	FIELD_INFO(TestStruct, strValue, "testValue")
	FIELD_INFO(TestStruct, substruct)
	FIELD_INFO(TestStruct, arrValue)

    FIELD_INFO(TestStruct, optEnumValue, &ENUM_INFO(EEnumTest))
    FIELD_INFO(TestStruct, optBoolValue, true)
    FIELD_INFO(TestStruct, optIntValue, -1)
    FIELD_INFO(TestStruct, optUintValue, 123154)
    FIELD_INFO(TestStruct, optFloatValue, eOptional)
STRUCT_INFO_END(TestStruct)

META_INFO(TestStruct)

STRUCT_INFO_BEGIN(TestSubStruct)
	FIELD_INFO(TestSubStruct, name, "TestSubStruct")
STRUCT_INFO_END(TestSubStruct)

META_INFO(TestSubStruct);


STRUCT_INFO_BEGIN(TestBaseStruct)
    FIELD_INFO(TestBaseStruct, id, eRequired)
STRUCT_INFO_END(TestBaseStruct)

META_INFO(TestBaseStruct)


void ObjectTest::testMetaInfo()
{
	TestStruct t;
    EXPECT_EQ(std::string(t.metaObject()->className()), "TestStruct") << "invalid className";
    EXPECT_EQ(t.metaObject()->totalFields(), 14) << "invalid number of fields";
	auto sd = t.metaObject()->structDescriptor();
    EXPECT_EQ(sd->m_superDescriptor, &STRUCT_INFO(TestBaseStruct)) << "invalid superclass";
    EXPECT_EQ(std::string(t.metaObject()->fieldDescriptor(1)->m_pszName), "enumValue") << "invalid enum type";
    EXPECT_EQ(t.metaObject()->fieldDescriptor(1)->m_eType, eFieldEnum) << "invalid enum type";
    EXPECT_EQ(t.metaObject()->fieldDescriptor(1)->valueInfo.ext.m_enum.enumInfo->m_defaultValue, eEnumValueUnk) << "invalid enum default type";
    EXPECT_EQ(std::string(t.metaObject()->fieldDescriptor(1)->valueInfo.ext.m_enum.enumInfo->m_enumName), "EEnumTest") << "invalid enum name";
    EXPECT_EQ(t.metaObject()->fieldDescriptor(1)->valueInfo.ext.m_enum.enumInfo->m_type, eEnumSimple) << "invalid enum type";
    EXPECT_EQ(std::string(t.metaObject()->fieldDescriptor(2)->m_pszName), "boolValue") << "invalid bool name";
    EXPECT_EQ(t.metaObject()->fieldDescriptor(2)->m_eType, eFieldBool) << "invalid bool type";
    EXPECT_EQ(std::string(t.metaObject()->fieldDescriptor(3)->m_pszName), "intValue") << "invalid int name";
    EXPECT_EQ(t.metaObject()->fieldDescriptor(3)->m_eType, eFieldInt) << "invalid int type";
    EXPECT_EQ(std::string(t.metaObject()->fieldDescriptor(4)->m_pszName), "uintValue") << "invalid uint name";
    EXPECT_EQ(t.metaObject()->fieldDescriptor(4)->m_eType, eFieldUint) << "invalid uint type";
    EXPECT_EQ(std::string(t.metaObject()->fieldDescriptor(5)->m_pszName), "floatValue") << "invalid float name";
    EXPECT_EQ(t.metaObject()->fieldDescriptor(5)->m_eType, eFieldFloat) << "invalid float type";
    EXPECT_EQ(std::string(t.metaObject()->fieldDescriptor(6)->m_pszName), "strValue") << "invalid string name";
    EXPECT_EQ(t.metaObject()->fieldDescriptor(6)->m_eType, eFieldString) << "invalid string type";
    EXPECT_EQ(std::string(t.metaObject()->fieldDescriptor(7)->m_pszName), "substruct") << "invalid object name";
    EXPECT_EQ(t.metaObject()->fieldDescriptor(7)->m_eType, eFieldObject) << "invalid object type";

}

void ObjectTest::testInitVisitor()
{
	TestStruct t;
	t.init();
    EXPECT_EQ(t.enumValue, eEnumValueUnk) << "t.enumValue incorrectly initilized";
    EXPECT_EQ(t.boolValue, true) << "t.boolValue incorrectly initilized";
    EXPECT_EQ(t.intValue, -1) << "t.intValue incorrectly initilized";
    EXPECT_EQ(t.uintValue, 123154) << "t.uintValue incorrectly initilized";
    EXPECT_EQ(t.floatValue, 25.0f) << "t.intValue incorrectly initilized";
    EXPECT_EQ(t.strValue, "testValue") << "t.strValue incorrectly initilized";
    EXPECT_EQ(t.substruct.name, "TestSubStruct");
    EXPECT_TRUE(t.arrValue.empty()) << "t.arrValue incorrectly initialized";
    EXPECT_EQ(*t.optEnumValue, eEnumValueUnk) << "t.optEnumValue incorrectly initialized";
    EXPECT_EQ(*t.optBoolValue, true) << "t.optBoolValue incorrectly initialized";
    EXPECT_EQ(*t.optIntValue, -1) << "t.optIntValue incorrectly initialized";
    EXPECT_EQ(*t.optUintValue, 123154) << "t.optUintValue incorrectly initialized";
    ASSERT_FALSE(t.optFloatValue) << "t.optFloatValue incorrectly initialized";
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
	t.floatValue = 1231.123f;
	t.strValue = "abdasdc";
	t.substruct.name = "1231";
    t.arrValue.push_back("12");
    t.arrValue.push_back("asdj");
    t.arrValue.push_back("");
    t.optFloatValue = 2.5;
    t2.fromString(t.toString());
    EXPECT_EQ(t.id, t2.id);
    EXPECT_EQ(t.enumValue, t2.enumValue);
    EXPECT_EQ(t.boolValue, t2.boolValue);
    EXPECT_EQ(t.intValue, t2.intValue);
    EXPECT_EQ(t.uintValue, t2.uintValue);
    EXPECT_EQ(t.floatValue, t2.floatValue);
    EXPECT_EQ(t.strValue, t2.strValue);
    EXPECT_EQ(t.substruct.name, t2.substruct.name);
    EXPECT_EQ(t.arrValue.size(), t2.arrValue.size());
	for (size_t i = 0; i < t.arrValue.size(); ++i)
        ASSERT_EQ(t.arrValue[i], t2.arrValue[i]);

    EXPECT_EQ(*t2.optEnumValue, eEnumValueUnk);
    EXPECT_EQ(*t2.optBoolValue, true);
    EXPECT_EQ(*t2.optIntValue, -1);
    EXPECT_EQ(*t2.optUintValue, 123154);
    EXPECT_EQ(t.optFloatValue, t2.optFloatValue);
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
