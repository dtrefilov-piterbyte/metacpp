#pragma once
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
};
