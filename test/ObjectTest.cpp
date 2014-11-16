#include "ObjectTest.h"
#include "Object.h"
#include <string>
#include <memory>
#include "JsonSerializerVisitor.h"

using namespace orm;

enum EEnumTest
{
	eEnumValue0,
	eEnumValue1,
	eEnumValueUnk = -1
};

STRUCT_INFO_DECLARE(TestStruct)
STRUCT_INFO_DECLARE(TestSubStruct)

struct TestSubStruct : public Object
{
    String name;

	PKMETA_INFO_DECLARE(TestSubStruct)
};

struct TestStruct : public Object
{
	EEnumTest enumValue;
	bool boolValue;
	int intValue;
	uint32_t uintValue;
	float floatValue;
    String strValue;
	TestSubStruct substruct;
    Array<String> arrValue;

	PKMETA_INFO_DECLARE(TestStruct)
};

ENUM_INFO_BEGIN(EEnumTest, eEnumSimple, eEnumValueUnk)
	VALUE_INFO(eEnumValue0)
	VALUE_INFO(eEnumValue1)
	VALUE_INFO(eEnumValueUnk)
ENUM_INFO_END(EEnumTest)

STRUCT_INFO_BEGIN(TestStruct)
	FIELD_INFO(TestStruct, enumValue, &ENUM_INFO(EEnumTest))
	FIELD_INFO(TestStruct, boolValue, true)
	FIELD_INFO(TestStruct, intValue, -1)
	FIELD_INFO(TestStruct, uintValue, 123154)
	FIELD_INFO(TestStruct, floatValue, 25.0f)
	FIELD_INFO(TestStruct, strValue, "testValue")
	FIELD_INFO(TestStruct, substruct)
	FIELD_INFO(TestStruct, arrValue)
STRUCT_INFO_END(TestStruct)

PKMETA_INFO(TestStruct)

STRUCT_INFO_BEGIN(TestSubStruct)
	FIELD_INFO(TestSubStruct, name, "TestSubStruct")
STRUCT_INFO_END(TestSubStruct)

PKMETA_INFO(TestSubStruct);

void ObjectTest::testMetaInfo()
{
	TestStruct t;
	ASSERT_EQ(std::string(t.metaObject()->classname()), "TestStruct") << "invalid className";
	ASSERT_EQ(t.metaObject()->totalFields(), 8) << "invalid number of fields";
	auto sd = t.metaObject()->structDescriptor();
	ASSERT_EQ(sd->m_superDescriptor, nullptr) << "invalid superclass";
	ASSERT_EQ(std::string(t.metaObject()->fieldDescriptor(0)->m_pszName), "enumValue") << "invalid enum type";
	ASSERT_EQ(t.metaObject()->fieldDescriptor(0)->m_eType, eFieldEnum) << "invalid enum type";
    ASSERT_EQ(t.metaObject()->fieldDescriptor(0)->valueInfo.ext.m_enum.enumInfo->m_defaultValue, eEnumValueUnk) << "invalid enum default type";
    ASSERT_EQ(std::string(t.metaObject()->fieldDescriptor(0)->valueInfo.ext.m_enum.enumInfo->m_enumName), "EEnumTest") << "invalid enum name";
    ASSERT_EQ(t.metaObject()->fieldDescriptor(0)->valueInfo.ext.m_enum.enumInfo->m_type, eEnumSimple) << "invalid enum type";
	ASSERT_EQ(std::string(t.metaObject()->fieldDescriptor(1)->m_pszName), "boolValue") << "invalid bool name";
	ASSERT_EQ(t.metaObject()->fieldDescriptor(1)->m_eType, eFieldBool) << "invalid bool type";
	ASSERT_EQ(std::string(t.metaObject()->fieldDescriptor(2)->m_pszName), "intValue") << "invalid int name";
	ASSERT_EQ(t.metaObject()->fieldDescriptor(2)->m_eType, eFieldInt) << "invalid int type";
	ASSERT_EQ(std::string(t.metaObject()->fieldDescriptor(3)->m_pszName), "uintValue") << "invalid uint name";
	ASSERT_EQ(t.metaObject()->fieldDescriptor(3)->m_eType, eFieldUint) << "invalid uint type";
	ASSERT_EQ(std::string(t.metaObject()->fieldDescriptor(4)->m_pszName), "floatValue") << "invalid float name";
	ASSERT_EQ(t.metaObject()->fieldDescriptor(4)->m_eType, eFieldFloat) << "invalid float type";
	ASSERT_EQ(std::string(t.metaObject()->fieldDescriptor(5)->m_pszName), "strValue") << "invalid string name";
	ASSERT_EQ(t.metaObject()->fieldDescriptor(5)->m_eType, eFieldString) << "invalid string type";
	ASSERT_EQ(std::string(t.metaObject()->fieldDescriptor(6)->m_pszName), "substruct") << "invalid object name";
	ASSERT_EQ(t.metaObject()->fieldDescriptor(6)->m_eType, eFieldObject) << "invalid object type";

}

void ObjectTest::testInitVisitor()
{
	TestStruct t;
	t.init();
	ASSERT_EQ(t.enumValue, eEnumValueUnk) << "t.enumValue incorrectly initilized";
	ASSERT_EQ(t.boolValue, true) << "t.boolValue incorrectly initilized";
	ASSERT_EQ(t.intValue, -1) << "t.intValue incorrectly initilized";
	ASSERT_EQ(t.uintValue, 123154) << "t.uintValue incorrectly initilized";
	ASSERT_EQ(t.floatValue, 25.0f) << "t.intValue incorrectly initilized";
	ASSERT_EQ(t.strValue, "testValue") << "t.strValue incorrectly initilized";
	ASSERT_EQ(t.substruct.name, "TestSubStruct");
	ASSERT_TRUE(t.arrValue.empty());
}

void ObjectTest::testSerialization()
{
	TestStruct t, t2;
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
    t2.fromString(t.toString());
	ASSERT_EQ(t.enumValue, t2.enumValue);
	ASSERT_EQ(t.boolValue, t2.boolValue);
	ASSERT_EQ(t.intValue, t2.intValue);
	ASSERT_EQ(t.uintValue, t2.uintValue);
	ASSERT_EQ(t.floatValue, t2.floatValue);
	ASSERT_EQ(t.strValue, t2.strValue);
	ASSERT_EQ(t.substruct.name, t2.substruct.name);
	ASSERT_EQ(t.arrValue.size(), t2.arrValue.size());
	for (size_t i = 0; i < t.arrValue.size(); ++i)
        ASSERT_EQ(t.arrValue[i], t2.arrValue[i]);
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
