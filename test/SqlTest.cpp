#include "SqlTest.h"
#include "SqlWhereClause.h"

using namespace ::metacpp;
using namespace ::metacpp::sql;

class Person : public Object
{
public:
    int             id;
    String          name;
    Nullable<int>   age;
    int             cityId;

    META_INFO_DECLARE(Person)
};

STRUCT_INFO_BEGIN(Person)
    FIELD_INFO(Person, name)
    FIELD_INFO(Person, age)
    FIELD_INFO(Person, cityId)
STRUCT_INFO_END(Person)

META_INFO(Person)

class City : public Object
{
public:
    int             id;
    String          name;

    META_INFO_DECLARE(City)
};

STRUCT_INFO_BEGIN(City)
    FIELD_INFO(City, id)
    FIELD_INFO(City, name)
STRUCT_INFO_END(City)

META_INFO(City)

TEST_F(SqlTest, test1)
{
    std::ostringstream ss;
    (COLUMN(Person, age).isNull() ||
            !(COLUMN(Person, name).like("George%") || COLUMN(Person, name) == "Jack") &&
            COLUMN(Person, cityId) == COLUMN(City, id)).
            printExpression(ss);
    cdebug() << ss.str();
    ASSERT_EQ(String(getMetaField(&Person::age)->name()), "age");
}
