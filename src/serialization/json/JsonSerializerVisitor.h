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
#ifndef JSONSERIALIZERVISITOR_H
#define JSONSERIALIZERVISITOR_H
#include "config.h"
#include "VisitorBase.h"
#include "MetaInfo.h"
#include <json/json.h>

namespace metacpp
{
class Object;

namespace serialization
{
namespace json
{

/** \brief Visitor for json serialization of objects */
class JsonSerializerVisitor :
    public VisitorBase
{
public:
    /** \brief Constructs a new instance of JsonSerializerVisitor */
    JsonSerializerVisitor(void);
    ~JsonSerializerVisitor(void);

    /** \brief Gets a value containing introspecting object */
    const Json::Value& rootValue() const;
protected:
    /** \brief Overrides VisitorBase::previsitStruct */
    void previsitStruct(Object *obj) override;
    /** \brief Overrides VisitorBase::visitField */
    void visitField(Object *obj, const MetaFieldBase *field) override;
private:
    void appendSubValue(Json::Value& val, EFieldType type, const void *pValue, const MetaFieldBase *field = nullptr);
private:
    Json::Value m_value;
};

} // namespace json
} // namespace serialization
} // namespace metacpp
#endif // JSONSERIALIZERVISITOR_H
