#ifndef JSSCRIPTTEST_H
#define JSSCRIPTTEST_H

#include <gtest/gtest.h>
#include "JSScriptEngine.h"

class JSScriptTest : public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

    static void SetUpTestCase();
    static void TearDownTestCase();

    std::unique_ptr<metacpp::scripting::ScriptEngineBase> m_engine;
};

#endif // JSSCRIPTTEST_H
