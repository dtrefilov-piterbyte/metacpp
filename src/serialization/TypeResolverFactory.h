#ifndef TYPERESOLVERFACTORY_H
#define TYPERESOLVERFACTORY_H
#include <Utils.h>
#include <Object.h>
#include <SharedDataPointer.h>

namespace metacpp
{
namespace serialization
{

class TypeResolverFactory : FactoryBase<Object *, String>
{
public:
    explicit TypeResolverFactory(Array<const metacpp::MetaObject *> knownTypes);

    Object *createInstance(String typeUri) override;
private:
    Array<const metacpp::MetaObject *> m_knownTypes;
};

} // namespace serialization
} // namespace metacpp

#endif // TYPERESOLVERFACTORY_H
