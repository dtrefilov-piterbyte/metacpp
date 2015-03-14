#include "SqlTest.h"
#include "SqlStorable.h"
#include "SqlStatement.h"
#include "SqlTransaction.h"
#include <thread>
#ifdef HAVE_SQLITE3
#include "SqliteConnector.h"
#endif
#ifdef HAVE_POSTGRES
#include "PostgresConnector.h"
#endif
#ifdef HAVE_MYSQL
#include "MySqlConnector.h"
#endif

using namespace ::metacpp;
using namespace ::metacpp::db;
using namespace ::metacpp::db::sql;

class City : public Object
{
public:
    int             id;
    String          name;

    META_INFO_DECLARE(City)
};

STRUCT_INFO_BEGIN(City)
    FIELD(City, id)
    FIELD(City, name)
STRUCT_INFO_END(City)

REFLECTIBLE_F(City)

META_INFO(City)

DEFINE_STORABLE(City,
                 PRIMARY_KEY(COL(City::id)),
                 UNIQUE_INDEX(COL(City::id))
                 )

class Person : public Object
{
public:
    int                 id;
    String              name;
    Nullable<int>       age;
    Nullable<double>    cat_weight;
    int                 cityId;
    Nullable<DateTime>  birthday;

    META_INFO_DECLARE(Person)
};

STRUCT_INFO_BEGIN(Person)
    FIELD(Person, id)
    FIELD(Person, name)
    FIELD(Person, age)
    FIELD(Person, cat_weight, 1.0)
    FIELD(Person, cityId)
    FIELD(Person, birthday)
STRUCT_INFO_END(Person)

REFLECTIBLE_F(Person)

META_INFO(Person)

DEFINE_STORABLE(Person,
                 PRIMARY_KEY(COL(Person::id)),
                 REFERENCES(COL(Person::cityId), COL(City::id)),
                 UNIQUE_INDEX(COL(Person::id)),
                 INDEX(COL(Person::cityId)),
                 CHECK(COL(Person::age), COL(Person::age) < 120)    // humans do not live so much
                 )

// User-defined SQL function now() for Postgresql
inline ExpressionNodeFunctionCall<DateTime> db_now()
{
    return ExpressionNodeFunctionCall<DateTime>("now");
}

void SqlTest::SetUp()
{
    m_conn = std::move(connectors::SqlConnectorBase::createConnector(Uri("sqlite3://memdb?mode=memory&cache=shared")));
    //m_conn = std::move(connectors::SqlConnectorBase::createConnector(Uri("postgres://?dbname=alien&hostaddr=127.0.0.1")));
    //m_conn = std::move(connectors::SqlConnectorBase::createConnector(Uri("mysql://localhost/test")));
    ASSERT_TRUE(m_conn.get()) << "Sql connector unavailable";
    m_conn->setConnectionPooling(3);
    connectors::SqlConnectorBase::setDefaultConnector(m_conn.get());
    ASSERT_TRUE(m_conn->connect());
    prepareSchema();
}

void SqlTest::TearDown()
{
    connectors::SqlConnectorBase::setDefaultConnector(nullptr);
    ASSERT_TRUE(m_conn.get());
    EXPECT_TRUE(m_conn->disconnect());
    m_conn.reset();
}

void SqlTest::SetUpTestCase()
{
#ifdef HAVE_SQLITE3
    connectors::SqlConnectorBase::registerConnectorFactory("sqlite3", std::make_shared<connectors::sqlite::SqliteConnectorFactory>());
#endif
#ifdef HAVE_POSTGRES
    connectors::SqlConnectorBase::registerConnectorFactory("postgres", std::make_shared<connectors::postgres::PostgresConnectorFactory>());
#endif
#ifdef HAVE_MYSQL
    connectors::SqlConnectorBase::registerConnectorFactory("mysql", std::make_shared<connectors::mysql::MySqlConnectorFactory>());
#endif
}

void SqlTest::TearDownTestCase()
{
#ifdef HAVE_SQLITE3
    connectors::SqlConnectorBase::unregisterConnectorFactory("sqlite3");
#endif
#ifdef HAVE_POSTGRES
    connectors::SqlConnectorBase::unregisterConnectorFactory("postgres");
#endif
#ifdef HAVE_MYSQL
    connectors::SqlConnectorBase::unregisterConnectorFactory("mysql");
#endif
}

void SqlTest::prepareSchema()
{
    SqlTransaction transaction;
    Storable<City>::createSchema(transaction);
    Storable<Person>::createSchema(transaction);
    transaction.commit();
}

void SqlTest::prepareData()
{
    SqlTransaction transaction;
    Storable<City> city;
    city.init();
    city.name = "Moscow";
    city.insertOne(transaction);
    Storable<Person> person;
    person.name = "Pupkin";
    person.birthday = DateTime::fromString("2004-12-31 00:00:00");
    person.cityId = city.id;
    person.insertOne(transaction);
    transaction.commit();
}

void SqlTest::clearData()
{
    SqlTransaction transaction;
    Storable<City> city;
    Storable<Person> person;
    person.remove().exec(transaction);
    city.remove().exec(transaction);
    transaction.commit();
}

#ifdef HAVE_SQLITE3


TEST(StorableTest, testConstraints)
{
    ASSERT_EQ(Storable<Person>::numConstraints(), 5);
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
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            });
    }

    for (size_t i = 0; i < numThreads; ++i)
        if (threads[i].joinable())
            threads[i].join();
}


TEST_F(SqlTest, selectTest)
{
    prepareData();
    try
    {
        SqlTransaction transaction;
        Storable<Person> person;
        Storable<City> city;
        SqlResultSet resultSet = person.select().innerJoin<City>().where((COL(Person::age).isNull() ||
                (coalesce(COL(Person::age), 0) + 2.5  * COL(Person::cat_weight)) > 250) &&
                COL(Person::cityId) == COL(City::id) &&
                !lower(COL(Person::name)).like("invalid_%") &&
                coalesce(COL(Person::birthday), DateTime::fromString("2000-01-01 00:00:00")) >= DateTime::fromString("2000-01-01 00:00:00"))
                .limit(10).orderAsc(COL(Person::name), COL(City::name)).orderDesc(COL(Person::age)).exec(transaction);

        StringArray persons;
        for (auto it : resultSet)
        {
            (void)it;
            persons.push_back(person.name);
        }

        person.remove().exec(transaction);

        transaction.commit();
    }
    catch (const std::exception& ex)
    {
        throw;
    }
    clearData();
}

TEST_F(SqlTest, updateTest)
{
    prepareData();
    try
    {
        prepareData();
        SqlTransaction transaction;
        Storable<Person> person;
        person.update().ref<City>().set(COL(Person::age) = null, COL(Person::cat_weight) = null)
                .where(COL(Person::cityId) == COL(City::id) && COL(City::name) == String("Moscow") &&
                       (COL(Person::age) == null || COL(Person::cat_weight) == null)).exec(transaction);
        transaction.commit();
    }
    catch (const std::exception& ex)
    {
        throw;
    }
    clearData();
}

TEST_F(SqlTest, insertTest)
{
    try
    {
        SqlTransaction transaction;
        Storable<City> city;
        auto resultSet = city.select().where(COL(City::name) == String("Moscow"))
                .exec(transaction);
        auto it = resultSet.begin();
        if (it != resultSet.end())
        {
            Storable<Person> person;
            person.init();
            person.cityId = city.id;
            person.name = "Pupkin";
            ASSERT_TRUE(person.insertOne(transaction));
            person.name = "Pupkin new";
            ASSERT_TRUE(person.updateOne(transaction));
            ASSERT_TRUE(person.removeOne(transaction));
        }

        transaction.commit();
    }
    catch (const std::exception& ex)
    {
        throw;
    }
}

TEST_F(SqlTest, deleteTest)
{
    try
    {
        SqlTransaction transaction;
        Storable<Person> person;
        person.remove().ref<City>().where(COL(City::id) == COL(Person::cityId) &&
                                          COL(City::name) == String("Moscow") &&
                                          COL(Person::name).like("invalid_%")).exec(transaction);
    }
    catch (const std::exception& ex)
    {
        throw;
    }
}
#endif
