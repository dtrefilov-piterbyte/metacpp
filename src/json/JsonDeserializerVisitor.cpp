/****************************************************************************
* Copyright 2014-2015 Trefilov Dmitrij                                      *
*                                                                           *
* Licensed under the Apache License, Version 2.0 (the "License");           *
* you may not use this file except in compliance with the License.          *
* You may obtain a copy of the License at                                   *
*                                                                           *
*    http://www.apache.org/licenses/LICENSE-2.0                             *
*                                                                           *
* Unless required by applicable law or agreed to in writing, software       *
* distributed under the License is distributed on an "AS IS" BASIS,         *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
* See the License for the specific language governing permissions and       *
* limitations under the License.                                            *
****************************************************************************/
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

void JsonDeserializerVisitor::visitField(Object *obj, const MetaFieldBase *field)
{
    ParseValue(m_value, field->type(), reinterpret_cast<char *>(obj) + field->offset(), field);
}

void JsonDeserializerVisitor::ParseValue(const Json::Value& parent, EFieldType type, void *pValue, const MetaFieldBase *field, Json::ArrayIndex i)
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
        case eFieldInt64: {
            ACCESS_NULLABLE(int64_t)
            break;
        }
        case eFieldUint64: {
            ACCESS_NULLABLE(uint64_t)
            break;
        }
        case eFieldFloat: {
            ACCESS_NULLABLE(float)
            break;
        }
        case eFieldDouble: {
            ACCESS_NULLABLE(double)
            break;
        }
        case eFieldString: {
            ACCESS_NULLABLE(metacpp::String)
            break;
        }
        case eFieldDateTime: {
            ACCESS_NULLABLE(metacpp::DateTime);
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
    case eFieldInt64:
        if (!val.isIntegral()) throw std::invalid_argument("Type mismatch");
        *reinterpret_cast<int64_t *>(pValue) = val.asInt64();
        break;
    case eFieldUint64:
        if (!val.isIntegral()) throw std::invalid_argument("Type mismatch");
        *reinterpret_cast<uint64_t *>(pValue) = val.asUInt64();
        break;
	case eFieldFloat:
		if (!val.isDouble()) throw std::invalid_argument("Type mismatch");
		*reinterpret_cast<float *>(pValue) = (float)val.asDouble();
		break;
    case eFieldDouble:
        if (!val.isDouble()) throw std::invalid_argument("Type mismatch");
        *reinterpret_cast<double *>(pValue) = val.asDouble();
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
    case eFieldDateTime: {
        if (!val.isString()) throw std::invalid_argument("Type mismatch");
        *reinterpret_cast<DateTime *>(pValue) = DateTime::fromString(val.asCString());
        break;
    }
	}	// switch
}

} // namespace metacpp
