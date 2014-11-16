#pragma once
#include <gtest/gtest.h>

class ObjectTest : public testing::Test
{
public:
	void testMetaInfo();
    void testInitVisitor();
	void testSerialization();
	void tesetDesierialization();
};
