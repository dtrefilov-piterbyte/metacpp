#include "JSScriptTest.h"

#ifdef HAVE_SPIDERMONKEY
#include "JSScriptEngine.h"
#include "ScriptThreadBase.h"
#include "Object.h"
#include <thread>
#include <stdexcept>


struct MyObject : public metacpp::Object
{
    int xValue;
    metacpp::String stringValue;
    metacpp::Array<double> arrayValue;
    Nullable<float> optValue;
    metacpp::DateTime dateValue;

    MyObject(int x = 0) : xValue(x)
    {
    }

    int x() const { return xValue; }
    void setX(int newX) { xValue = newX; }

    static metacpp::String foo() { return "foo"; }
    static metacpp::String foo(const metacpp::String& arg) { return "foo" + arg; }

    static metacpp::String className() { return MyObject::staticMetaObject()->name(); }

    META_INFO_DECLARE(MyObject)
};

STRUCT_INFO_BEGIN(MyObject)
    FIELD(MyObject, xValue)
    FIELD(MyObject, stringValue)
    FIELD(MyObject, arrayValue)
    FIELD(MyObject, optValue)
    FIELD(MyObject, dateValue)
STRUCT_INFO_END(MyObject)

METHOD_INFO_BEGIN(MyObject)
    METHOD(MyObject, x)
    METHOD(MyObject, setX)
    METHOD(MyObject, className)
    SIGNATURE_METHOD(MyObject, foo, metacpp::String (*)())
    SIGNATURE_METHOD(MyObject, foo, metacpp::String (*)(const metacpp::String&))
METHOD_INFO_END(MyObject)

REFLECTIBLE_FM(MyObject)

META_INFO(MyObject)

void JSScriptTest::SetUp()
{
    m_engine.reset(new metacpp::scripting::js::JSScriptEngine());
    m_engine->registerClass(MyObject::staticMetaObject());
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

TEST_F(JSScriptTest, testSimpleRun)
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

#ifdef MT_SPIDERMONKEY
TEST_F(JSScriptTest, testTerminate)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("while (1) { }");
    program->compile(ss, "filename");
    auto thread = program->createThread();
    std::exception_ptr ex;
    std::thread th([&]{ try { thread->run();  } catch (...) { ex = std::current_exception(); } });
    while (!thread->running())
        std::this_thread::yield();
    thread->abort(1000);
    if (th.joinable())
        th.join();
    ASSERT_TRUE((bool)ex);
    EXPECT_THROW(std::rethrow_exception(ex), metacpp::scripting::TerminationException);
}

TEST_F(JSScriptTest, testMultipleThreads)
{
    // test multiple threads in same script engine running simultenioulsy
    auto program = m_engine->createProgram();
    std::istringstream ss("while (1) { }");
    program->compile(ss, "filename");
    std::vector<std::thread> threads;
    std::vector<metacpp::SharedObjectPointer<metacpp::scripting::ScriptThreadBase>> scriptThreads;
    const size_t numThreads = 10;

    for (size_t i = 0; i < numThreads; ++i)
    {
        auto thread = program->createThread();
        scriptThreads.push_back(thread);
        threads.emplace_back([&]{ try { thread->run();  } catch (...) {  } });
        while (!thread->running())
            std::this_thread::yield();
    }

    for (size_t i = 0; i < numThreads; ++i)
    {
        scriptThreads[i]->abort(1000);
        if (threads[i].joinable())
            threads[i].join();
    }
}

TEST_F(JSScriptTest, testMultipleThreadsRunFailure)
{
    // test failure running same thread simultiniously
    auto program = m_engine->createProgram();
    std::istringstream ss("while (1) { }");
    program->compile(ss, "filename");
    auto thread = program->createThread();
    std::exception_ptr ex = nullptr;
    std::thread thMain([&]{ try { thread->run();  } catch (...) { } });
    while (!thread->running())
        std::this_thread::yield();
    // supplementary thread will fail to start
    std::thread thSupp([&]{ try { thread->run();  } catch (...) { ex = std::current_exception(); } });

    EXPECT_TRUE(thread->abort(1000));
    thMain.join();
    thSupp.join();

    ASSERT_TRUE((bool)ex);
    EXPECT_THROW(std::rethrow_exception(ex), std::runtime_error);
}
#endif

TEST_F(JSScriptTest, testSequentialRun)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("function foo() { return 1; }\nfunction bar() { return 2; }");
    program->compile(ss, "filename");
    auto thread1 = program->createThread("foo");
    auto thread2 = program->createThread("bar");
    EXPECT_EQ(metacpp::variant_cast<int>(thread1->run()), 1);
    EXPECT_EQ(metacpp::variant_cast<int>(thread2->run()), 2);
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
    std::istringstream ss("function toStdTime(dt) { return dt.getTime() }");
    program->compile(ss, "filename");
    auto value = program->createThread("toStdTime", metacpp::DateTime(981021563))->run();
    EXPECT_EQ(metacpp::variant_cast<time_t>(value), 981021563000);
}

TEST_F(JSScriptTest, testObjectResult)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("function f() { return new MyObject(); }");
    program->compile(ss, "filename");
    auto thread = program->createThread("f");
    metacpp::Variant result = thread->run();
    ASSERT_EQ(result.type(), eFieldObject);
    ASSERT_NO_THROW(metacpp::variant_cast<MyObject *>(result));
}

TEST_F(JSScriptTest, testObjectArgument)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("function f(obj) { }");
    program->compile(ss, "filename");
    metacpp::Object *obj = new MyObject();
    auto thread = program->createThread("f", obj);
    thread->run();
}

TEST_F(JSScriptTest, testObjectPassThrough)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("function f(obj) { return obj; }");
    program->compile(ss, "filename");
    MyObject *obj = new MyObject();
    auto thread = program->createThread("f", obj);
    metacpp::Variant result = thread->run();
    ASSERT_EQ(result.type(), eFieldObject);
    ASSERT_EQ(metacpp::variant_cast<MyObject *>(result), obj);
}

TEST_F(JSScriptTest, testObjectOwnMethodCall)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("function f(obj) { return obj.x() }");
    program->compile(ss, "filename");
    // script engine takes object ownership
    auto thread = program->createThread("f", new MyObject(12378));
    ASSERT_EQ(metacpp::variant_cast<int>(thread->run()), 12378);
}

TEST_F(JSScriptTest, testObjectStaticMethodCall)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("function f() { return MyObject.className() }");
    program->compile(ss, "filename");
    // script engine takes object ownership
    auto thread = program->createThread("f");
    ASSERT_EQ(metacpp::variant_cast<metacpp::String>(thread->run()), "MyObject");
}

TEST_F(JSScriptTest, testLinkProtoConstructor)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("function f() { var obj = new MyObject(); obj.setX(25); return obj; }");
    program->compile(ss, "filename");
    // script engine takes object ownership
    auto thread = program->createThread("f");
    metacpp::Variant result = thread->run();
    ASSERT_EQ(result.type(), eFieldObject);
    MyObject *obj;
    ASSERT_NO_THROW(obj = metacpp::variant_cast<MyObject *>(result));
    ASSERT_EQ(obj->x(), 25);
}

TEST_F(JSScriptTest, testOverloadedCallFoo)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("function f() { return MyObject.foo() }");
    program->compile(ss, "filename");
    // script engine takes object ownership
    auto thread = program->createThread("f");
    ASSERT_EQ(metacpp::variant_cast<metacpp::String>(thread->run()), "foo");
}

TEST_F(JSScriptTest, testOverloadedCallFooBar)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("function f(arg) { return MyObject.foo(arg) }");
    program->compile(ss, "filename");
    // script engine takes object ownership
    auto thread = program->createThread("f", "bar");
    ASSERT_EQ(metacpp::variant_cast<metacpp::String>(thread->run()), "foobar");
}

TEST_F(JSScriptTest, testMethodNotFound)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("function f() { return MyObject.foo(\'bar\', \'extraArg\') }");
    program->compile(ss, "filename");
    // script engine takes object ownership
    auto thread = program->createThread("f");
    EXPECT_THROW(thread->run(), metacpp::scripting::ScriptRuntimeError);
}

TEST_F(JSScriptTest, testSetProperty)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("obj = MyObject(); obj.xValue = 12; if (obj.x() !== 12) throw Error()");
    program->compile(ss, "filename");
    // script engine takes object ownership
    auto thread = program->createThread();
    thread->run();
}

TEST_F(JSScriptTest, testGetProperty)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("obj = MyObject(); if (obj.xValue !== 0) throw Error()");
    program->compile(ss, "filename");
    // script engine takes object ownership
    auto thread = program->createThread();
    thread->run();
}

TEST_F(JSScriptTest, testSetDynamicProperty)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("function f() { obj = MyObject(); obj.newProp = \'foo\'; return obj; }");
    program->compile(ss, "filename");
    // script engine takes object ownership
    auto thread = program->createThread("f");
    auto result = thread->run();
    auto obj = metacpp::variant_cast<MyObject *>(result);
    EXPECT_EQ(metacpp::variant_cast<metacpp::String>(obj->getProperty("newProp")), "foo");
}

TEST_F(JSScriptTest, testGetDynamicProperty)
{
    auto program = m_engine->createProgram();
    std::istringstream ss("function f(obj) { return obj[\'`123!@#\']; }");
    program->compile(ss, "filename");
    // script engine takes object ownership
    MyObject *obj = new MyObject();
    obj->setProperty("`123!@#", "bar");
    auto thread = program->createThread("f", obj);
    EXPECT_EQ(metacpp::variant_cast<metacpp::String>(thread->run()), "bar");
}

#endif
