#include "JsonSerializerVisitor.h"
#include <cassert>

namespace metacpp
{

JsonSerializerVisitor::JsonSerializerVisitor(void)
	: m_value(Json::objectValue)
{
}


JsonSerializerVisitor::~JsonSerializerVisitor(void)
{
}

void JsonSerializerVisitor::visitField(Object *obj, const FieldInfoDescriptor *desc)
{
	appendSubValue(m_value, desc->m_eType, reinterpret_cast<char *>(obj) + desc->m_dwOffset, desc);
}

void JsonSerializerVisitor::appendSubValue(Json::Value& parent, EFieldType type, const void *pValue, const FieldInfoDescriptor *desc, Json::ArrayIndex i)
{
	Json::Value& val = desc ? parent[desc->m_pszName] : parent[i];
#define ACCESS_NULLABLE(type) \
    auto& field = *reinterpret_cast<const Nullable<type > *>(pValue); \
    if (!field) return; \
    pValue = &field.get();

    if (desc && desc->m_nullable)
    {
        switch (type)
        {
        default:
        case eFieldVoid:
            throw std::invalid_argument(std::string("Unsupported nullable field type: ") + (char *)type);
        case eFieldBool: {
            ACCESS_NULLABLE(bool)
            break;
        }
        case eFieldInt: {
            ACCESS_NULLABLE(int32_t)
            break;
        }
        case eFieldEnum:
        case eFieldUint: {
            ACCESS_NULLABLE(uint32_t)
            break;
        }
        case eFieldFloat: {
            ACCESS_NULLABLE(float)
            break;
        }
        }
    }
	switch (type)
	{
	default:
	case eFieldVoid:
		throw std::invalid_argument(std::string("Unknown field type: ") + (char *)type);
	case eFieldBool:
		val = *reinterpret_cast<const bool *>(pValue);
		break;
	case eFieldInt:
		val = *reinterpret_cast<const int32_t *>(pValue);
		break;
	case eFieldUint:
		val = *reinterpret_cast<const uint32_t *>(pValue);
		break;
	case eFieldFloat:
		val = *reinterpret_cast<const float *>(pValue);
		break;
	case eFieldString:
        val = reinterpret_cast<const metacpp::String *>(pValue)->data();
		break;
	case eFieldEnum:
		if (desc)
            val = desc->valueInfo.ext.m_enum.enumInfo->toString(*reinterpret_cast<const uint32_t *>(pValue));
		else
			val = *reinterpret_cast<const uint32_t *>(pValue);
		break;
	case eFieldArray:
	{
		assert(desc);	// nested arrays are not allowed
        {
            const metacpp::Array<char> *arrayValue = reinterpret_cast<const metacpp::Array<char> *>(pValue);
            for (size_t i = 0; i < arrayValue->size(); ++i)
            {
                const void *pSubValue = arrayValue->data() + i * desc->valueInfo.ext.m_array.elemSize;
                appendSubValue(val, desc->valueInfo.ext.m_array.elemType, pSubValue, nullptr, i);
            }
        }
		break;
	}
	case eFieldObject: {
        JsonSerializerVisitor nestedSerializer;
        nestedSerializer.visit(const_cast<Object *>(reinterpret_cast<const Object *>(pValue)));
		val = nestedSerializer.rootValue();
		break;
	}
	}	// switch
}

const Json::Value& JsonSerializerVisitor::rootValue() const
{
	return m_value;
}

} // namespace metacpp
