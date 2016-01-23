#include "ScriptProgramBase.h"
#include <streambuf>
#include <fstream>

namespace metacpp {
namespace scripting {

ScriptProgramBase::ScriptProgramBase() {
}

ScriptProgramBase::~ScriptProgramBase() {
}

SharedObjectPointer<ScriptThreadBase> ScriptProgramBase::createThread(const String &functionName, const VariantArray &args)
{
    auto deleter = [this](ScriptThreadBase *thread) { closeThreadImpl(thread); };
    return SharedObjectPointer<ScriptThreadBase>(createThreadImpl(functionName, args), deleter);
}

void ScriptProgramBase::compile(const String& sourceFile) {
    std::ifstream is(sourceFile.c_str(), std::ios_base::binary | std::ios_base::in);
    is.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
    return compile(is, "filename");
}

namespace {
struct membuf : public std::streambuf {
    membuf(const char *pBuffer, size_t size) {
        setg(const_cast<char *>(pBuffer), const_cast<char *>(pBuffer),
             const_cast<char *>(pBuffer) + size);
    }
};
} // namespace `anonymous'

void ScriptProgramBase::compile(const void *pBuffer, size_t size, const String &fileName) {
    membuf buf(reinterpret_cast<const char *>(pBuffer), size);
    std::istream is(&buf);
    return compile(is, fileName);
}

} // namespace scripting
} // namespace metacpp
