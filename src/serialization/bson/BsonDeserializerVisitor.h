#ifndef BSONDESERIALIZERVISITOR_H
#define BSONDESERIALIZERVISITOR_H
#include "VisitorBase.h"
#include "mongo/bson/bson.h"

namespace metacpp
{
namespace serialization
{
namespace bson
{

class BsonDeserializerVisitor : public VisitorBase
{
public:
    BsonDeserializerVisitor(const mongo::BSONObj& rootObj);
    ~BsonDeserializerVisitor();
protected:
    void visitField(Object *obj, const MetaFieldBase *desc) override;
private:
    void parseValue(const mongo::BSONObj& parent, EFieldType type, void *pValue, const MetaFieldBase *desc = nullptr, int i = 0);
private:
    const mongo::BSONObj& m_rootObj;
};

} // namespace bson
} // namespace serialization
} // namespace metacpp

#endif // BSONDESERIALIZERVISITOR_H
