#include "StringTest.h"
#include "Uri.h"
#include "Variant.h"
#include <sstream>

using namespace metacpp;

void StringTest::testNull()
{
    String str1;
    EXPECT_TRUE(str1.isNull());
    EXPECT_TRUE(str1.isNullOrEmpty());
    String str2 = "not null";
    EXPECT_FALSE(str2.isNull());
    EXPECT_FALSE(str2.isNullOrEmpty());
    EXPECT_TRUE(String::getNull().isNull());
    EXPECT_FALSE(String::getEmpty().isNull());
    EXPECT_TRUE(String::getEmpty().isNullOrEmpty());
}

TEST_F(StringTest, TestNull)
{
	testNull();
}

//void StringTest::testRefCount()
//{
//    String str("test");
//	ASSERT_EQ(str.refCount(), 1);
//    String str2 = str;
//	ASSERT_EQ(str.refCount(), 2);
//	ASSERT_EQ(str2.refCount(), 2);
//	ASSERT_EQ(str, str2);
//}

//TEST_F(StringTest, TestRefCount)
//{
//	testRefCount();
//}

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

template<typename TStreamChar, typename TChar>
static void TestStreamOperators(const TChar *str)
{
    std::basic_ostringstream<TStreamChar> oss;
    metacpp::operator <<(oss, StringBase<TChar>(str));
    std::basic_istringstream<TStreamChar> iss(oss.str());
    StringBase<TChar> s;
    iss >> s;
    EXPECT_EQ(s, StringBase<TChar>(str));
}

TEST_F(StringTest, testStreamOperators)
{
    EXPECT_NO_THROW(TestStreamOperators<char>(""));
    EXPECT_NO_THROW(TestStreamOperators<char>("asdlaasd"));

    //EXPECT_NO_THROW(TestStreamOperators<char16_t>(""));
    //EXPECT_NO_THROW(TestStreamOperators<char16_t>("asdlaasd"));

    EXPECT_NO_THROW(TestStreamOperators<char>(U16("")));
    EXPECT_NO_THROW(TestStreamOperators<char>(U16("asdlaasd")));

    //EXPECT_NO_THROW(TestStreamOperators<char16_t>(U16("")));
    //EXPECT_NO_THROW(TestStreamOperators<char16_t>(U16("asdlaasd")));
}

void StringTest::testStreams()
{
    {
        metacpp::StringStream ss1("TestString", std::ios_base::out | std::ios_base::ate);
        ss1 << 123;
        ASSERT_EQ(ss1.str(), "TestString123");
    }
    {
        metacpp::StringStream ss2("TestString");
        ss2 << 123;
        ASSERT_EQ(ss2.str(), "123tString");
    }
    {
        metacpp::StringStream ss3("123 456", std::ios_base::in | std::ios_base::out | std::ios_base::ate);
        ss3 << " " << 789;
        int n1, n2, n3;
        ss3 >> n1 >> n2 >> n3;
        EXPECT_EQ(n1, 123);
        EXPECT_EQ(n2, 456);
        EXPECT_EQ(n3, 789);
    }
    {
        metacpp::StringBuf buf;
        std::ostream os(&buf);
        os << "Test: " << std::hex << 255;
        EXPECT_EQ(buf.str(), "Test: ff");
    }
}

TEST_F(StringTest, testStreams)
{
    testStreams();
}

int StringTest::testToValue(const String& s)
{
    return s.toValue<int>();
}

TEST_F(StringTest, testToValue)
{
    EXPECT_EQ(12, testToValue("12"));
    EXPECT_EQ(-12, testToValue("-12"));
    EXPECT_EQ(12, testToValue(" 12"));
    EXPECT_EQ(12, testToValue(" 12asd"));
    EXPECT_ANY_THROW(testToValue("abc"));
    EXPECT_ANY_THROW(testToValue(String()));
}

void StringTest::testJoin()
{
    StringArray strArr = { "lorem", "ipsum", "dolor", "sit", "amet" };
    ASSERT_EQ(join(strArr, " "), "lorem ipsum dolor sit amet");
}

TEST_F(StringTest, testJoin)
{
    testJoin();
}
void StringTest::testReplace(const String &inStr, const String &from, const String &to, const String &outStr)
{
    String str(inStr);
    str.replace(from, to);
    EXPECT_EQ(str, outStr);
}

TEST_F(StringTest, testReplace)
{
    testReplace("12345", "12", "ab", "ab345");
    testReplace("aabcdae", "a", "aa", "aaaabcdaae");
    testReplace("C%2B%2B", "%2B", "+", "C++");
}

void StringTest::testUri(const String& strUri, const String &schema, const String &hierarchy,
                         const std::initializer_list<std::pair<String, String> > &params,
                         const String &host, const String &port, const String &path, const String &username, const String &password)
{
    metacpp::Uri uri(strUri);
    EXPECT_EQ(uri.schemeName(), schema);
    EXPECT_EQ(uri.hierarchy(), hierarchy);
    for (auto param : params)
        EXPECT_EQ(uri.param(param.first), param.second);
    EXPECT_EQ(uri.host(), host);
    EXPECT_EQ(uri.port(), port);
    EXPECT_EQ(uri.path(), path);
    EXPECT_EQ(uri.username(), username);
    EXPECT_EQ(uri.password(), password);
}

TEST_F(StringTest, TestUri)
{
    testUri("http://example.com/foo/bar?param1=value1&param2=value2", "http", "example.com/foo/bar",
        { std::make_pair<String, String>("param1", "value1"), std::make_pair<String, String>("param2", "value2") },
            "example.com", String(), "foo/bar");
    testUri("postgres://user:pass@localhost:5432?connect_timeout=5", "postgres", "user:pass@localhost:5432",
        { std::make_pair<String, String>("connect_timeout", "5") }, "localhost", "5432", String(), "user", "pass");
    testUri("sqlite3://test.db?mode=memory&cache=shared", "sqlite3", "test.db",
        { std::make_pair<String, String>("cache", "shared"), std::make_pair<String, String>("mode", "memory") },
            "test.db");
    testUri("user@mailhost", String(), "user@mailhost", { }, "mailhost", String(), String(), "user");
    testUri("http://example.com?param=v+s%21", "http", "example.com",
        { std::make_pair<String, String>("param", "v s!") },
            "example.com");
}

TEST_F(StringTest, TestAWConversion)
{
    ASSERT_EQ(string_cast<WString>("test"), U16("test"));
    ASSERT_EQ(string_cast<WString>("Hello, world!"), U16("Hello, world!"));
    ASSERT_EQ(string_cast<WString>("кирилица"), U16("кирилица"));
}

TEST_F(StringTest, TestWAConversion)
{
    ASSERT_EQ(string_cast<String>(U16("test")), "test");
    ASSERT_EQ(string_cast<String>(U16("Hello, world!")), "Hello, world!");
	ASSERT_EQ(string_cast<String>(U16("кирилица")), "кирилица");
}

TEST_F(StringTest, TestAAConversion)
{
    ASSERT_EQ(string_cast<String>(String("test")), "test");
    ASSERT_EQ(string_cast<String>(String("кирилица")), "кирилица");
}

TEST_F(StringTest, TestWWConversion)
{
    ASSERT_EQ(string_cast<WString>(WString(U16("test"))), U16("test"));
    ASSERT_EQ(string_cast<WString>(WString(U16("кирилица"))), U16("кирилица"));
}

TEST_F(StringTest, TestStdAConversion)
{
    ASSERT_EQ(string_cast<String>(std::string("test")), "test");
    ASSERT_EQ(string_cast<String>(std::basic_string<char16_t>(U16("test"))), "test");
    ASSERT_EQ(string_cast<String>(std::string("кирилица")), "кирилица");
    ASSERT_EQ(string_cast<String>(std::basic_string<char16_t>(U16("кирилица"))), "кирилица");
}

TEST_F(StringTest, TestStdWConversion)
{
    ASSERT_EQ(string_cast<WString>(std::string("test")), U16("test"));
    ASSERT_EQ(string_cast<WString>(std::basic_string<char16_t>(U16("test"))), U16("test"));
    ASSERT_EQ(string_cast<WString>(std::string("кирилица")), U16("кирилица"));
    ASSERT_EQ(string_cast<WString>(std::basic_string<char16_t>(U16("кирилица"))), U16("кирилица"));
}

TEST_F(StringTest, TestUrlencode)
{
    EXPECT_EQ(String("You shall pass!").urlencode(),
              "You+shall+pass%21");
    EXPECT_EQ(String("Unescaped characters: [~-_.]").urlencode(),
              "Unescaped+characters%3A+%5B~-_.%5D");
    EXPECT_EQ(WString(U16("You shall pass!")).urlencode(),
              U16("You+shall+pass%21"));
}

TEST_F(StringTest, TestUrldecode)
{
    EXPECT_EQ(String("You+shall+pass%21").urldecode(),
              "You shall pass!");
    EXPECT_EQ(String("Unescaped+characters%3A+%5B~-_.%5D").urldecode(),
              "Unescaped characters: [~-_.]");
    EXPECT_EQ(String("Unescaped+characters%3a+%5b~-_.%5d").urldecode(),
              "Unescaped characters: [~-_.]");
    EXPECT_EQ(WString(U16("You+shall+pass%21")).urldecode(),
              U16("You shall pass!"));
    EXPECT_THROW(String("You+shall+not+pass%!1!").urldecode(), std::invalid_argument);
}

TEST_F(StringTest, TestStrncasecmp)
{
    auto check = [](const char *a, const char *b, size_t len, bool result) {
        EXPECT_EQ(metacpp::detail::StringHelper<char>::strncasecmp(a, b, len) == 0, result);
    };
    check("abc123", "abc", 3, true);
    check("abc123", "abc", 4, false);
    check("abc123", "abc", 2, true);
    check("abc123", "ABC", 3, true);
    check("abc123", "ABC", 4, false);
    check("abc123", "ABC", 2, true);
}

TEST_F(StringTest, TestStrcpy)
{
    char buffer[256];
    auto check = [&](const char *a) {
        metacpp::detail::StringHelper<char>::strcpy(buffer, a);
        EXPECT_EQ(buffer, String(a));
    };
    check("test");
    check("кирилица");
}

TEST_F(StringTest, TestStrncpy)
{
    char buffer[256];
    auto check = [&](const char *a, size_t len, const char *check) {
        metacpp::detail::StringHelper<char>::strncpy(buffer, a, len);
        EXPECT_EQ(String(buffer), String(check));
    };
    check("test", 4, "test");
    check("test123", 4, "test");
    check("test", 5, "test");
}

TEST_F(StringTest, TestStrstr)
{
    auto check = [](const char *haystack, const char *needle, const char *check) {
        EXPECT_EQ(metacpp::detail::StringHelper<char>::strstr(haystack, needle), check);
    };
    char test_str[] = "very long string";
    check(test_str, test_str, test_str);
    check(test_str, "very", test_str);
    check(test_str, "long", test_str + 5);
    check(test_str, "different", nullptr);
}

TEST_F(StringTest, TestWStrcpy)
{
    char16_t buffer[256];
    auto check = [&](const char16_t *a) {
        metacpp::detail::StringHelper<char16_t>::strcpy(buffer, a);
        EXPECT_EQ(WString(buffer), WString(a));
    };
    check(U16("test"));
    check(U16("кирилица"));
}

TEST_F(StringTest, TestWStrncpy)
{
    char16_t buffer[256];
    auto check = [&](const char16_t *a, size_t len, const char16_t *check) {
        metacpp::detail::StringHelper<char16_t>::strncpy(buffer, a, len);
        EXPECT_EQ(buffer, WString(check));
    };
    check(U16("test"), 4, U16("test"));
    check(U16("test123"), 4, U16("test"));
    check(U16("test"), 5, U16("test"));
}

TEST_F(StringTest, TestWStrstr)
{
    auto check = [](const char16_t *haystack, const char16_t *needle, const char16_t *check) {
        EXPECT_EQ(metacpp::detail::StringHelper<char16_t>::strstr(haystack, needle), check);
    };
    const char16_t *test_str = U16("very long string");
    check(test_str, test_str, test_str);
    check(test_str, U16("very"), test_str);
    check(test_str, U16("long"), test_str + 5);
    check(test_str, U16("different"), nullptr);
}
