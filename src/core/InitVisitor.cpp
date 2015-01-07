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
#include "InitVisitor.h"

namespace metacpp
{

pkInitVisitor::pkInitVisitor()
{
}


pkInitVisitor::~pkInitVisitor(void)
{
}

void pkInitVisitor::visitField(Object *obj, const MetaField *field)
{
    if (field->nullable())
    {
        switch (field->type())
        {
        default:
        case eFieldVoid:
            throw std::invalid_argument(std::string("Unknown field type: ") + (char)field->type());
        case eFieldBool:
            if (eOptional == field->mandatoriness())
                field->access<Nullable<bool> >(obj).reset();
            else
                field->access<Nullable<bool> >(obj) =
                        reinterpret_cast<const MetaFieldBool *>(field)->defaultValue();
            break;
        case eFieldInt:
            if (eOptional == field->mandatoriness())
                field->access<Nullable<int32_t> >(obj).reset();
            else
                field->access<Nullable<int32_t> >(obj) =
                        reinterpret_cast<const MetaFieldInt *>(field)->defaultValue();
            break;
        case eFieldUint:
            if (eOptional == field->mandatoriness())
                field->access<Nullable<uint32_t> >(obj).reset();
            else
                field->access<Nullable<uint32_t> >(obj) =
                        reinterpret_cast<const MetaFieldUint *>(field)->defaultValue();
            break;
        case eFieldInt64:
            if (eOptional == field->mandatoriness())
                field->access<Nullable<int64_t> >(obj).reset();
            else
                field->access<Nullable<int64_t> >(obj) =
                        reinterpret_cast<const MetaFieldInt64 *>(field)->defaultValue();
            break;
        case eFieldUint64:
            if (eOptional == field->mandatoriness())
                field->access<Nullable<uint64_t> >(obj).reset();
            else
                field->access<Nullable<uint64_t> >(obj) =
                        reinterpret_cast<const MetaFieldUint64 *>(field)->defaultValue();
            break;
        case eFieldEnum:
            if (eOptional == field->mandatoriness())
                field->access<Nullable<uint32_t> >(obj).reset();
            else
                field->access<Nullable<uint32_t> >(obj) =
                        reinterpret_cast<const MetaFieldEnum *>(field)->defaultValue();
            break;
        case eFieldFloat:
            if (eOptional == field->mandatoriness())
                field->access<Nullable<float> >(obj).reset();
            else
                field->access<Nullable<float> >(obj) =
                        reinterpret_cast<const MetaFieldFloat *>(field)->defaultValue();
            break;
        case eFieldDouble:
            if (eOptional == field->mandatoriness())
                field->access<Nullable<double> >(obj).reset();
            else
                field->access<Nullable<double> >(obj) = reinterpret_cast<const MetaFieldDouble *>(field)->defaultValue();
        case eFieldDateTime:
            if (eOptional == field->mandatoriness())
                field->access<Nullable<DateTime> >(obj).reset();
            else
                field->access<Nullable<DateTime> >(obj) = DateTime(0);
            break;
        case eFieldString:
            if (eOptional == field->mandatoriness())
                field->access<Nullable<String> >(obj).reset();
            else
                field->access<Nullable<String> >(obj) = reinterpret_cast<const MetaFieldString *>(field)->defaultValue();
        }
        return;
    }

    switch (field->type())
	{
	default:
	case eFieldVoid:
        throw std::invalid_argument(std::string("Unknown field type: ") + (char)field->type());
    case eFieldBool:
        field->access<bool>(obj) = reinterpret_cast<const MetaFieldBool *>(field)->defaultValue();
		break;
    case eFieldInt:
        field->access<int32_t>(obj) = reinterpret_cast<const MetaFieldInt *>(field)->defaultValue();
		break;
	case eFieldUint:
        field->access<uint32_t>(obj) = reinterpret_cast<const MetaFieldUint *>(field)->defaultValue();
		break;
    case eFieldInt64:
        field->access<int64_t>(obj) = reinterpret_cast<const MetaFieldInt64 *>(field)->defaultValue();
        break;
    case eFieldUint64:
        field->access<uint64_t>(obj) = reinterpret_cast<const MetaFieldUint64 *>(field)->defaultValue();
        break;
    case eFieldFloat:
        field->access<float>(obj) = reinterpret_cast<const MetaFieldFloat *>(field)->defaultValue();
		break;
    case eFieldDouble:
        field->access<double>(obj) = reinterpret_cast<const MetaFieldDouble *>(field)->defaultValue();
        break;
    case eFieldString:
        field->access<metacpp::String>(obj) = reinterpret_cast<const MetaFieldString *>(field)->defaultValue();
		break;
    case eFieldEnum:
        field->access<uint32_t>(obj) = reinterpret_cast<const MetaFieldEnum *>(field)->defaultValue();
		break;
    case eFieldArray:
        field->access<metacpp::Array<char> >(obj).clear();
		break;
    case eFieldObject: {
        field->access<Object>(obj).init();
		break;
    case eFieldDateTime:
        field->access<DateTime>(obj) = DateTime(0);
        break;
    }
    }
}

} // namespace metacpp
