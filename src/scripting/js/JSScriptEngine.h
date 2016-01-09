#ifndef JSSCRIPTENGINE_H
#define JSSCRIPTENGINE_H

#include "ScriptEngineBase.h"
#include "JSScriptProgram.h"
#include <jsapi.h>

namespace metacpp {
namespace scripting {
namespace js {

class JSScriptEngine : public ScriptEngineBase
{
public:
    explicit JSScriptEngine();
    ~JSScriptEngine();
protected:
    ScriptProgramBase *createProgramImpl() override;
    void closeProgramImpl(ScriptProgramBase *program) override;
private:
    Array<JSScriptProgram *> m_programs;
    mutable std::mutex m_programsMutex;
    JSRuntime *m_runtime;
    JSClass m_global_class;
};

} // namespace js
} // namespace scripting
} // namespace metacpp

#endif // JSSCRIPTENGINE_H
