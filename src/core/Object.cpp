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
#include "Object.h"
#include "InitVisitor.h"

#ifdef HAVE_JSONCPP
#include "JsonSerializerVisitor.h"
#include "JsonDeserializerVisitor.h"
#endif

#ifdef HAVE_MONGODB
#include "BsonSerializerVisitor.h"
#endif

namespace metacpp
{

Object::~Object()
{
}

void Object::init()
{
    pkInitVisitor vis;
	vis.visit(this);
}

#ifdef HAVE_JSONCPP
String Object::toJson(bool prettyFormatted) const
{
    using namespace serialization::json;
    JsonSerializerVisitor vis;
    vis.visit(const_cast<Object *>(this));
	if (prettyFormatted)
	{
		Json::StyledWriter writer;
        return String(writer.write(vis.rootValue()).c_str());
	}
	else
	{
        Json::FastWriter writer;
        return String(writer.write(vis.rootValue()).c_str());
    }
}

void Object::fromJson(const String& s)
{
    using namespace serialization::json;
	Json::Reader reader;
	Json::Value root;
    if (!reader.parse(s.begin(), s.end(), root, false))
		throw std::invalid_argument("Json::Reader::parse failed");
    JsonDeserializerVisitor vis(root);
    vis.visit(this);
}
#endif

Variant Object::invoke(const String &methodName, const VariantArray &args)
{
    return doInvoke(methodName, args, false);
}

Variant Object::invoke(const String &methodName, const VariantArray &args) const
{
    return doInvoke(methodName, args, true);
}

void Object::setProperty(const String &propName, const Variant &val)
{
    auto field = metaObject()->fieldByName(propName);
    if (field)
        field->setValue(val, this);
    else
        m_dynamicProperties[propName] = val;
}

Variant Object::getProperty(const String &propName) const
{
    auto field = metaObject()->fieldByName(propName);
    if (field)
        return field->getValue(this);
    auto it = m_dynamicProperties.find(propName);
    if (it != m_dynamicProperties.end())
        return it->second;
    return Variant();
}

const MetaObject *Object::staticMetaObject()
{
    return &ms_metaObject;
}

Variant Object::doInvoke(const String &methodName, const VariantArray &args, bool constness) const
{
    for (size_t i = 0; i < metaObject()->totalMethods(); ++i)
    {
        auto method = metaObject()->method(i);
        if (eMethodOwn == method->type() && args.size() == method->numArguments() && methodName.equals(method->name()))
        {
            // cannot call non-const methods on const objects
            if (constness && !method->constness())
                continue;
            try
            {
                return method->invoker()->invoke(const_cast<Object *>(this), args);
            }
            catch (const BindArgumentException& /*ex*/)
            {
                continue;
            }
        }
    }
    throw MethodNotFoundException(String("Cannot find own method " + methodName + " compatible with arguments provided").c_str());
}


STRUCT_INFO_BEGIN(Object)
STRUCT_INFO_END(Object)

REFLECTIBLE_F(Object)

const MetaObject Object::ms_metaObject(&REFLECTIBLE_DESCRIPTOR(Object));

} // namespace metacpp
