#include "JSScriptThread.h"
#include "JSScriptEngine.h"
#include <thread>

namespace metacpp
{
namespace scripting
{
namespace js
{

JSScriptThread::JSScriptThread(JSRuntime *parentRuntime, const ByteArray &bytecode,
                               const JSClass *global_class)
    : m_runtime(nullptr), m_context(nullptr), m_script(nullptr), m_global(nullptr),
      m_bRunning(false), m_bTerminating(false)
{
    (void)parentRuntime;
    m_runtime = JS_NewRuntime(8 * 1024 * 1024, JS_USE_HELPER_THREADS);
    if (!m_runtime)
        throw std::runtime_error("Could not create JS runtime");
    JS_SetRuntimePrivate(m_runtime, this);
    m_context = JS_NewContext(m_runtime, 8192);
    if (!m_context)
        throw std::runtime_error("Could not create context");
    m_global = JS_NewGlobalObject(m_context, const_cast<JSClass *>(global_class), nullptr);
    JS_EnterCompartment(m_context, m_global);
    JS_InitStandardClasses(m_context, m_global);
    m_script = JS_DecodeScript(m_context, bytecode.data(), bytecode.size(), nullptr, nullptr);
    if (!m_script)
        throw std::runtime_error("Could not decode compiled script");
    JS_SetErrorReporter(m_context, JSScriptThread::dispatchError);
    JS_SetOperationCallback(m_context, JSScriptThread::operationCallback);
}

JSScriptThread::~JSScriptThread()
{
    if (m_context) {
        JS_LeaveCompartment(m_context, nullptr);
        JS_DestroyContext(m_context);
    }
    if (m_runtime) {
        JS_DestroyRuntime(m_runtime);
    }
}

bool JSScriptThread::running() const
{
    return false;
}

void JSScriptThread::run()
{
    if (!m_script)
        throw std::runtime_error("Invalid script");

    JS_SetRuntimeThread(m_runtime);
    m_exception = nullptr;

    JS::Value value;

    m_bRunning = true;
    bool bRes = JS_ExecuteScript(m_context, m_global, m_script, &value);
    m_bRunning = false;
    if (!bRes)
    {
        if (m_exception)
            std::rethrow_exception(m_exception);
        else
            throw std::runtime_error("Execution failed");
    }

    m_bRunning = false;
}

bool JSScriptThread::abort()
{
    m_bTerminating = true;
    JS_TriggerOperationCallback(m_runtime);
    uint32_t ctr = 0;
    while (m_bRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (ctr++ >= 50)
            return false;
    }
    return true;
}

void JSScriptThread::onError(const char *message, JSErrorReport *report)
{
    m_exception = std::make_exception_ptr(
                ScriptRuntimeError(message, report->filename, report->lineno, report->column));
}

void JSScriptThread::dispatchError(JSContext *ctx, const char *message, JSErrorReport *report)
{
    JSScriptThread *thread = reinterpret_cast<JSScriptThread *>
            (JS_GetRuntimePrivate(JS_GetRuntime(ctx)));
    thread->onError(message, report);
}

JSBool JSScriptThread::operationCallback(JSContext *cx)
{
    JSRuntime *runtime = JS_GetRuntime(cx);
    JSScriptThread *thread = reinterpret_cast<JSScriptThread *>
            (JS_GetRuntimePrivate(runtime));
    if (thread->m_bTerminating) {
        thread->m_exception = std::make_exception_ptr(TerminationException());
        return JS_FALSE;
    }
    return JS_TRUE;
}



} // namespace js
} // namespace scripting
} // namespace metacpp
