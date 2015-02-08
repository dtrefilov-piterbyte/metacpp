#ifndef STRINGTEST_H
#define STRINGTEST_H
#include "String.h"
#include <gtest/gtest.h>

class StringTest : public testing::Test
{
public:
	void testNull();
	void testRefCount();
	void testDetach();
	void testStl();
	void testAppend(const char *a, const char *b, const char *result);
	void testArrayOfStrings();
	void testFindSubstr();
	void testSubStr2();
	void testStringBuilder();
    void testAssign(const char *v);
    void testStreams(const char *str);
    void testJoin();
    void testReplace(const metacpp::String& inStr, const metacpp::String& from, const metacpp::String& to, const metacpp::String& outStr);
    void testUri(const metacpp::String &uri, const metacpp::String& schema, const metacpp::String& hierarchy,
                 const std::initializer_list<std::pair<metacpp::String, metacpp::String> >& params = std::initializer_list<std::pair<metacpp::String, metacpp::String> >(),
                 const metacpp::String& host = metacpp::String(), const metacpp::String& port = metacpp::String(), const metacpp::String& path = metacpp::String(),
                 const metacpp::String& username = metacpp::String(), const metacpp::String& password = metacpp::String());
};
#endif // STRINGTEST_H
