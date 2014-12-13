#ifndef INITVISITOR_H
#define INITVISITOR_H

#include "VisitorBase.h"

namespace metacpp
{

class pkInitVisitor :
	public VisitorBase
{
public:
    pkInitVisitor();
	~pkInitVisitor(void);

	void visitField(Object *obj, const FieldInfoDescriptor *desc) override;
};

} // namespace metacpp

#endif // INITVISITOR_H
