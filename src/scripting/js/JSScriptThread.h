#ifndef JSSCRIPTTHREAD_H
#define JSSCRIPTTHREAD_H
#include "ScriptThreadBase.h"
#include <jsapi.h>
#include <stdexcept>
#include <thread>
#include <condition_variable>

namespace metacpp {
namespace scripting {
namespace js {

class JSScriptEngine;

class JSScriptThread : public ScriptThreadBase
{
public:
    explicit JSScriptThread(const ByteArray& bytecode,
                            JSScriptEngine *engine);
    ~JSScriptThread();

    void setCallFunction(const String& functionName, const VariantArray& args);

    bool running() const override;
    Variant run() override;
    virtual bool abort() override;
private:
    void onError(const char *message, JSErrorReport *report);
    static void dispatchError(JSContext *ctx, const char *message, JSErrorReport *report);
#if MOZJS_MAJOR_VERSION >= 31
    static bool interruptCallback(JSContext *cx);
#else
    static JSBool operationCallback(JSContext *cx);
#endif
private:
    ByteArray m_bytecode;
    JSScriptEngine *m_engine;
    JSRuntime *m_runtime;
    String m_functionName;
    VariantArray m_arguments;
    Variant m_result;
    std::mutex m_runMutex;
    std::condition_variable m_finishedVariable;
    std::atomic<bool> m_bRunning;
    std::atomic<bool> m_bTerminating;
    std::exception_ptr m_exception;
};

} // namespace js
} // namespace scripting
} // namespace metacpp

#endif // JSSCRIPTTHREAD_H
