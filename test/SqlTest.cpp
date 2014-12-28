#include "SqlTest.h"
#include "SqlWhereClause.h"

using namespace ::metacpp;
using namespace ::metacpp::sql;

class Person : public Object
{
public:
    String          name;
    Nullable<int>   age;

    META_INFO_DECLARE(Person)
};

STRUCT_INFO_BEGIN(Person)
    FIELD_INFO(Person, name)
    FIELD_INFO(Person, age)
STRUCT_INFO_END(Person)

META_INFO(Person)

TEST_F(SqlTest, test1)
{
    std::ostringstream ss;
    (COLUMN(Person, age).isNull() &&
            (COLUMN(Person, name) == "George" || COLUMN(Person, name) == "Jack")).
            buildStatement(ss);
    cdebug() << ss.str();
    ASSERT_EQ(String(getMetaField(&Person::age)->name()), "age");
}
