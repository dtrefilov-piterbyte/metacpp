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
#include "config.h"
//#include "Object.h"

namespace metacpp
{

class Object;
class MetaFieldBase;

/** \brief Base class for visitors used for object introspection */
class VisitorBase
{
public:
    /** \brief Constructs a new instance of the VisitorBase */
    VisitorBase();
    virtual ~VisitorBase(void);
    /** \brief Introspects object using this visitor */
    void visit(Object *object);
protected:
    /** \brief Method called before starting introspection */
    virtual void previsitStruct(Object *obj);
    /** \brief Method called on each property of the object during introspection */
    virtual void visitField(Object *obj, const MetaFieldBase *) = 0;
    /** \brief Method called at the end of introspection */
    virtual void postvisitStruct(Object *obj);
};

} // namespace metacpp
#endif // VISITOR_BASE_H
