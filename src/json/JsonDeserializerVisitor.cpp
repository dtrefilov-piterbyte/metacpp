#include "JsonDeserializerVisitor.h"
#include <cassert>

namespace orm
{

JsonDeserializerVisitor::JsonDeserializerVisitor(const Json::Value& val)
	: m_value(val)
{
}


JsonDeserializerVisitor::~JsonDeserializerVisitor(void)
{
}

void JsonDeserializerVisitor::visitField(Object *obj, const FieldInfoDescriptor *desc)
{
	ParseValue(m_value, desc->m_eType, reinterpret_cast<char *>(obj) + desc->m_dwOffset, desc);
}

void JsonDeserializerVisitor::ParseValue(const Json::Value& parent, EFieldType type, void *pValue, const FieldInfoDescriptor *desc, Json::ArrayIndex i)
{
	Json::Value val = desc ? parent.get(desc->m_pszName, Json::nullValue) : parent.get(i, Json::nullValue);
	switch (type)
	{
	default:
	case eFieldVoid:
		throw std::invalid_argument(std::string("Unknown field type: ") + (char *)type);
	case eFieldBool:
		if (!val.isBool()) throw std::invalid_argument("Type mismatch");
		*reinterpret_cast<bool *>(pValue) = val.asBool();
		break;
	case eFieldInt:
		if (!val.isInt()) throw std::invalid_argument("Type mismatch");
		*reinterpret_cast<int32_t *>(pValue) = val.asInt();
		break;
	case eFieldUint:
		if (!val.isUInt()) throw std::invalid_argument("Type mismatch");
		*reinterpret_cast<uint32_t *>(pValue) = val.asUInt();
		break;
	case eFieldFloat:
		if (!val.isDouble()) throw std::invalid_argument("Type mismatch");
		*reinterpret_cast<float *>(pValue) = (float)val.asDouble();
		break;
	case eFieldString:
		if (!val.isString()) throw std::invalid_argument("Type mismatch");
        *reinterpret_cast<orm::String *>(pValue) = val.asString();
		break;
	case eFieldEnum:
		if (desc && val.isString())
		{
            *reinterpret_cast<uint32_t *>(pValue) = desc->valueInfo.ext.m_enum.enumInfo->fromString(val.asString());
			break;
		}
		if (!val.isUInt()) throw std::invalid_argument("Type mismatch");
		*reinterpret_cast<uint32_t *>(pValue) = val.asUInt();
		break;
	case eFieldArray:
	{
		if (!val.isArray() && !val.isNull())  throw std::invalid_argument("Type mismatch");
		assert(desc);	// nested arrays are not allowed

        {
            orm::Array<char> *arrayValue = reinterpret_cast<orm::Array<char> *>(pValue);
            arrayValue->resize(val.size());
            for (size_t i = 0; i < val.size(); ++i)
            {
                void *pValue = arrayValue->data() + desc->valueInfo.ext.m_array.elemSize * i;
                ParseValue(val, desc->valueInfo.ext.m_array.elemType, pValue, nullptr, i);
            }
        }

		break;
	}
	case eFieldObject: {
        JsonDeserializerVisitor nestedSerializer(val);
        nestedSerializer.visit(reinterpret_cast<Object *>(pValue));
		break;
	}
	}	// switch
}

} // namespace pkapi
