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
#include "JsonSerializerVisitor.h"
#include "Object.h"
#include <cassert>
#include <time.h>
#include <mutex>

namespace metacpp
{
namespace serialization
{
namespace json
{

JsonSerializerVisitor::JsonSerializerVisitor(void)
	: m_value(Json::objectValue)
{
}


JsonSerializerVisitor::~JsonSerializerVisitor(void)
{
}

void JsonSerializerVisitor::visitField(Object *obj, const MetaFieldBase *field)
{
    appendSubValue(m_value, field->type(), reinterpret_cast<char *>(obj) + field->offset(), field);
}

void JsonSerializerVisitor::appendSubValue(Json::Value& parent, EFieldType type, const void *pValue,
                                           const MetaFieldBase *field, Json::ArrayIndex i)
{
    Json::Value& val = field ? parent[field->name()] : parent[i];
#define ACCESS_NULLABLE(type) \
    auto& f = *reinterpret_cast<const Nullable<type > *>(pValue); \
    if (!f) return; \
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
        case eFieldDateTime: {
            ACCESS_NULLABLE(DateTime)
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
    case eFieldInt64:
        val = (Json::Int64)*reinterpret_cast<const int64_t *>(pValue);
        break;
    case eFieldUint64:
        val = (Json::UInt64)*reinterpret_cast<const uint64_t *>(pValue);
        break;
	case eFieldFloat:
		val = *reinterpret_cast<const float *>(pValue);
		break;
    case eFieldDouble:
        val = *reinterpret_cast<const double *>(pValue);
        break;
	case eFieldString:
        val = reinterpret_cast<const metacpp::String *>(pValue)->data();
		break;
	case eFieldEnum:
        if (field)
            val = reinterpret_cast<const MetaFieldEnum *>(field)->toString(*reinterpret_cast<const uint32_t *>(pValue));
		else
			val = *reinterpret_cast<const uint32_t *>(pValue);
		break;
	case eFieldArray:
	{
        assert(field);	// nested arrays are not allowed
        {
            const metacpp::Array<char> *arrayValue = reinterpret_cast<const metacpp::Array<char> *>(pValue);
            for (size_t i = 0; i < arrayValue->size(); ++i)
            {
                const void *pSubValue = arrayValue->data() + i * reinterpret_cast<const MetaFieldArray *>(field)->arrayElementSize();
                appendSubValue(val, reinterpret_cast<const MetaFieldArray *>(field)->arrayElementType(), pSubValue, nullptr, i);
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
    case eFieldDateTime: {
        val = reinterpret_cast<const metacpp::DateTime *>(pValue)->toString().c_str();
        break;
    }
	}	// switch
}

const Json::Value& JsonSerializerVisitor::rootValue() const
{
	return m_value;
}

} // namespace json
} // namespace serialization
} // namespace metacpp
