#ifndef OBJECTTEST_H
#define OBJECTTEST_H
#include <gtest/gtest.h>

class ObjectTest : public testing::Test
{
public:
	void testMetaInfo();
    void testInitVisitor();
	void testSerialization();
    void testBsonSerialization();
	void tesetDesierialization();
};
#endif // OBJECTTEST_H
