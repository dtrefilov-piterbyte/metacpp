#include "SqlTest.h"
#include "SqlColumnMatcher.h"
#include "SqlColumnAssignment.h"
#include "SqlStorable.h"
#include "SqlStatement.h"
#include "SqlTransaction.h"
#include "CDebug.h"
#include <thread>

using namespace ::metacpp;
using namespace ::metacpp::sql;

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

DECLARE_STORABLE(City,
                 PRIMARY_KEY(COL(City, id)),
                 UNIQUE_INDEX(COL(City, id))
                 )

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

DECLARE_STORABLE(Person,
                 PRIMARY_KEY(COL(Person, id)),
                 REFERENCES(COL(Person, cityId), COL(City, id)),
                 UNIQUE_INDEX(COL(Person, id)),
                 INDEX(COL(Person, cityId)),
                 CHECK(COL(Person, age), COL(Person, age) < 120)    // people do not live so much
                 )



TEST(StorableTest, testConstraints)
{
    ASSERT_EQ(Storable<Person>::numConstraints(), 5);
}

void SqlTest::SetUp()
{
    m_conn = new connectors::sqlite::SqliteConnector(true ? "test.sqlite" : "file:sqltest?mode=memory&cache=shared");
    connectors::SqlConnectorBase::setDefaultConnector(m_conn);
    ASSERT_TRUE(m_conn->connect());
    prepareSchema();
}

void SqlTest::TearDown()
{
    connectors::SqlConnectorBase::setDefaultConnector(nullptr);
    delete m_conn;
}

TEST_F(SqlTest, transactionCommitTest)
{
    SqlTransaction transaction;
    ASSERT_NO_THROW(transaction.commit());
}

TEST_F(SqlTest, transactionRollbackTest)
{
    SqlTransaction transaction;
    ASSERT_NO_THROW(transaction.rollback());
}

TEST_F(SqlTest, transactionManualBegin)
{
    SqlTransaction transaction(SqlTransactionAutoCloseManual);
    ASSERT_NO_THROW(transaction.begin());
    ASSERT_ANY_THROW(transaction.begin());
    ASSERT_NO_THROW(transaction.commit());
    ASSERT_ANY_THROW(transaction.commit());
}

TEST_F(SqlTest, multipleTransactionsTest)
{
    const size_t numThreads = 10;
    std::thread threads[numThreads];
    for (size_t i = 0; i < numThreads; ++i)
    {
        threads[i] = std::thread([]()
            {
                SqlTransaction transaction;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            });
    }

    for (size_t i = 0; i < numThreads; ++i)
        if (threads[i].joinable())
            threads[i].join();
}


TEST_F(SqlTest, selectTest)
{
    try
    {
        SqlTransaction transaction;
        Storable<Person> person;
        SqlResultSet resultSet = person.select().innerJoin<City>().where((COL(Person, age).isNull() ||
                                                 (COL(Person, age) + 2.5  * COL(Person, cat_weight)) > 250 ||
                                                 !(COL(Person, name).like("George%") || COL(Person, name) == String("Jack"))) &&
                                                COL(Person, cityId) == COL(City, id)).limit(10).exec(transaction);

        StringArray persons;
        for (auto it : resultSet)
        {
            (void)it;
            cdebug() << person.name;
            persons.push_back(person.name);
        }
        transaction.commit();
    }
    catch (const std::exception& ex)
    {
        throw;
    }
}

TEST_F(SqlTest, updateTest)
{
    try
    {
        SqlTransaction transaction;
        Storable<Person> person;
        person.update().ref<City>().set(COL(Person, age) = 20, COL(Person, cat_weight) = nullptr)
                .where(COL(Person, cityId) == COL(City, id) && COL(City, name) == String("Moscow")).exec(transaction);
        transaction.commit();
    }
    catch (const std::exception& ex)
    {
        throw;
    }
}

TEST_F(SqlTest, insertTest)
{
    try
    {
        SqlTransaction transaction;
        Storable<City> city;
        auto resultSet = city.select().where(COL(City, name) == String("Moscow"))
                .exec(transaction);
        auto it = resultSet.begin();
        if (it != resultSet.end())
        {
            Storable<Person> person;
            person.init();
            person.id = 0;
            person.age.reset();
            person.cat_weight.reset();
            person.cityId = city.id;
            person.name = "Pupkin";
            ASSERT_EQ(1, person.insertOne(transaction));
            person.name = "Pupkin new";
            ASSERT_EQ(1, person.updateOne(transaction));
            ASSERT_EQ(1, person.removeOne(transaction));
        }

        transaction.commit();
    }
    catch (const std::exception& ex)
    {
        throw;
    }
}

void SqlTest::prepareSchema()
{
    SqlTransaction transaction;
    Storable<City>::createSchema(transaction);
    Storable<Person>::createSchema(transaction);
    transaction.commit();
}
