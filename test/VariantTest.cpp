#include "VariantTest.h"
#include "Variant.h"
#include "MetaInfo.h"
#include "Object.h"
#include <gtest/gtest-param-test.h>

using namespace metacpp;

namespace {
struct MyObject : public Object
{
    MyObject() { }
    META_INFO_DECLARE(MyObject)
};

METHOD_INFO_BEGIN(MyObject)
    CONSTRUCTOR(MyObject)
METHOD_INFO_END(MyObject)

REFLECTIBLE_DERIVED_M(MyObject, Object)

META_INFO(MyObject)

}

TEST_F(VariantTest, AssignTest)
{
    Variant v(12);
    Variant v2 = v;
    EXPECT_EQ(variant_cast<int>(v), 12);
    EXPECT_EQ(variant_cast<int>(v2), 12);
    v2 = 13;
    EXPECT_EQ(variant_cast<int>(v), 12);
    EXPECT_EQ(variant_cast<int>(v2), 13);
}

TEST_F(VariantTest, ExtractObjectTest)
{
    Variant v(MyObject::staticMetaObject()->createInstance());
    auto obj = dynamic_cast<MyObject *>(v.extractObject());
    obj->deleteThis();
}

TEST_F(VariantTest, StreamTest)
{
    std::ostringstream ss;
    ss << Variant("test") << Variant(12);
    EXPECT_EQ(ss.str(), "test12");
}

typedef ::testing::Types<bool, int32_t, uint32_t, int64_t, uint64_t, float, double, String, DateTime> VariantTypes;
TYPED_TEST_CASE_P(TypedVariantTest);

TYPED_TEST_P(TypedVariantTest, ConstructValidTest)
{
    TypeParam p = TypeParam();
    Variant v(p);
    ASSERT_TRUE(v.valid());
}

TYPED_TEST_P(TypedVariantTest, ConstructInvalidTest)
{
    Variant v;
    ASSERT_FALSE(v.valid());
}

TYPED_TEST_P(TypedVariantTest, TypeTest)
{
    TypeParam p = TypeParam();
    Variant v(p);
    ASSERT_EQ(v.type(), ::detail::FullFieldInfoHelper<TypeParam>::type());
}

TYPED_TEST_P(TypedVariantTest, AssignTest)
{
    TypeParam p = TypeParam();
    Variant v(p), v1 = v;
    EXPECT_EQ(v.type(), v1.type());
    EXPECT_EQ(v.value<TypeParam>(), v1.value<TypeParam>());
}

TYPED_TEST_P(TypedVariantTest, IsArithmetic)
{
    TypeParam p = TypeParam();
    Variant v(p);
    ASSERT_EQ(v.isArithmetic(), std::is_arithmetic<TypeParam>::value);
}

TYPED_TEST_P(TypedVariantTest, IsIntegral)
{
    TypeParam p = TypeParam();
    Variant v(p);
    ASSERT_EQ(v.isIntegral(), std::is_integral<TypeParam>::value);
}

TYPED_TEST_P(TypedVariantTest, IsFloatingPoint)
{
    TypeParam p = TypeParam();
    Variant v(p);
    ASSERT_EQ(v.isFloatingPoint(), std::is_floating_point<TypeParam>::value);
}

TYPED_TEST_P(TypedVariantTest, IsString)
{
    TypeParam p = TypeParam();
    Variant v(p);
    ASSERT_EQ(v.isString(), (std::is_same<String, TypeParam>::value));
}

TYPED_TEST_P(TypedVariantTest, IsDateTime)
{
    TypeParam p = TypeParam();
    Variant v(p);
    ASSERT_EQ(v.isDateTime(), (std::is_same<DateTime, TypeParam>::value));
}

REGISTER_TYPED_TEST_CASE_P(TypedVariantTest, ConstructValidTest, ConstructInvalidTest, TypeTest, AssignTest, IsArithmetic, IsIntegral, IsFloatingPoint, IsString, IsDateTime);

INSTANTIATE_TYPED_TEST_CASE_P(TypedVariantTestInstance, TypedVariantTest, VariantTypes);


#define INSTANTIATE_CONVERT_TEST(T1, T2, Params) \
typedef ConvertVariantTest<T1, T2> T1##_##T2##_ConvertVariantTest; \
 \
TEST_P(T1##_##T2##_ConvertVariantTest, ConvertTest) \
{ \
    Variant v(GetParam().first); \
    ASSERT_EQ(v.value<decltype(GetParam().second)>(), GetParam().second); \
} \
 \
INSTANTIATE_TEST_CASE_P(T1##_##T2##_ConvertVariantTestInstance, T1##_##T2##_ConvertVariantTest, \
                        Params);

#define INSTANTIATE_INVALID_CONVERT_TEST(T1, T2, Params) \
typedef InvalidConvertVariantTest<T1> T1##_##T2##_InvalidConvertVariantTest; \
 \
TEST_P(T1##_##T2##_InvalidConvertVariantTest, ConvertTest) \
{ \
    EXPECT_THROW(variant_cast<T2>(Variant(GetParam())), std::invalid_argument); \
} \
 \
INSTANTIATE_TEST_CASE_P(T1##_##T2##_InvalidConvertVariantTestInstance, T1##_##T2##_InvalidConvertVariantTest, \
                        Params);


INSTANTIATE_CONVERT_TEST(int32_t, bool, ::testing::Values(
                             std::make_pair<int32_t, bool>(0, false),
                             std::make_pair<int32_t, bool>(1, true),
                             std::make_pair<int32_t, bool>(2, true),
                             std::make_pair<int32_t, bool>(-1, true)
                             ))

INSTANTIATE_CONVERT_TEST(uint32_t, bool, ::testing::Values(
                             std::make_pair<uint32_t, bool>(0, false),
                             std::make_pair<uint32_t, bool>(1, true),
                             std::make_pair<uint32_t, bool>(2, true),
                             std::make_pair<uint32_t, bool>(0xFFFFFFFF, true)
                             ))

INSTANTIATE_CONVERT_TEST(float, bool, ::testing::Values(
                             std::make_pair<float, bool>(0.0, false),
                             std::make_pair<float, bool>(1.5, true),
                             std::make_pair<float, bool>(2.0, true),
                             std::make_pair<float, bool>(std::numeric_limits<float>::infinity(), true),
                             std::make_pair<float, bool>(std::numeric_limits<float>::signaling_NaN(), true)
                             ))

INSTANTIATE_CONVERT_TEST(double, bool, ::testing::Values(
                             std::make_pair<double, bool>(0.0, false),
                             std::make_pair<double, bool>(1.5, true),
                             std::make_pair<double, bool>(2.0, true),
                             std::make_pair<double, bool>(std::numeric_limits<float>::infinity(), true),
                             std::make_pair<double, bool>(std::numeric_limits<float>::signaling_NaN(), true)
                             ))

INSTANTIATE_CONVERT_TEST(int32_t, uint32_t, ::testing::Values(
                             std::make_pair<int32_t, uint32_t>(0, 0),
                             std::make_pair<int32_t, uint32_t>(1, 1),
                             std::make_pair<int32_t, uint32_t>(-1, 0xFFFFFFFF)
                             ))

INSTANTIATE_CONVERT_TEST(float, int64_t, ::testing::Values(
                             std::make_pair<float, int64_t>(0.0f, 0),
                             std::make_pair<float, int64_t>(0.9f, 0),
                             std::make_pair<float, int64_t>(20.1f, 20),
                             std::make_pair<float, int64_t>(-1.0f, -1)
                             ))


INSTANTIATE_CONVERT_TEST(int32_t, String, ::testing::Values(
                             std::make_pair<int32_t, String>(0, "0"),
                             std::make_pair<int32_t, String>(1, "1"),
                             std::make_pair<int32_t, String>(-5, "-5"),
                             std::make_pair<int32_t, String>(16, "16")
                             ))

INSTANTIATE_CONVERT_TEST(uint32_t, String, ::testing::Values(
                             std::make_pair<uint32_t, String>(0, "0"),
                             std::make_pair<uint32_t, String>(1, "1"),
                             std::make_pair<uint32_t, String>(0xFFFFFFFF, "4294967295"),
                             std::make_pair<uint32_t, String>(16, "16")
                             ))


INSTANTIATE_CONVERT_TEST(DateTime, String, ::testing::Values(
                             std::make_pair<DateTime, String>(DateTime::fromString("0100-01-01 00:00:00"), "0100-01-01 00:00:00"),
                             std::make_pair<DateTime, String>(DateTime::fromString("1970-01-01 00:00:00"), "1970-01-01 00:00:00"),
                             std::make_pair<DateTime, String>(DateTime::fromString("2012-01-01 00:23:32"), "2012-01-01 00:23:32")
                             ))

INSTANTIATE_INVALID_CONVERT_TEST(String, DateTime, ::testing::Values(
                                     String(),
                                     "asdad"))

INSTANTIATE_CONVERT_TEST(String, DateTime, ::testing::Values(
                             std::make_pair<String, DateTime>("0100-01-01 00:00:00", DateTime::fromString("0100-01-01 00:00:00")),
                             std::make_pair<String, DateTime>("1970-01-01 00:00:00", DateTime::fromString("1970-01-01 00:00:00")),
                             std::make_pair<String, DateTime>("2012-01-01 00:23:32", DateTime::fromString("2012-01-01 00:23:32"))
                             ))
