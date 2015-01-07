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
#include <json/json.h>

namespace metacpp
{

class JsonDeserializerVisitor :
	public VisitorBase
{
public:
    JsonDeserializerVisitor(const Json::Value& val);
    ~JsonDeserializerVisitor(void);
protected:
    void visitField(Object *obj, const MetaField *desc) override;
private:
    void ParseValue(const Json::Value& parent, EFieldType type, void *pValue, const MetaField *desc = nullptr, Json::ArrayIndex i = 0);
private:
	const Json::Value& m_value;
};

} // namespace metacpp
#endif // JSONDESERIALIZERVISITOR_H
