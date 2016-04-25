#ifndef VARIANTTEST_H
#define VARIANTTEST_H
#include <gtest/gtest.h>
#include "Variant.h"

class VariantTest : public ::testing::Test
{
};

template<typename T>
class TypedVariantTest : public ::testing::Test
{
};

template<typename T1, typename T2>
class ConvertVariantTest : public ::testing::TestWithParam<std::pair<T1, T2> >
{
};

template<typename T>
class InvalidConvertVariantTest : public ::testing::TestWithParam<T>
{
};

#endif // VARIANTTEST_H
