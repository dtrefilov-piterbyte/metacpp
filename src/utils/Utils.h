#ifndef UTILS_H
#define UTILS_H

namespace metacpp
{

template<typename TBaseEntityPtr, typename... Args>
class FactoryBase
{
    virtual TBaseEntityPtr createInstance(Args... args) = 0;
};

} // namespace metacpp

#endif // UTILS_H

