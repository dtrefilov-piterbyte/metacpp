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

static const char szTypeName[] = "@type";

JsonSerializerVisitor::JsonSerializerVisitor(void)
    : m_value(Json::objectValue)
{
}


JsonSerializerVisitor::~JsonSerializerVisitor(void)
{
}

void JsonSerializerVisitor::visitField(Object *obj, const MetaFieldBase *field)
{
    appendSubValue(m_value[field->name()], field->type(),
            reinterpret_cast<const char *>(obj) + field->offset(),
            field);
}

template<typename T>
const void *DereferenceNullable(const Nullable<T> *pValue)
{
    auto& f = *pValue;
    if (!f) return nullptr;
    return &f.get();
}

void JsonSerializerVisitor::appendSubValue(Json::Value& val, EFieldType type, const void *pValue, const MetaFieldBase *field)
{
    if (field && field->nullable())
    {
        switch (type)
        {
        default:
        case eFieldVoid:
            throw std::invalid_argument(std::string("Unsupported nullable field type: ") + (char *)type);
        case eFieldBool:
            pValue = DereferenceNullable(reinterpret_cast<const Nullable<bool> *>(pValue));
            break;
        case eFieldInt:
            pValue = DereferenceNullable(reinterpret_cast<const Nullable<int32_t> *>(pValue));
            break;
        case eFieldEnum:
        case eFieldUint:
            pValue = DereferenceNullable(reinterpret_cast<const Nullable<uint32_t> *>(pValue));
            break;
        case eFieldInt64:
            pValue = DereferenceNullable(reinterpret_cast<const Nullable<int64_t> *>(pValue));
            break;
        case eFieldUint64:
            pValue = DereferenceNullable(reinterpret_cast<const Nullable<uint64_t> *>(pValue));
            break;
        case eFieldFloat:
            pValue = DereferenceNullable(reinterpret_cast<const Nullable<float> *>(pValue));
            break;
        case eFieldDouble:
            pValue = DereferenceNullable(reinterpret_cast<const Nullable<double> *>(pValue));
            break;
        case eFieldDateTime:
            pValue = DereferenceNullable(reinterpret_cast<const Nullable<DateTime> *>(pValue));
            break;
        case eFieldVariant:
            pValue = DereferenceNullable(reinterpret_cast<const Nullable<Variant> *>(pValue));
        }
        if (!pValue)
            return;
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
        if (!field)
            throw std::invalid_argument("Nested arrays are not supported");
        const metacpp::Array<char> *arrayValue = reinterpret_cast<const metacpp::Array<char> *>(pValue);
        for (size_t i = 0; i < arrayValue->size(); ++i)
        {
            const void *pSubValue = arrayValue->data() + i * reinterpret_cast<const MetaFieldArray *>
                    (field)->arrayElementSize();
            appendSubValue(val[(Json::ArrayIndex)i],
                    reinterpret_cast<const MetaFieldArray *>(field)->arrayElementType(),
                    pSubValue);
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
        auto pDateTime = reinterpret_cast<const metacpp::DateTime *>(pValue);
        if (pDateTime->valid())
            val = pDateTime->toString().c_str();
        break;
    }
    case eFieldVariant: {
        auto pVariant = reinterpret_cast<const metacpp::Variant *>(pValue);
        if (pVariant->valid()) {
            if (pVariant->isArray()) {
                VariantArray array = variant_cast<VariantArray>(*pVariant);
                for (size_t i = 0; i < array.size(); ++i)
                    appendSubValue(val[(Json::ArrayIndex)i],
                            array[i].type(),
                            array[i].buffer());
            }
            else
                appendSubValue(val, pVariant->type(), pVariant->buffer());
        }
        break;
    }
    } // switch
}

const Json::Value& JsonSerializerVisitor::rootValue() const
{
    return m_value;
}

void JsonSerializerVisitor::previsitStruct(Object *obj)
{
    m_value[szTypeName] = obj->metaObject()->name();
}

} // namespace json
} // namespace serialization
} // namespace metacpp
