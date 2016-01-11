#ifndef JSSCRIPTTHREAD_H
#define JSSCRIPTTHREAD_H
#include "ScriptThreadBase.h"
#include <jsapi.h>
#include <stdexcept>

namespace metacpp {
namespace scripting {
namespace js {

class JSScriptThread : public ScriptThreadBase
{
public:
    explicit JSScriptThread(JSRuntime *parentRuntime, const ByteArray& bytecode,
                            const JSClass *m_global_class);
    ~JSScriptThread();

    void setCallFunction(const String& functionName, const VariantArray& args);

    bool running() const override;
    Variant run() override;
    virtual bool abort() override;
private:
    void onError(const char *message, JSErrorReport *report);
    static void dispatchError(JSContext *ctx, const char *message, JSErrorReport *report);
    static JSBool operationCallback(JSContext *cx);
private:
    String m_functionName;
    VariantArray m_arguments;
    Variant m_result;
    JSRuntime *m_runtime;
    JSContext *m_context;
    JSScript *m_script;
    JSObject *m_global;
    std::atomic<bool> m_bRunning;
    std::atomic<bool> m_bTerminating;
    std::exception_ptr m_exception;
};

} // namespace js
} // namespace scripting
} // namespace metacpp

#endif // JSSCRIPTTHREAD_H
