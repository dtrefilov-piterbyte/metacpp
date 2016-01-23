#ifndef JSSCRIPTTEST_H
#define JSSCRIPTTEST_H

#include <gtest/gtest.h>
#include "ScriptEngineBase.h"

class JSScriptTest : public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

    static void SetUpTestCase();
    static void TearDownTestCase();

    std::unique_ptr<metacpp::scripting::ScriptEngineBase> m_engine;
    metacpp::SharedObjectPointer<metacpp::scripting::ScriptProgramBase> m_program;
};

#endif // JSSCRIPTTEST_H
