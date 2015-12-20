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
#ifndef BSONDESERIALIZERVISITOR_H
#define BSONDESERIALIZERVISITOR_H
#include "VisitorBase.h"
#include "Object.h"
#include "mongo/bson/bson.h"

namespace metacpp
{
namespace serialization
{
namespace bson
{

/** \brief Visitor for bson deserialization of objects */
class BsonDeserializerVisitor : public VisitorBase
{
public:
    /** \brief Constructs new instance of JsonDeserializerVisitor with a given value */
    explicit BsonDeserializerVisitor(const mongo::BSONObj& rootObj);
    ~BsonDeserializerVisitor();
protected:
    /** \brief Overrides VisitorBase::visitField */
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
