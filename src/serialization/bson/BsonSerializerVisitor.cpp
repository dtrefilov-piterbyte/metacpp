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
#include "BsonSerializerVisitor.h"

namespace metacpp
{
namespace serialization
{
namespace bson
{

BsonSerializerVisitor::BsonSerializerVisitor()
{

}

BsonSerializerVisitor::~BsonSerializerVisitor()
{

}

void BsonSerializerVisitor::visitField(Object *obj, const MetaFieldBase *field)
{
    appendSubValue(m_rootBuilder, field->type(), reinterpret_cast<char *>(obj) + field->offset(), field);
}

mongo::BSONObj BsonSerializerVisitor::doneObj()
{
    return m_rootBuilder.obj();
}

void BsonSerializerVisitor::appendSubValue(mongo::BSONObjBuilder &parent, EFieldType type, const void *pValue, const MetaFieldBase *field, int arrayIndex)
{
    String fieldName = field ? field->name() : String::fromValue(arrayIndex);
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
        parent.append(fieldName.c_str(), *reinterpret_cast<const bool *>(pValue));
        break;
    case eFieldInt:
        parent.append(fieldName.c_str(), *reinterpret_cast<const int32_t *>(pValue));
        break;
    case eFieldEnum:
    case eFieldUint:
        parent.append(fieldName.c_str(), *reinterpret_cast<const uint32_t *>(pValue));
        break;
    case eFieldInt64:
        parent.append(fieldName.c_str(), (long long)*reinterpret_cast<const int64_t *>(pValue));
        break;
    case eFieldUint64:
        parent.append(fieldName.c_str(), (long long)*reinterpret_cast<const uint64_t *>(pValue));
        break;
    case eFieldFloat:
        parent.append(fieldName.c_str(), *reinterpret_cast<const float *>(pValue));
        break;
    case eFieldDouble:
        parent.append(fieldName.c_str(), *reinterpret_cast<const double *>(pValue));
        break;
    case eFieldString:
        parent.append(fieldName.c_str(), reinterpret_cast<const String *>(pValue)->c_str());
        break;
    case eFieldArray:
    {
        assert(field);	// nested arrays are not allowed
        {
            const metacpp::Array<char> *arrayValue = reinterpret_cast<const metacpp::Array<char> *>(pValue);
            mongo::BSONObjBuilder arrayBuilder;
            for (size_t i = 0; i < arrayValue->size(); ++i)
            {
                const void *pSubValue = arrayValue->data() + i * reinterpret_cast<const MetaFieldArray *>(field)->arrayElementSize();
                appendSubValue(arrayBuilder, reinterpret_cast<const MetaFieldArray *>(field)->arrayElementType(), pSubValue, nullptr, i);
            }
            parent.appendArray(fieldName.c_str(), arrayBuilder.obj());
        }
        break;
    }
    case eFieldObject: {
        BsonSerializerVisitor nestedSerializer;
        nestedSerializer.visit(const_cast<Object *>(reinterpret_cast<const Object *>(pValue)));
        parent.append(fieldName.c_str(), nestedSerializer.doneObj());
        break;
    }
    case eFieldDateTime: {
        time_t tt = reinterpret_cast<const metacpp::DateTime *>(pValue)->toStdTime();
        parent.append(fieldName.c_str(), mongo::Date_t(tt * 1000));
        break;
    }
    }	// switch
}

} // namespace bson
} // namespace serialization
} // namespace metacpp
