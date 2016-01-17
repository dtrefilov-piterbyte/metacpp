#include "JSScriptEngine.h"
#include "JSScriptProgram.h"

namespace metacpp {
namespace scripting {
namespace js {

JSScriptEngine::JSScriptEngine()
    : m_runtime(nullptr),
      m_mainThreadId(std::this_thread::get_id())
{
    static struct SpiderMonkeyInit {
        SpiderMonkeyInit()
        {
#if MOZJS_MAJOR_VERSION >= 31
            JS_Init();
#endif

        }

        ~SpiderMonkeyInit()
        {
            JS_ShutDown();
        }
    } s_init;

    memset(&m_globalClass, 0, sizeof(JSClass));
    m_globalClass.name = "global";
    m_globalClass.flags = JSCLASS_GLOBAL_FLAGS;
#if MOZJS_MAJOR_VERSION < 38
    m_globalClass.addProperty = JS_PropertyStub;
    m_globalClass.delProperty = JS_DeletePropertyStub;
    m_globalClass.getProperty = JS_PropertyStub;
    m_globalClass.setProperty = JS_StrictPropertyStub;
    m_globalClass.enumerate = JS_EnumerateStub;
    m_globalClass.resolve = JS_ResolveStub;
    m_globalClass.convert = JS_ConvertStub;
#endif

#if MOZJS_MAJOR_VERSION >= 38
    m_runtime = JS_NewRuntime(JS::DefaultHeapMaxBytes);
#else
    m_runtime = JS_NewRuntime(32 * 1024 * 1024, JS_NO_HELPER_THREADS);
#endif
    if (!m_runtime)
        throw std::runtime_error("Could not initialize JS runtime");
}

JSScriptEngine::~JSScriptEngine()
{
    std::lock_guard<std::mutex> _guard(m_programsMutex);
    if (m_programs.size())
    {
        std::cerr << m_programs.size() << "  left unclosed" << std::endl;
        for (auto program : m_programs)
            delete program;
    }
    if (m_runtime)
        JS_DestroyRuntime(m_runtime);
}

JSRuntime *JSScriptEngine::rootRuntime() const
{
    return m_runtime;
}

const JSClass *JSScriptEngine::globalClass() const
{
    return &m_globalClass;
}

bool JSScriptEngine::isInMainThread() const
{
    return std::this_thread::get_id() == m_mainThreadId;
}

ScriptProgramBase *JSScriptEngine::createProgramImpl()
{
    std::lock_guard<std::mutex> _guard(m_programsMutex);
    m_programs.push_back(new JSScriptProgram(this));
    return m_programs.back();
}

void JSScriptEngine::closeProgramImpl(ScriptProgramBase *program)
{
    std::lock_guard<std::mutex> _guard(m_programsMutex);
    auto it = std::find(m_programs.begin(), m_programs.end(), program);
    if (it == m_programs.end())
        throw std::invalid_argument("Program not found");
    m_programs.erase(it);
    delete program;
}

} // namespace js
} // namespace scripting
} // namespace metacpp
