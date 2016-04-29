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
#include "VisitorBase.h"
#include "Object.h"

namespace metacpp
{

VisitorBase::VisitorBase()
{
}

VisitorBase::~VisitorBase(void)
{
}

void VisitorBase::previsitStruct(Object *obj)
{
    (void)obj;
}

void VisitorBase::postvisitStruct(Object *obj)
{
    (void)obj;
}

void VisitorBase::visit(Object *obj)
{
    previsitStruct(obj);
    for (size_t i = 0; i < obj->metaObject()->totalFields(); ++i)
        visitField(obj, obj->metaObject()->field(i));
    postvisitStruct(obj);
}

} // namespace metacpp
