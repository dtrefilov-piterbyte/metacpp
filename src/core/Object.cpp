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
#include "JsonSerializerVisitor.h"
#include "JsonDeserializerVisitor.h"

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

String Object::toString(bool prettyFormatted) const
{
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

void Object::fromString(const String& s)
{
	Json::Reader reader;
	Json::Value root;
    if (!reader.parse(s.begin(), s.end(), root, false))
		throw std::invalid_argument("Json::Reader::parse failed");
    JsonDeserializerVisitor vis(root);
	vis.visit(this);
}

STRUCT_INFO_BEGIN(Object)
STRUCT_INFO_END(Object)

} // namespace metacpp
