#pragma once
#include "Object.h"

namespace orm
{

class VisitorBase
{
public:
	VisitorBase();
	virtual ~VisitorBase(void);
	void visit(Object *obj);
protected:
	virtual void previsitStruct(Object *obj, const StructInfoDescriptor *);
	virtual void visitField(Object *obj, const FieldInfoDescriptor *);
	virtual void postvisitStruct(Object *obj, const StructInfoDescriptor *);
};

} // namespace pkapi
