#include "SqlTest.h"
#include "SqlColumnMatcher.h"
#include "SqlColumnAssignment.h"
#include "SqlStorable.h"
#include "SqlStatement.h"
#include "SqlTransaction.h"
#include "CDebug.h"

using namespace ::metacpp;
using namespace ::metacpp::sql;

class Person : public Object
{
public:
    int             id;
    String          name;
    Nullable<int>   age;
    Nullable<double> cat_weight;
    int             cityId;

    META_INFO_DECLARE(Person)
};

STRUCT_INFO_BEGIN(Person)
    FIELD_INFO(Person, id)
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

//TEST_F(SqlTest, test1)
//{
//    PersonStorable person;
//    person.record()->init();
//    person.obj()->age.reset();
//
//    SqlStatementSelect statementSelect(&person);
//    cdebug() << statementSelect.innerJoin<City>().where((COLUMN(Person, age).isNull() ||
//        (COLUMN(Person, age) + 2.5  * COLUMN(Person, cat_weight)) > 250 ||
//        !(COLUMN(Person, name).like("George%") || COLUMN(Person, name) == String("Jack"))) &&
//        COLUMN(Person, cityId) == COLUMN(City, id)).limit(10).buildQuery(SqlSyntaxSqlite);
//
//    SqlStatementInsert statementInsert(&person);
//    cdebug() << statementInsert.buildQuery(SqlSyntaxSqlite);
//
//    SqlStatementUpdate statementUpdate(&person);
//    cdebug() << statementUpdate.ref<City>().set(COLUMN(Person, age) = 20, COLUMN(Person, cat_weight) = nullptr)
//                .where(COLUMN(Person, cityId) == COLUMN(City, id) && COLUMN(City, name) == String("Moscow"))
//                .buildQuery(SqlSyntaxSqlite);
//
//    SqlStatementDelete statementDelete(&person);
//    cdebug() << statementDelete.ref<City>().where(
//                    COLUMN(Person, cityId) == COLUMN(City, id) && COLUMN(City, name) == String("Bobruysk"))
//                .buildQuery(SqlSyntaxSqlite);
//}

void SqlTest::SetUp()
{
    m_conn = new connectors::sqlite::SqliteConnector("test.sqlite");
    connectors::SqlConnectorBase::setDefaultConnector(m_conn);
    ASSERT_TRUE(m_conn->connect());
}

void SqlTest::TearDown()
{
    connectors::SqlConnectorBase::setDefaultConnector(nullptr);
    delete m_conn;
}

void SqlTest::transactionsTest()
{
    {
        SqlTransaction transaction;
        ASSERT_NO_THROW(transaction.commit());
    }
    {
        SqlTransaction transaction;
        ASSERT_NO_THROW(transaction.rollback());
    }
}

//TEST_F(SqlTest, transactionsTest)
//{
//    transactionsTest();
//}

void SqlTest::selectTest()
{
    try
    {
        SqlTransaction transaction;
        PersonStorable person;
        SqlResultSet resultSet = person.select().innerJoin<City>().where((COLUMN(Person, age).isNull() ||
                                                 (COLUMN(Person, age) + 2.5  * COLUMN(Person, cat_weight)) > 250 ||
                                                 !(COLUMN(Person, name).like("George%") || COLUMN(Person, name) == String("Jack"))) &&
                                                COLUMN(Person, cityId) == COLUMN(City, id)).limit(10).exec(transaction);

        StringArray persons;
        for (auto it : resultSet)
        {
            persons.push_back(person.obj()->name);
        }
        transaction.commit();
    }
    catch (const std::exception& ex)
    {
        throw;
    }
}

TEST_F(SqlTest, selectTest)
{
    selectTest();
}

void SqlTest::updateTest()
{
    try
    {
        SqlTransaction transaction;
        PersonStorable person;
        int nRows = person.update().ref<City>().set(COLUMN(Person, age) = 20, COLUMN(Person, cat_weight) = nullptr)
                .where(COLUMN(Person, cityId) == COLUMN(City, id) && COLUMN(City, name) == String("Moscow")).exec(transaction);
        transaction.commit();
    }
    catch (const std::exception& ex)
    {
        throw;
    }
}

TEST_F(SqlTest, updateTest)
{
    updateTest();
}

void SqlTest::insertTest()
{
    try
    {
        SqlTransaction transaction;
        CityStorable city;
        auto resultSet = city.select().where(COLUMN(City, name) == String("Moscow"))
                .exec(transaction);
        auto it = resultSet.begin();
        if (it != resultSet.end())
        {
            PersonStorable person;
            person.obj()->init();
            person.obj()->id = 0;
            person.obj()->age.reset();
            person.obj()->cat_weight.reset();
            person.obj()->cityId = city.obj()->id;
            person.obj()->name = "Pupkin";
            person.insertOne(transaction);
        }

        transaction.commit();
    }
    catch (const std::exception& ex)
    {
        throw;
    }
}

TEST_F(SqlTest, insertTest)
{
    insertTest();
}

