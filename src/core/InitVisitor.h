#pragma once
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
