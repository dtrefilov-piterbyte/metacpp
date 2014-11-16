#pragma once
#include "VisitorBase.h"

namespace orm
{

class pkInitVisitor :
	public VisitorBase
{
public:
	pkInitVisitor(bool omit);
	~pkInitVisitor(void);

	void visitField(Object *obj, const FieldInfoDescriptor *desc) override;
private:
	bool m_omit;
};

} // namespace pkapi
