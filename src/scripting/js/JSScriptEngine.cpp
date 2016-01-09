#include "JSScriptEngine.h"
#include "JSScriptProgram.h"

#if MOZJS_MAJOR_VERSION > 31
#include <js/Initialization.h>
#endif

namespace metacpp {
namespace scripting {
namespace js {

JSScriptEngine::JSScriptEngine()
    : m_runtime(nullptr)
{
    static struct SpiderMonkeyInit {
        SpiderMonkeyInit()
        {
#if MOZJS_MAJOR_VERSION > 31
            JS_Init();
#endif

        }

        ~SpiderMonkeyInit()
        {
            JS_ShutDown();
        }
    } s_init;

    memset(&m_global_class, 0, sizeof(JSClass));
    m_global_class.name = "global";
    m_global_class.flags = JSCLASS_GLOBAL_FLAGS;
    m_global_class.addProperty = JS_PropertyStub;
    m_global_class.delProperty = JS_DeletePropertyStub;
    m_global_class.getProperty = JS_PropertyStub;
    m_global_class.setProperty = JS_StrictPropertyStub;
    m_global_class.enumerate = JS_EnumerateStub;
    m_global_class.resolve = JS_ResolveStub;
    m_global_class.convert = JS_ConvertStub;

    m_runtime = JS_NewRuntime(8 * 1024 * 1024, JS_NO_HELPER_THREADS);
    if (!m_runtime)
        throw std::runtime_error("Could not initialize JS runtime");
    JS_SetRuntimePrivate(m_runtime, this);
    JS_ClearRuntimeThread(m_runtime);
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

ScriptProgramBase *JSScriptEngine::createProgramImpl()
{
    std::lock_guard<std::mutex> _guard(m_programsMutex);
    m_programs.push_back(new JSScriptProgram(m_runtime, &m_global_class));
    return m_programs.back();
}

void JSScriptEngine::closeProgramImpl(ScriptProgramBase *program)
{
    std::lock_guard<std::mutex> _guard(m_programsMutex);
    JSScriptProgram *jsProgram = dynamic_cast<JSScriptProgram *>(program);
    if (!jsProgram)
        throw std::invalid_argument("Not a JS program");
    auto it = std::find(m_programs.begin(), m_programs.end(), jsProgram);
    if (it == m_programs.end())
        throw std::invalid_argument("Program not found");
    m_programs.erase(it);
    delete jsProgram;
}

} // namespace js
} // namespace scripting
} // namespace metacpp
