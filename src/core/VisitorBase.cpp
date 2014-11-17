#include "VisitorBase.h"

namespace metacpp
{

VisitorBase::VisitorBase()
{
}

VisitorBase::~VisitorBase(void)
{
}

void VisitorBase::previsitStruct(Object *obj, const StructInfoDescriptor *)
{
}

void VisitorBase::visitField(Object *obj, const FieldInfoDescriptor *)
{
}

void VisitorBase::postvisitStruct(Object *obj, const StructInfoDescriptor *)
{
}

void VisitorBase::visit(Object *obj)
{
	previsitStruct(obj, obj->metaObject()->structDescriptor());
	for (size_t i = 0; i < obj->metaObject()->totalFields(); ++i)
		visitField(obj, obj->metaObject()->fieldDescriptor(i));
	postvisitStruct(obj, obj->metaObject()->structDescriptor());
}

} // namespace metacpp
