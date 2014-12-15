#ifndef VISITOR_BASE_H
#define VISITOR_BASE_H
#include "Object.h"

namespace metacpp
{

class VisitorBase
{
public:
	VisitorBase();
	virtual ~VisitorBase(void);
	void visit(Object *obj);
protected:
    virtual void previsitStruct(Object *obj);
    virtual void visitField(Object *obj, const MetaField *);
    virtual void postvisitStruct(Object *obj);
};

} // namespace metacpp
#endif // VISITOR_BASE_H
