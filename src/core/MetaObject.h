#ifndef METAOBJECT_H
#define METAOBJECT_H
#include "MetaInfo.h"
#include <vector>

namespace metacpp
{

class pkMetaObject
{
	friend class pkVisitorBase;
public:
	explicit pkMetaObject(const StructInfoDescriptor *descriptor);
	~pkMetaObject(void);

    inline const char *className() const { return m_descriptor->m_strucName; }
	inline const StructInfoDescriptor *structDescriptor() const { return m_descriptor; }
	inline const FieldInfoDescriptor *fieldDescriptor(size_t i) const { preparseFields(); return m_fields[i]; }
	inline size_t totalFields() const { preparseFields(); return m_fields.size(); }
private:
	void preparseFields() const;

	const StructInfoDescriptor *m_descriptor;
	mutable bool m_initialized;
	mutable std::vector<const FieldInfoDescriptor *> m_fields;
};

} // namespace metacpp
#endif // METAOBJECT_H
