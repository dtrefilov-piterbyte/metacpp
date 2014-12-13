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
	void visitField(Object *obj, const FieldInfoDescriptor *desc) override;
private:
	void ParseValue(const Json::Value& parent, EFieldType type, void *pValue, const FieldInfoDescriptor *desc = nullptr, Json::ArrayIndex i = 0);
private:
	const Json::Value& m_value;
};

} // namespace metacpp
#endif // JSONDESERIALIZERVISITOR_H
