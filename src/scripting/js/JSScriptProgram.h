#ifndef JSSCRIPTPROGRAM_H
#define JSSCRIPTPROGRAM_H
#include <Array.h>
#include "ScriptProgramBase.h"
#include "JSScriptThread.h"
#include <jsapi.h>
#include <mutex>

namespace metacpp
{
namespace scripting
{
namespace js
{

class JSScriptProgram : public ScriptProgramBase
{
public:
    explicit JSScriptProgram(JSRuntime *runtime, const JSClass *global_class);
    ~JSScriptProgram();

    void compile(std::istream& source, const String& filename) override;
    void compile(const void *pBuffer, size_t size, const String& filename) override;
protected:
    ScriptThreadBase *createThreadImpl(const String& functionName,
                                       const VariantArray& args = VariantArray()) override;
    void closeThreadImpl(ScriptThreadBase *thread) override;
private:
    JSRuntime *m_runtime;
    const JSClass *m_global_class;
    ByteArray m_bytecode;
    std::mutex m_threadsMutex;
    Array<ScriptThreadBase *> m_threads;
};

} // namespace js
} // namespace scripting
} // namespace metacpp


#endif // JSSCRIPTPROGRAM_H
