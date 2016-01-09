#ifndef SCRIPTABLE_H
#define SCRIPTABLE_H
#include "Object.h"

namespace metacpp
{
namespace scripting
{

/** \brief Base class for objects bindable to ScriptEngineBase */
class ScriptableBase {
public:
    ScriptableBase();
    virtual ~ScriptableBase();

    virtual MetaObject *metaObject() const = 0;
};

template<typename TObj>
class Scriptable : public ScriptableBase {
public:

    Scriptable() {
    }

    ~Scriptable() {
    }

    MetaObject *metaObject() const {
        return TObj::staticMetaObject();
    }
};

} // namespace scripting
} // namespace metacpp

#endif // SCRIPTABLE_H
