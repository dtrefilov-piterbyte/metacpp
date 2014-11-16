#include "InitVisitor.h"

namespace orm
{

pkInitVisitor::pkInitVisitor(bool omit)
	: m_omit(omit)
{
}


pkInitVisitor::~pkInitVisitor(void)
{
}

void pkInitVisitor::visitField(Object *obj, const FieldInfoDescriptor *desc)
{
	if (m_omit || (desc->valueInfo.mandatoriness == eOptional)) return;
	switch (desc->m_eType)
	{
	default:
	case eFieldVoid:
		throw std::invalid_argument(std::string("Unknown field type: ") + (char)desc->m_eType);
	case eFieldBool:
        accessField<bool>(obj, desc) = desc->valueInfo.ext.m_bool.defaultValue;
		break;
	case eFieldInt:
        accessField<int32_t>(obj, desc) = desc->valueInfo.ext.m_int.defaultValue;
		break;
	case eFieldUint:
        accessField<uint32_t>(obj, desc) = desc->valueInfo.ext.m_uint.defaultValue;
		break;
	case eFieldFloat:
        accessField<float>(obj, desc) = desc->valueInfo.ext.m_float.defaultValue;
		break;
	case eFieldString:
        accessField<orm::String>(obj, desc) = desc->valueInfo.ext.m_string.defaultValue;
		break;
	case eFieldEnum:
        accessField<uint32_t>(obj, desc) = desc->valueInfo.ext.m_enum.enumInfo->m_defaultValue;
		break;
	case eFieldArray:
        accessField<orm::Array<char> >(obj, desc).clear();
		break;
	case eFieldObject: {
		accessField<Object>(obj, desc).init();
		break;
	}
	}
}

} // namespace pkapi
