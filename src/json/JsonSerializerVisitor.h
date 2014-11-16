#pragma once
#include "VisitorBase.h"
#include <json/json.h>

namespace orm
{

class JsonSerializerVisitor :
    public VisitorBase
{
    friend class Object;
public:
    JsonSerializerVisitor(void);
    ~JsonSerializerVisitor(void);

protected:
    void visitField(Object *obj, const FieldInfoDescriptor *desc) override;
private:
	void appendSubValue(Json::Value& parent, EFieldType type, const void *pValue, const FieldInfoDescriptor *desc = nullptr, Json::ArrayIndex = 0);
	const Json::Value& rootValue() const;
private:
	Json::Value m_value;
};

} // namespace pkapi
