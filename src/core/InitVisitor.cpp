#include "InitVisitor.h"

namespace metacpp
{

pkInitVisitor::pkInitVisitor()
{
}


pkInitVisitor::~pkInitVisitor(void)
{
}

void pkInitVisitor::visitField(Object *obj, const FieldInfoDescriptor *desc)
{
    if (desc->m_nullable)
    {
        switch (desc->m_eType)
        {
        default:
        case eFieldVoid:
            throw std::invalid_argument(std::string("Unknown field type: ") + (char)desc->m_eType);
        case eFieldBool:
            if (eOptional == desc->valueInfo.mandatoriness)
                accessField<Nullable<bool > >(obj, desc).reset();
            else
                accessField<Nullable<bool > >(obj, desc) = desc->valueInfo.ext.m_bool.defaultValue;
            break;
        case eFieldInt:
            if (eOptional == desc->valueInfo.mandatoriness)
                accessField<Nullable<int32_t > >(obj, desc).reset();
            else
                accessField<Nullable<int32_t > >(obj, desc) = desc->valueInfo.ext.m_int.defaultValue;
            break;
        case eFieldUint:
            if (eOptional == desc->valueInfo.mandatoriness)
                accessField<Nullable<int32_t > >(obj, desc).reset();
            else
                accessField<Nullable<int32_t > >(obj, desc) = desc->valueInfo.ext.m_uint.defaultValue;
            break;
        case eFieldEnum:
            if (eOptional == desc->valueInfo.mandatoriness)
                accessField<Nullable<int32_t > >(obj, desc).reset();
            else
                accessField<Nullable<int32_t > >(obj, desc) = desc->valueInfo.ext.m_enum.enumInfo->m_defaultValue;
            break;
        case eFieldFloat:
            if (eOptional == desc->valueInfo.mandatoriness)
                accessField<Nullable<int32_t > >(obj, desc).reset();
            else
                accessField<Nullable<int32_t > >(obj, desc) = desc->valueInfo.ext.m_float.defaultValue;
            break;
        }
        return;
    }

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
        accessField<metacpp::String>(obj, desc) = desc->valueInfo.ext.m_string.defaultValue;
		break;
    case eFieldEnum:
        accessField<uint32_t>(obj, desc) = desc->valueInfo.ext.m_enum.enumInfo->m_defaultValue;
		break;
    case eFieldArray:
        accessField<metacpp::Array<char> >(obj, desc).clear();
		break;
    case eFieldObject: {
        accessField<Object>(obj, desc).init();
		break;
    }
    }
}

} // namespace metacpp
