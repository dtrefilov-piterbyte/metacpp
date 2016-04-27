#include "JSScriptProgram.h"
#include "JSScriptThread.h"
#include "JSScriptEngine.h"

namespace metacpp
{
namespace scripting
{
namespace js
{

JSScriptProgram::JSScriptProgram(JSScriptEngine *engine)
    : m_engine(engine)
{
}

JSScriptProgram::~JSScriptProgram()
{
}

void JSScriptProgram::compile(const void *pBuffer, size_t size, const String &filename)
{
    JS_AbortIfWrongThread(m_engine->rootRuntime());
    auto deleter = [](JSContext *c) { if (c) JS_DestroyContext(c); };
    std::unique_ptr<JSContext, decltype(deleter)> cx
    { JS_NewContext(m_engine->rootRuntime(), 8192), deleter };
    if (!cx)
        throw std::runtime_error("Could not create JS context");

#if MOZJS_MAJOR_VERSION >= 31
    JS::RootedObject global(cx.get(), JS_NewGlobalObject(cx.get(), m_engine->globalClass(), nullptr,
        JS::FireOnNewGlobalHook, JS::CompartmentOptions()));
#else
    JS::RootedObject global(cx.get(), JS_NewGlobalObject(cx.get(),
        const_cast<JSClass *>(m_engine->globalClass()), nullptr));
#endif
    JS_EnterCompartment(cx.get(), global);
    JS::CompileOptions options(cx.get());
    options.setFileAndLine(filename.c_str(), 1);
#if MOZJS_MAJOR_VERSION >= 38
    JS::RootedScript script(cx.get());
#if MOZJS_MAJOR_VERSION >= 46
    if (!JS::CompileForNonSyntacticScope(cx.get(), options, reinterpret_cast<const char *>(pBuffer), size, &script))
#else
    if (!JS::Compile(cx.get(), global, options, reinterpret_cast<const char *>(pBuffer), size, &script))
#endif
        throw std::runtime_error("Could not compile");
#else
    JS::RootedScript script(cx.get(), JS::Compile(cx.get(), global, options,
        reinterpret_cast<const char *>(pBuffer), size));
    if (!script)
        throw std::runtime_error("Could not compile");
#endif
    uint32_t bytecodeLength = 0;
    void *pBytecode = JS_EncodeScript(cx.get(), script, &bytecodeLength);
    m_bytecode = ByteArray(reinterpret_cast<const uint8_t *>(pBytecode), bytecodeLength);
    JS_free(cx.get(), pBytecode);
}

ScriptThreadBase *JSScriptProgram::createThreadImpl(const String &functionName, const VariantArray &args)
{
    std::lock_guard<std::mutex> _guard(m_threadsMutex);
    if (m_bytecode.empty())
        throw std::runtime_error("Program must be compiled first");
    JSScriptThread *thread =  new JSScriptThread(m_bytecode, m_engine);
    thread->setCallFunction(functionName, args);
    m_threads.push_back(thread);
    return thread;
}

void JSScriptProgram::closeThreadImpl(ScriptThreadBase *thread)
{
    std::lock_guard<std::mutex> _guard(m_threadsMutex);
    auto it = std::find(m_threads.begin(), m_threads.end(), thread);
    if (it == m_threads.end())
        throw std::invalid_argument("Thread not found");
    m_threads.erase(it);
    delete thread;
}

} // namespace js
} // namespace scripting
} // namespace metacpp
