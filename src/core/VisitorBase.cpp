#include "VisitorBase.h"

namespace metacpp
{

VisitorBase::VisitorBase()
{
}

VisitorBase::~VisitorBase(void)
{
}

void VisitorBase::previsitStruct(Object *obj)
{
    (void)obj;
}

void VisitorBase::visitField(Object *obj, const MetaField *)
{
    (void)obj;
}

void VisitorBase::postvisitStruct(Object *obj)
{
    (void)obj;
}

void VisitorBase::visit(Object *obj)
{
    previsitStruct(obj);
	for (size_t i = 0; i < obj->metaObject()->totalFields(); ++i)
        visitField(obj, obj->metaObject()->field(i));
    postvisitStruct(obj);
}

} // namespace metacpp
