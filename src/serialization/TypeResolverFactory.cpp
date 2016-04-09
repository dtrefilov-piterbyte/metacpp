#include "TypeResolverFactory.h"

namespace metacpp
{
namespace serialization
{

TypeResolverFactory::TypeResolverFactory(Array<const MetaObject *> knownTypes)
    : m_knownTypes(knownTypes)
{
}

Object *TypeResolverFactory::createInstance(String typeUri)
{
    for (const MetaObject *knownType : m_knownTypes) {
        if (knownType->name() == typeUri) {
            return knownType->createInstance();
        }
    }
    throw std::invalid_argument(String("Could not resolve type " + typeUri).c_str());
}

} // namespace serialization
} // namespace metacpp
