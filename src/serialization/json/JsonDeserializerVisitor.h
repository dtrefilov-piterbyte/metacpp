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
#ifndef JSONDESERIALIZERVISITOR_H
#define JSONDESERIALIZERVISITOR_H
#include "VisitorBase.h"
#include "MetaInfo.h"
#include <json/json.h>

namespace metacpp
{
namespace serialization
{
namespace json
{

/** \brief Visitor for json deserialization of objects */
class JsonDeserializerVisitor :
	public VisitorBase
{
public:
    /** \brief Constructs new instance of JsonDeserializerVisitor with a given value */
    JsonDeserializerVisitor(const Json::Value& value);
    ~JsonDeserializerVisitor(void);
protected:
    /** \brief Overrides VisitorBase::visitField */
    void visitField(Object *obj, const MetaFieldBase *desc) override;
private:
    void parseValue(const Json::Value& val, EFieldType type, void *pValue, const MetaFieldBase *desc = nullptr);
private:
	const Json::Value& m_value;
};

} // namespace json
} // namespace serialization
} // namespace metacpp
#endif // JSONDESERIALIZERVISITOR_H
