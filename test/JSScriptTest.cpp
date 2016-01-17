#include "JSScriptTest.h"

#ifdef HAVE_SPIDERMONKEY
#include "JSScriptEngine.h"
#include "ScriptThreadBase.h"
#include <thread>

void JSScriptTest::SetUp()
{
    m_engine.reset(new metacpp::scripting::js::JSScriptEngine());
}

void JSScriptTest::TearDown()
{
    m_engine.reset();
}

void JSScriptTest::SetUpTestCase()
{

}

void JSScriptTest::TearDownTestCase()
{

}

TEST_F(JSScriptTest, testCompileSuccess)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("function f() { return 1; }");
    program->compile(ss, "filename");
}

TEST_F(JSScriptTest, testCompileFailure)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("invalid javascript program");
    EXPECT_THROW(program->compile(ss, "filename"), std::exception);
}

TEST_F(JSScriptTest, testCreateThread)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("function f() { return 1; }");
    program->compile(ss, "filename");
    auto thread = program->createThread();
    thread->run();
}

TEST_F(JSScriptTest, testThrow)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("throw 1;");
    program->compile(ss, "filename");
    auto thread = program->createThread();
    EXPECT_THROW(thread->run(), metacpp::scripting::ScriptRuntimeError);
}

TEST_F(JSScriptTest, testTerminate)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("while (1) { }");
    program->compile(ss, "filename");
    auto thread = program->createThread();
    std::exception_ptr ex;
    std::thread th([&]{ try { thread->run();  } catch (...) { ex = std::current_exception(); } });
    while (!thread->running())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    thread->abort();
    if (th.joinable())
        th.join();
    ASSERT_TRUE((bool)ex);
    EXPECT_THROW(std::rethrow_exception(ex), metacpp::scripting::TerminationException);
}

TEST_F(JSScriptTest, testMultipleThreads)
{
    // test multiple scripts in same script engine running simultenioulsy
    auto program = m_engine->createProgram();
    std::istringstream ss("while (1) { }");
    program->compile(ss, "filename");
    std::vector<std::thread> threads;
    std::vector<std::shared_ptr<metacpp::scripting::ScriptThreadBase>> scriptThreads;
    const size_t numThreads = 10;

    for (size_t i = 0; i < numThreads; ++i)
    {
        auto thread = program->createThread();
        scriptThreads.push_back(thread);
        threads.emplace_back([&]{ try { thread->run();  } catch (...) {  } });
        while (!thread->running())
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    for (size_t i = 0; i < numThreads; ++i)
    {
        scriptThreads[i]->abort();
        if (threads[i].joinable())
            threads[i].join();
    }
}

TEST_F(JSScriptTest, testFunctionCall)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("function f(a, b) { return (a * b).toString(); }");
    program->compile(ss, "filename");
    auto thread = program->createThread("f", 2, 3.5);
    metacpp::Variant value = thread->run();
    ASSERT_TRUE(value.isString());
    EXPECT_EQ(metacpp::variant_cast<metacpp::String>(value), "7");
}

TEST_F(JSScriptTest, testArrayResult)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("[ 1, 2.5, 'test' ]");
    program->compile(ss, "filename");
    metacpp::Variant value = program->createThread()->run();
    ASSERT_TRUE(value.isArray());
    auto array = metacpp::variant_cast<metacpp::VariantArray>(value);
    ASSERT_EQ(array.size(), 3);
    EXPECT_EQ(metacpp::variant_cast<int>(array[0]), 1);
    EXPECT_EQ(metacpp::variant_cast<double>(array[1]), 2.5);
    EXPECT_EQ(metacpp::variant_cast<metacpp::String>(array[2]), "test");
}

TEST_F(JSScriptTest, testArrayArgument)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("function len(a) { return a.length }");
    program->compile(ss, "filename");
    auto value = program->createThread("len", metacpp::Variant({ 12, "test" }))->run();
    ASSERT_TRUE(value.isIntegral());
    EXPECT_EQ(metacpp::variant_cast<int>(value), 2);
}

TEST_F(JSScriptTest, testDateResult)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("function f() { return new Date(2001, 01, 01, 12, 59, 23); }");
    program->compile(ss, "filename");
    auto thread = program->createThread("f");
    metacpp::Variant result = thread->run();
    ASSERT_TRUE(result.isDateTime());
    auto dt = metacpp::variant_cast<metacpp::DateTime>(result);
    EXPECT_EQ(dt.year(), 2001);
    EXPECT_EQ(dt.month(), metacpp::February);
    EXPECT_EQ(dt.day(), 1);
    EXPECT_EQ(dt.hours(), 12);
    EXPECT_EQ(dt.minutes(), 59);
    EXPECT_EQ(dt.seconds(), 23);
}

TEST_F(JSScriptTest, testDateArgument)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("function toISO(dt) { return dt.toISOString() }");
    program->compile(ss, "filename");
    auto value = program->createThread("toISO", metacpp::DateTime(2001, metacpp::February, 1, 12, 59, 23))->run();
    ASSERT_TRUE(value.isString());
    EXPECT_EQ(metacpp::variant_cast<metacpp::String>(value), "2001-02-01T09:59:23.000Z");
}

#endif
