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
    //auto sd = t.metaObject()->descriptor();
    //ASSERT_EQ(sd->m_superDescriptor, &STRUCT_INFO(TestBaseStruct));
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
    t.datetimeValue = DateTime::fromISOString("0100-10-12 00:00:00");
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
    std::cout << t.toString() << std::endl;
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
