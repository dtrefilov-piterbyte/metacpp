#ifndef JSSCRIPTENGINE_H
#define JSSCRIPTENGINE_H

#include "ScriptEngineBase.h"
#include "JSScriptProgram.h"
#include <jsapi.h>
#include <thread>

namespace metacpp {
namespace scripting {
namespace js {

class JSScriptEngine : public ScriptEngineBase
{
public:
    explicit JSScriptEngine();
    ~JSScriptEngine();

    JSRuntime *rootRuntime() const;
    const JSClass *globalClass() const;
    bool isInMainThread() const;
protected:
    ScriptProgramBase *createProgramImpl() override;
    void closeProgramImpl(ScriptProgramBase *program) override;
private:
    Array<ScriptProgramBase *> m_programs;
    mutable std::mutex m_programsMutex;
    JSRuntime *m_runtime;
    JSClass m_globalClass;
    std::thread::id m_mainThreadId;
};

} // namespace js
} // namespace scripting
} // namespace metacpp

#endif // JSSCRIPTENGINE_H
