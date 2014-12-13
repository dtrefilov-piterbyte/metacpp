#include "JsonDeserializerVisitor.h"
#include <cassert>
#include <time.h>

namespace metacpp
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
#define ACCESS_NULLABLE(type) \
    auto& field = *reinterpret_cast<Nullable<type > *>(pValue); \
    field.reset(!val.isNull()); \
    if (val.isNull()) return; \
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
        case eFieldString: {
            ACCESS_NULLABLE(metacpp::String)
            break;
        }
        case eFieldTime: {
            ACCESS_NULLABLE(std::time_t);
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
		if (!val.isBool()) throw std::invalid_argument("Type mismatch");
		*reinterpret_cast<bool *>(pValue) = val.asBool();
		break;
	case eFieldInt:
        if (!val.isIntegral()) throw std::invalid_argument("Type mismatch");
		*reinterpret_cast<int32_t *>(pValue) = val.asInt();
		break;
	case eFieldUint:
        if (!val.isIntegral()) throw std::invalid_argument("Type mismatch");
		*reinterpret_cast<uint32_t *>(pValue) = val.asUInt();
		break;
	case eFieldFloat:
		if (!val.isDouble()) throw std::invalid_argument("Type mismatch");
		*reinterpret_cast<float *>(pValue) = (float)val.asDouble();
		break;
	case eFieldString:
		if (!val.isString()) throw std::invalid_argument("Type mismatch");
        *reinterpret_cast<metacpp::String *>(pValue) = val.asString();
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
            metacpp::Array<char> *arrayValue = reinterpret_cast<metacpp::Array<char> *>(pValue);
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
    case eFieldTime: {
        if (!val.isString()) throw std::invalid_argument("Type mismatch");
        struct tm tm;
        strptime(val.asCString(), "%Y-%m-%d %H:%M:%S", &tm);
        *reinterpret_cast<std::time_t *>(pValue) = timegm(&tm);
        break;
    }
	}	// switch
}

} // namespace metacpp
