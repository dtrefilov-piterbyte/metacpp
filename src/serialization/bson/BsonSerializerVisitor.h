#ifndef BSONSERIALIZATIONVISITOR_H
#define BSONSERIALIZATIONVISITOR_H
#include "VisitorBase.h"
#include <mongo/bson/bson.h>

namespace metacpp
{
namespace serialization
{
namespace bson
{

class BsonSerializerVisitor : public VisitorBase
{
public:
    BsonSerializerVisitor();
    ~BsonSerializerVisitor();

    mongo::BSONObj doneObj();
protected:
    void visitField(Object *obj, const MetaFieldBase *field) override;
private:
    void appendSubValue(mongo::BSONObjBuilder &parent, EFieldType type, const void *pValue,
                        const MetaFieldBase *desc, int arrayIndex = 0);
private:
    mongo::BSONObjBuilder m_rootBuilder;
};

} // namespace bson
} // namespace serialization
} // namespace metacpp

#endif // BSONSERIALIZATIONVISITOR_H
