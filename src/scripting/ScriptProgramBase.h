#ifndef SCRIPTPROGRAMBASE_H
#define SCRIPTPROGRAMBASE_H
#include <iostream>
#include <memory>
#include <StringBase.h>
#include <Variant.h>

namespace metacpp {
namespace scripting {

class ScriptThreadBase;

class ScriptProgramBase {
public:
    ScriptProgramBase();
    virtual ~ScriptProgramBase();

    std::shared_ptr<ScriptThreadBase> createThread(const String& functionName = String(),
                                                  const VariantArray& args = VariantArray());

    template<typename... TArgs>
    std::shared_ptr<ScriptThreadBase> createThread(const String& functionName,
                                                   TArgs... args)
    {
        return createThread(functionName, {args...});
    }

public:
    virtual void compile(std::istream& source, const String& fileName) = 0;

    virtual void compile(const String& sourceFile);
    virtual void compile(const void *pBuffer, size_t size, const String& fileName);
protected:
    virtual ScriptThreadBase *createThreadImpl(const String& functionName,
                                           const VariantArray& args = VariantArray()) = 0;
    virtual void closeThreadImpl(ScriptThreadBase *thread) = 0;

};

} // namespace scripting
} // namespace metacpp

#endif // SCRIPTPROGRAMBASE_H
