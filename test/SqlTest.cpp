#include "SqlTest.h"
#include "SqlColumnMatcher.h"
#include "SqlColumnAssignment.h"
#include "SqlStorable.h"
#include "SqlStatement.h"
#include "CDebug.h"

using namespace ::metacpp;
using namespace ::metacpp::sql;

class Person : public Object
{
public:
    int             id;
    String          name;
    Nullable<int>   age;
    Nullable<float> cat_weight;
    int             cityId;

    META_INFO_DECLARE(Person)
};

STRUCT_INFO_BEGIN(Person)
    FIELD_INFO(Person, name)
    FIELD_INFO(Person, age)
    FIELD_INFO(Person, cat_weight)
    FIELD_INFO(Person, cityId)
STRUCT_INFO_END(Person)

META_INFO(Person)

typedef STORABLE(Person, id) PersonStorable;

class City : public Object
{
public:
    int             id;
    String          name;

    META_INFO_DECLARE(City)
};

STRUCT_INFO_BEGIN(City)
    FIELD_INFO(City, id)
    FIELD_INFO(City, name, "Moscow")
STRUCT_INFO_END(City)

META_INFO(City)

typedef STORABLE(City, id) CityStorable;

TEST_F(SqlTest, test1)
{
    PersonStorable person;
    person.record()->init();
    person.obj()->age.reset();

    SqlStatementSelect statementSelect(&person);
    cdebug() << statementSelect.innerJoin<City>().where((COLUMN(Person, age).isNull() ||
        (COLUMN(Person, age) + 2.5  * COLUMN(Person, cat_weight)) > 250 ||
        !(COLUMN(Person, name).like("George%") || COLUMN(Person, name) == String("Jack"))) &&
        COLUMN(Person, cityId) == COLUMN(City, id)).limit(10).buildQuery(SqlSyntaxSqlite);

    SqlStatementInsert statementInsert(&person);
    cdebug() << statementInsert.buildQuery(SqlSyntaxSqlite);

    SqlStatementUpdate statementUpdate(&person);
    cdebug() << statementUpdate.from<City>().set(COLUMN(Person, age) = 20, COLUMN(Person, cat_weight) = nullptr)
                .where(COLUMN(Person, cityId) == COLUMN(City, id) && COLUMN(City, name) == String("Moscow"))
                .buildQuery(SqlSyntaxSqlite);

    SqlStatementDelete statementDelete(&person);
    cdebug() << statementDelete.from<City>().where(
                    COLUMN(Person, cityId) == COLUMN(City, id) && COLUMN(City, name) == String("Bobruysk"))
                .buildQuery(SqlSyntaxSqlite);
}
