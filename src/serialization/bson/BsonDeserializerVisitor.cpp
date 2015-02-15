#include "BsonDeserializerVisitor.h"

namespace metacpp
{
namespace serialization
{
namespace bson
{

BsonDeserializerVisitor::BsonDeserializerVisitor(const mongo::BSONObj &rootObj)
    : m_rootObj(rootObj)
{
}

BsonDeserializerVisitor::~BsonDeserializerVisitor()
{
}

void BsonDeserializerVisitor::visitField(Object *obj, const MetaFieldBase *field)
{
    parseValue(m_rootObj, field->type(), reinterpret_cast<char *>(obj) + field->offset(), field);
}

void BsonDeserializerVisitor::parseValue(const mongo::BSONObj &parent, EFieldType type, void *pValue, const MetaFieldBase *field, int i)
{
    mongo::BSONElement val = field ? parent[field->name()] : parent[i];
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
        if (!val.isBoolean()) throw std::invalid_argument("Type mismatch: not a boolean");
        *reinterpret_cast<bool *>(pValue) = val.Bool();
        break;
    case eFieldInt:
    case eFieldUint:
    case eFieldEnum:
        if (!val.isNumber()) throw std::invalid_argument("Type mismatch: not an int");
        *reinterpret_cast<int32_t *>(pValue) = val.numberInt();
        break;
    case eFieldUint64:
    case eFieldInt64:
        if (!val.isNumber()) throw std::invalid_argument("Type mismatch: not a long");
        *reinterpret_cast<int64_t *>(pValue) = val.numberLong();
        break;
    case eFieldFloat:
        if (!val.isNumber()) throw std::invalid_argument("Type mismatch: not a number");
        *reinterpret_cast<float *>(pValue) = (float)val.numberDouble();
        break;
    case eFieldDouble:
        if (!val.isNumber()) throw std::invalid_argument("Type mismatch: not a number");
        *reinterpret_cast<double *>(pValue) = val.numberDouble();
        break;
    case eFieldString:
        if (mongo::String != val.type()) throw std::invalid_argument("Type mismatch: not a string");
        *reinterpret_cast<metacpp::String *>(pValue) = val.String();
        break;
    case eFieldArray:
    {
        if (!val.isABSONObj() && !val.isNull())  throw std::invalid_argument("Type mismatch: not an array");
        assert(field);	// nested arrays are not allowed
        mongo::BSONObj obj = val.Obj();
        if (!obj.couldBeArray()) throw std::invalid_argument("Type mismatch: not an array");

        {
            metacpp::Array<char> *arrayValue = reinterpret_cast<metacpp::Array<char> *>(pValue);
            arrayValue->resize(obj.nFields());
            for (size_t i = 0; i < (size_t)val.size(); ++i)
            {
                void *pValue = arrayValue->data() + reinterpret_cast<const MetaFieldArray *>(field)->arrayElementSize() * i;
                parseValue(obj, reinterpret_cast<const MetaFieldArray *>(field)->arrayElementType(), pValue, nullptr, i);
            }
        }

        break;
    }
    case eFieldObject: {
        if (!val.isABSONObj()) throw std::invalid_argument("Type mismatch: not an object");
        BsonDeserializerVisitor nestedSerializer(val.Obj());
        nestedSerializer.visit(reinterpret_cast<Object *>(pValue));
        break;
    }
    case eFieldDateTime: {
        if (mongo::Date != val.type()) throw std::invalid_argument("Type mismatch: not a timestamp");
        *reinterpret_cast<DateTime *>(pValue) = DateTime(val.Date().toTimeT());
        break;
    }
    }	// switch
}


} // namespace bson
} // namespace serialization
} // namespace metacpp
