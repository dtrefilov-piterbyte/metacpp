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

void ScriptProgramBase::compile(std::istream &is, const String &filename)
{
    is.seekg (0, is.end);
    size_t length = is.tellg();
    is.seekg (0, is.beg);
    Array<char> buffer;
    buffer.resize(length);
    is.read(buffer.data(), buffer.size());
    compile(buffer.data(), buffer.size(), filename);
}

void ScriptProgramBase::compile(const String& sourceFile) {
    std::ifstream is(sourceFile.c_str(), std::ios_base::binary | std::ios_base::in);
    is.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
    return compile(is, sourceFile);
}

} // namespace scripting
} // namespace metacpp
