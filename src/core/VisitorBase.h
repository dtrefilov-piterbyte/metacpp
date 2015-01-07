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
#ifndef VISITOR_BASE_H
#define VISITOR_BASE_H
#include "Object.h"

namespace metacpp
{

class VisitorBase
{
public:
	VisitorBase();
	virtual ~VisitorBase(void);
	void visit(Object *obj);
protected:
    virtual void previsitStruct(Object *obj);
    virtual void visitField(Object *obj, const MetaField *);
    virtual void postvisitStruct(Object *obj);
};

} // namespace metacpp
#endif // VISITOR_BASE_H
