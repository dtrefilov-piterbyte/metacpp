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
#ifndef BSONSERIALIZATIONVISITOR_H
#define BSONSERIALIZATIONVISITOR_H
#include "VisitorBase.h"
#include "Object.h"
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
