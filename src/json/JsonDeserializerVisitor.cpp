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

void JsonDeserializerVisitor::visitField(Object *obj, const MetaField *field)
{
    ParseValue(m_value, field->type(), reinterpret_cast<char *>(obj) + field->offset(), field);
}

void JsonDeserializerVisitor::ParseValue(const Json::Value& parent, EFieldType type, void *pValue, const MetaField *field, Json::ArrayIndex i)
{
    Json::Value val = field ? parent.get(field->name(), Json::nullValue) : parent.get(i, Json::nullValue);
#define ACCESS_NULLABLE(type) \
    auto& f = *reinterpret_cast<Nullable<type > *>(pValue); \
    f.reset(!val.isNull()); \
    if (val.isNull()) return; \
    pValue = &f.get();

    if (field && field->nullable())
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
        if (field && val.isString())
		{
            *reinterpret_cast<uint32_t *>(pValue) = reinterpret_cast<const MetaFieldEnum *>(field)->fromString(val.asCString());
			break;
		}
		if (!val.isUInt()) throw std::invalid_argument("Type mismatch");
		*reinterpret_cast<uint32_t *>(pValue) = val.asUInt();
		break;
	case eFieldArray:
	{
		if (!val.isArray() && !val.isNull())  throw std::invalid_argument("Type mismatch");
        assert(field);	// nested arrays are not allowed

        {
            metacpp::Array<char> *arrayValue = reinterpret_cast<metacpp::Array<char> *>(pValue);
            arrayValue->resize(val.size());
            for (size_t i = 0; i < val.size(); ++i)
            {
                void *pValue = arrayValue->data() + reinterpret_cast<const MetaFieldArray *>(field)->arrayElementSize() * i;
                ParseValue(val, reinterpret_cast<const MetaFieldArray *>(field)->arrayElementType(), pValue, nullptr, i);
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
