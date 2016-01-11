#include "JSScriptProgram.h"
#include "JSScriptThread.h"

namespace metacpp
{
namespace scripting
{
namespace js
{

JSScriptProgram::JSScriptProgram(JSRuntime *runtime, const JSClass *global_class)
    : m_runtime(runtime), m_global_class(global_class)
{
}

JSScriptProgram::~JSScriptProgram()
{
}

void JSScriptProgram::compile(std::istream &is, const String &filename)
{
    is.seekg (0, is.end);
    size_t length = is.tellg();
    is.seekg (0, is.beg);
    Array<char> buffer;
    buffer.resize(length);
    is.read(buffer.data(), buffer.size());
    compile(buffer.data(), buffer.size(), filename);
}

void JSScriptProgram::compile(const void *pBuffer, size_t size, const String &filename)
{
    std::unique_ptr<JSContext, std::function<void(JSContext *)> > cx
    { JS_NewContext(m_runtime, 8192),
                [](JSContext *c) { if (c) JS_DestroyContext(c); } };
    if (!cx)
        throw std::runtime_error("Could not create JS context");

    JS::CompileOptions options(cx.get());
    options.setFileAndLine(filename.c_str(), 1);
    JS::RootedObject global(cx.get(), JS_NewGlobalObject(cx.get(),
                                                         const_cast<JSClass *>(m_global_class), nullptr));
    {
        JSAutoCompartment ac(cx.get(), global);
        JSScript *script = JS::Compile(cx.get(), global, options, reinterpret_cast<const char *>(pBuffer), size);
        if (!script)
            throw std::runtime_error("Could not compile");
        uint32_t bytecodeLength = 0;
        void *pBytecode = JS_EncodeScript(cx.get(), script, &bytecodeLength);
        m_bytecode = ByteArray(reinterpret_cast<const uint8_t *>(pBytecode), bytecodeLength);
    }
}

ScriptThreadBase *JSScriptProgram::createThreadImpl(const String &functionName, const VariantArray &args)
{
    std::lock_guard<std::mutex> _guard(m_threadsMutex);
    if (m_bytecode.empty())
        throw std::runtime_error("Program must be compiled first");
    JSScriptThread *thread =  new JSScriptThread(m_runtime, m_bytecode, m_global_class);
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
