#pragma once
#include "MetaInfo.h"
#include <vector>

namespace orm
{

class pkMetaObject
{
	friend class pkVisitorBase;
public:
	explicit pkMetaObject(const StructInfoDescriptor *descriptor);
	~pkMetaObject(void);

	inline const char *classname() const { return m_descriptor->m_strucName; }
	inline const StructInfoDescriptor *structDescriptor() const { return m_descriptor; }
	inline const FieldInfoDescriptor *fieldDescriptor(size_t i) const { preparseFields(); return m_fields[i]; }
	inline size_t totalFields() const { preparseFields(); return m_fields.size(); }
private:
	void preparseFields() const;

	const StructInfoDescriptor *m_descriptor;
	mutable bool m_initialized;
	mutable std::vector<const FieldInfoDescriptor *> m_fields;
};

} // namespace pkapi
