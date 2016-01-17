#ifndef SCRIPTENGINEBASE_H
#define SCRIPTENGINEBASE_H
#include "ScriptProgramBase.h"
#include <MetaObject.h>
#include <stdexcept>

namespace metacpp {
namespace scripting {

class ScriptRuntimeError
        : std::exception
{
public:
    ScriptRuntimeError(const char *message, const char *filename,
                       uint32_t line, uint32_t column);

    const char *what() const noexcept override;
private:
    String m_what;
};

/** \brief Base class for all script engine VMs */
class ScriptEngineBase {
public:
    ScriptEngineBase();
    virtual ~ScriptEngineBase();

    std::shared_ptr<ScriptProgramBase> createProgram();

    void registerClass(const MetaObject *metaObject);

    Array<const MetaObject *> registeredClasses() const;
protected:

    /** \brief Create a new instance of ScriptProgramBase corresponding to this type of VM */
    virtual ScriptProgramBase *createProgramImpl() = 0;
    /** \brief Finalize and destroy ScriptProgramBase previously created by createProgram */
    virtual void closeProgramImpl(ScriptProgramBase *program) = 0;
private:
    Array<const MetaObject *> m_registeredClasses;
};

} // namespace scripting
} // namespace metacpp

#endif // SCRIPTENGINEBASE_H
