#include "StringTest.h"

using namespace orm;

void StringTest::testNull()
{
    String str1;
    EXPECT_TRUE(str1.isNull());
    EXPECT_TRUE(str1.isNullOrEmpty());
    String str2 = "not null";
    EXPECT_FALSE(str2.isNull());
    EXPECT_FALSE(str2.isNullOrEmpty());
    EXPECT_TRUE(String::null().isNull());
    EXPECT_FALSE(String::empty().isNull());
}

TEST_F(StringTest, TestNull)
{
	testNull();
}

void StringTest::testRefCount()
{
    String str("test");
	ASSERT_EQ(str.refCount(), 1);
    String str2 = str;
	ASSERT_EQ(str.refCount(), 2);
	ASSERT_EQ(str2.refCount(), 2);
	ASSERT_EQ(str, str2);
}

TEST_F(StringTest, TestRefCount)
{
	testRefCount();
}

void StringTest::testDetach()
{
    String str1 = "1231923j", str2 = str1;
	str1[2] = '1';
	ASSERT_EQ(str1, "1211923j");
	ASSERT_EQ(str2, "1231923j");
}

TEST_F(StringTest, TestDetach)
{
	testDetach();
}

// test random access iterators
void StringTest::testStl()
{
    String str = "12378923";
	std::sort(str.begin(), str.end());
	ASSERT_EQ(str, "12233789");
}

TEST_F(StringTest, TestStl)
{
	testStl();
}

void StringTest::testAppend(const char *a, const char *b, const char *result)
{
    String str1 = a;
	str1.append(b);
	ASSERT_EQ(str1, result);
}

TEST_F(StringTest, TestAppend)
{
	testAppend("123123", "abc", "123123abc");
	testAppend(nullptr, "123", "123");
	testAppend("123", nullptr, "123");
	testAppend("1asd3", "", "1asd3");
}

void StringTest::testArrayOfStrings()
{
    Array<String> arr;
	arr.push_back("Hello, ");
	arr.push_back("world!");
    String join;
	join += arr[0];
	join += arr[1];
	ASSERT_EQ(join, "Hello, world!");
}

TEST_F(StringTest, TestArrayOfStrings)
{
	testArrayOfStrings();
}

void StringTest::testFindSubstr()
{
    String test = "123abc123";
    EXPECT_EQ(test.firstIndexOf("123"), 0);
    EXPECT_EQ(test.firstIndexOf("1234"), String::npos);
    EXPECT_EQ(test.firstIndexOf("abc"), 3);
    EXPECT_EQ(test.lastIndexOf("123"), 6);
    EXPECT_EQ(test.lastIndexOf("abc"), 3);
    EXPECT_EQ(test.lastIndexOf("1234"), String::npos);
}

TEST_F(StringTest, TestSubstr)
{
	testFindSubstr();
}

void StringTest::testSubStr2()
{
    String test = "123abc456";
    EXPECT_TRUE(test.startsWith("123"));
    EXPECT_FALSE(test.startsWith("234"));
    EXPECT_FALSE(test.startsWith("1234"));
    EXPECT_TRUE(test.endsWith("456"));
    EXPECT_FALSE(test.endsWith("45"));
    EXPECT_FALSE(test.endsWith("4567"));
}

TEST_F(StringTest, TestSubstr2)
{
	testSubStr2();
}

void StringTest::testStringBuilder()
{
    String a = ", "; String b = "world";
	const char *hello = "Hello";
    String sb = hello + a + b + "!";
	ASSERT_EQ(sb, "Hello, world!");
}

TEST_F(StringTest, TestStringBuilder)
{
	testStringBuilder();
}

void StringTest::testAssign(const char *str)
{
    String s1;
    s1 = str;
    ASSERT_EQ(s1, str);
}

TEST_F(StringTest, TestAssign)
{
    testAssign("");
    testAssign("asdlaasd");
    testAssign(nullptr);
}

TEST_F(StringTest, TestAWConversion)
{
    ASSERT_EQ(string_cast<WString>("test"), u"test");
    ASSERT_EQ(string_cast<WString>("Hello, world!"), u"Hello, world!");
}

TEST_F(StringTest, TestWAConversion)
{
    ASSERT_EQ(string_cast<String>(u"test"), "test");
    ASSERT_EQ(string_cast<String>(u"Hello, world!"), "Hello, world!");
}
