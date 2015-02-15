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

/** \brief Visitor for bson serialization of objects */
class BsonSerializerVisitor : public VisitorBase
{
public:
    /** \brief Constructs a new instance of BsonSerializerVisitor */
    BsonSerializerVisitor();
    ~BsonSerializerVisitor();

    /** \brief Gets a value containing introspecting object */
    mongo::BSONObj doneObj();
protected:
    /** \brief Overrides VisitorBase::visitField */
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
