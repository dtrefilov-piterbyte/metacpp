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
    int                 id;
    String              name;
    Nullable<String>    country;

    META_INFO_DECLARE(City)
};

STRUCT_INFO_BEGIN(City)
    FIELD(City, id)
    FIELD(City, name)
    FIELD(City, country)
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
    city.country = "Russia";
    city.insertOne(transaction);

    Storable<Person> person;
    person.name = "Pupkin";
    person.cityId = city.id;
    person.insertOne(transaction);

    person.name = "Lenin";
    person.cat_weight = 2.0;
    person.age = 53;
    person.insertOne(transaction);

    city.name = "Ibadan";
    city.country = nullptr;
    city.insertOne(transaction);

    person.name = "Smith";
    person.birthday = DateTime(1960, April, 4);
    person.age = 55;
    person.cat_weight = 1.0;
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


TEST_F(SqlTest, testConstraints)
{
    ASSERT_EQ(Storable<Person>::numConstraints(), 5);

    auto constraintPkey = Storable<Person>::getConstraint(0);
    auto constraintRef = Storable<Person>::getConstraint(1);
    auto constraintUnique = Storable<Person>::getConstraint(2);
    auto constraintIndex = Storable<Person>::getConstraint(3);
    auto constraintCheck = Storable<Person>::getConstraint(4);
    ASSERT_EQ(constraintPkey->type(), SqlConstraintTypePrimaryKey);
    ASSERT_EQ(constraintPkey->metaField()->name(), String("id"));
    ASSERT_EQ(constraintPkey->metaObject(), Person::staticMetaObject());

    ASSERT_EQ(constraintRef->type(), SqlConstraintTypeForeignKey);
    ASSERT_EQ(constraintRef->metaField()->name(), String("cityId"));
    ASSERT_EQ(std::dynamic_pointer_cast<SqlConstraintForeignKey>(constraintRef)
              ->referenceMetaObject(), City::staticMetaObject());
    ASSERT_EQ(std::dynamic_pointer_cast<SqlConstraintForeignKey>(constraintRef)
              ->referenceMetaField()->name(), String("id"));

    ASSERT_EQ(constraintUnique->type(), SqlConstraintTypeIndex);
    ASSERT_EQ(constraintUnique->metaObject(), Person::staticMetaObject());
    ASSERT_EQ(constraintUnique->metaField()->name(), String("id"));
    ASSERT_EQ(std::dynamic_pointer_cast<SqlConstraintIndex>(constraintUnique)
              ->unique(), true);

    ASSERT_EQ(constraintIndex->type(), SqlConstraintTypeIndex);
    ASSERT_EQ(constraintIndex->metaObject(), Person::staticMetaObject());
    ASSERT_EQ(constraintIndex->metaField()->name(), String("cityId"));
    ASSERT_EQ(std::dynamic_pointer_cast<SqlConstraintIndex>(constraintIndex)
              ->unique(), false);

    ASSERT_EQ(constraintCheck->type(), SqlConstraintTypeCheck);
    ASSERT_EQ(constraintCheck->metaObject(), Person::staticMetaObject());
    ASSERT_EQ(constraintCheck->metaField()->name(), String("age"));
    ASSERT_EQ(std::dynamic_pointer_cast<SqlConstraintCheck>(constraintCheck)
              ->checkExpression(), String("age < 120"));

}

void SqliteTest::SetUp()
{
    m_conn = std::move(connectors::SqlConnectorBase::createConnector(Uri("sqlite3://:memory:")));
    //m_conn = std::move(connectors::SqlConnectorBase::createConnector(Uri("postgres://?dbname=alien&hostaddr=127.0.0.1")));
    //m_conn = std::move(connectors::SqlConnectorBase::createConnector(Uri("mysql://localhost/test")));
    ASSERT_TRUE(static_cast<bool>(m_conn)) << "Sql connector unavailable";
    connectors::SqlConnectorBase::setDefaultConnector(m_conn.get());
    ASSERT_TRUE(m_conn->connect());
    prepareSchema();

    prepareData();
}

void SqliteTest::TearDown()
{
    clearData();

    connectors::SqlConnectorBase::setDefaultConnector(nullptr);
    ASSERT_TRUE(static_cast<bool>(m_conn));
    EXPECT_TRUE(m_conn->disconnect());
    m_conn.reset();
}

void SqliteTest::SetUpTestCase()
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

void SqliteTest::TearDownTestCase()
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

#ifdef HAVE_SQLITE3

TEST_F(SqliteTest, transactionCommitTest)
{
    SqlTransaction transaction;
    ASSERT_NO_THROW(transaction.commit());
}

TEST_F(SqliteTest, transactionRollbackTest)
{
    SqlTransaction transaction;
    ASSERT_NO_THROW(transaction.rollback());
}

TEST_F(SqliteTest, transactionManualBegin)
{
    SqlTransaction transaction(SqlTransactionAutoCloseManual);
    ASSERT_NO_THROW(transaction.begin());
    ASSERT_ANY_THROW(transaction.begin());
    ASSERT_NO_THROW(transaction.commit());
    ASSERT_ANY_THROW(transaction.commit());
}

TEST_F(SqliteTest, multipleTransactionsTest)
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

static bool HasPerson(const Array<Person>& persons, const String& name)
{
    auto pupkin = std::find_if(persons.begin(), persons.end(), [name](const Person& p) { return p.name == name; });
    if (pupkin == persons.end())
        return false;

    return true;
}

static bool HasPupkin(const Array<Person>& persons) { return HasPerson(persons, "Pupkin"); }
static bool HasSmith(const Array<Person>& persons) { return HasPerson(persons, "Smith"); }
static bool HasLenin(const Array<Person>& persons) { return HasPerson(persons, "Lenin"); }

TEST_F(SqliteTest, simpleSelectTest)
{
    SqlTransaction transaction;

    auto cities = Storable<City>::fetchAll(transaction);
    auto persons = Storable<Person>::fetchAll(transaction);


    ASSERT_EQ(cities.size(), 2);
    auto moscow = std::find_if(cities.begin(), cities.end(), [](const City& c) { return c.name == "Moscow"; });
    ASSERT_NE(moscow, cities.end());
    auto ibadan = std::find_if(cities.begin(), cities.end(), [](const City& c) { return c.name == "Ibadan"; });
    ASSERT_NE(ibadan, cities.end());

    EXPECT_EQ(moscow->name, "Moscow");
    EXPECT_EQ(moscow->country, String("Russia"));

    EXPECT_EQ(ibadan->name, "Ibadan");
    EXPECT_EQ(ibadan->country, nullptr);

    auto pupkin = std::find_if(persons.begin(), persons.end(), [](const Person& p) { return p.name == "Pupkin"; });
    auto smith = std::find_if(persons.begin(), persons.end(), [](const Person& p) { return p.name == "Smith"; });
    auto lenin = std::find_if(persons.begin(), persons.end(), [](const Person& p) { return p.name == "Lenin"; });

    ASSERT_EQ(persons.size(), 3);
    ASSERT_TRUE(HasPupkin(persons));
    ASSERT_TRUE(HasSmith(persons));
    ASSERT_TRUE(HasLenin(persons));

    EXPECT_EQ(pupkin->cityId, moscow->id);
    EXPECT_EQ(pupkin->name, "Pupkin");
    EXPECT_EQ(pupkin->birthday, nullptr);
    EXPECT_EQ(pupkin->age, nullptr);
    EXPECT_EQ(pupkin->cat_weight, nullptr);

    EXPECT_EQ(smith->cityId, ibadan->id);
    EXPECT_EQ(smith->name, "Smith");
    EXPECT_EQ(smith->birthday, DateTime(1960, April, 4));
    EXPECT_EQ(smith->age, 55);
    EXPECT_EQ(smith->cat_weight, 1.0);

    EXPECT_EQ(lenin->cityId, moscow->id);
    EXPECT_EQ(lenin->name, "Lenin");
    EXPECT_EQ(lenin->birthday, nullptr);
    EXPECT_EQ(lenin->age, 53);
    EXPECT_EQ(lenin->cat_weight, 2.0);
}

TEST_F(SqliteTest, testInnerJoin)
{
    SqlTransaction transaction;
    Storable<Person> person;
    Array<Person> persons;

    for (auto row : person.select().innerJoin<City>().where(COL(Person::cityId) == COL(City::id) &&
                                                            COL(City::name) == String("Moscow"))
         .exec(transaction))
    {
        EXPECT_EQ(row, persons.size());
        persons.push_back(person);
    }

    ASSERT_EQ(persons.size(), 2);
    EXPECT_TRUE(HasPupkin(persons));
    EXPECT_TRUE(HasLenin(persons));
}

TEST_F(SqliteTest, testOuterJoin)
{
    SqlTransaction transaction;
    Storable<Person> person;
    Array<Person> persons;

    for (auto row : person.select().outerJoin<City>().where(COL(Person::cityId) == COL(City::id) &&
                                                            COL(City::name) == String("Moscow"))
         .exec(transaction))
    {
        EXPECT_EQ(row, persons.size());
        persons.push_back(person);
    }

    ASSERT_EQ(persons.size(), 3);
    EXPECT_TRUE(HasPupkin(persons));
    EXPECT_TRUE(HasSmith(persons));
    EXPECT_TRUE(HasLenin(persons));
}

TEST_F(SqliteTest, testIsNullOperator)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, COL(Person::age).isNull());

    ASSERT_EQ(persons.size(), 1);
    EXPECT_TRUE(HasPupkin(persons));
}

TEST_F(SqliteTest, testIsNotNullOperator)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, COL(Person::birthday).isNotNull());

    ASSERT_EQ(persons.size(), 1);
    EXPECT_TRUE(HasSmith(persons));
}

TEST_F(SqliteTest, testEqualOperator)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, COL(Person::name) == String("Smith"));

    ASSERT_EQ(persons.size(), 1);
    EXPECT_TRUE(HasSmith(persons));
}

TEST_F(SqliteTest, testNotEqualOperator)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, COL(Person::name) != String("Smith"));

    ASSERT_EQ(persons.size(), 2);
    EXPECT_TRUE(HasPupkin(persons));
    EXPECT_TRUE(HasLenin(persons));
}

TEST_F(SqliteTest, testLessOperator)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, COL(Person::age) < 55);

    ASSERT_EQ(persons.size(), 1);
    EXPECT_TRUE(HasLenin(persons));
}

TEST_F(SqliteTest, testLessOrEqualOperator)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, COL(Person::age) <= 55);

    ASSERT_EQ(persons.size(), 2);
    EXPECT_TRUE(HasLenin(persons));
    EXPECT_TRUE(HasSmith(persons));
}

TEST_F(SqliteTest, testGreaterOperator)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, COL(Person::age) > 53);

    ASSERT_EQ(persons.size(), 1);
    EXPECT_TRUE(HasSmith(persons));
}

TEST_F(SqliteTest, testGreaterOrEqualOperator)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, COL(Person::age) >= 53);

    ASSERT_EQ(persons.size(), 2);
    EXPECT_TRUE(HasLenin(persons));
    EXPECT_TRUE(HasSmith(persons));
}

TEST_F(SqliteTest, testLikeOperator)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, COL(Person::name).like("%n"));

    ASSERT_EQ(persons.size(), 2);
    EXPECT_TRUE(HasLenin(persons));
    EXPECT_TRUE(HasPupkin(persons));
}

TEST_F(SqliteTest, testPlusOperator)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, COL(Person::age) + 2 == 55);

    ASSERT_EQ(persons.size(), 1);
    EXPECT_TRUE(HasLenin(persons));
}

TEST_F(SqliteTest, testConcatOperator)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction,
        (COL(Person::name) + " (Ulyanov)") == String("Lenin (Ulyanov)"));

    ASSERT_EQ(persons.size(), 1);
    EXPECT_TRUE(HasLenin(persons));
}

TEST_F(SqliteTest, testMinusOperator)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, COL(Person::age) - 2 == 53);

    ASSERT_EQ(persons.size(), 1);
    EXPECT_TRUE(HasSmith(persons));
}

TEST_F(SqliteTest, testMultiplyOperator)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, COL(Person::age) * 2 == 110);

    ASSERT_EQ(persons.size(), 1);
    EXPECT_TRUE(HasSmith(persons));
}

TEST_F(SqliteTest, testDivideOperator)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, COL(Person::age) / 2 == 26);

    ASSERT_EQ(persons.size(), 1);
    EXPECT_TRUE(HasLenin(persons));
}

TEST_F(SqliteTest, testReminderOperator)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, COL(Person::age) % 2 == 1);

    ASSERT_EQ(persons.size(), 2);
    EXPECT_TRUE(HasLenin(persons));
    EXPECT_TRUE(HasSmith(persons));
}

TEST_F(SqliteTest, testBitwiseAndOperator)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, (COL(Person::age) & 1) == 1);

    ASSERT_EQ(persons.size(), 2);
    EXPECT_TRUE(HasLenin(persons));
    EXPECT_TRUE(HasSmith(persons));
}

TEST_F(SqliteTest, testBitwiseOrOperator)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, (COL(Person::age) | 128) == 128 + 55);

    ASSERT_EQ(persons.size(), 1);
    EXPECT_TRUE(HasSmith(persons));
}

TEST_F(SqliteTest, testShiftLeft)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, (COL(Person::age) << 2) == 55 << 2);

    ASSERT_EQ(persons.size(), 1);
    EXPECT_TRUE(HasSmith(persons));
}

TEST_F(SqliteTest, testShiftRight)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, (COL(Person::age) >> 1) == 55 >> 1);

    ASSERT_EQ(persons.size(), 1);
    EXPECT_TRUE(HasSmith(persons));
}

TEST_F(SqliteTest, testUnaryPlus)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, +COL(Person::cat_weight) > 0);

    ASSERT_EQ(persons.size(), 2);
    EXPECT_TRUE(HasLenin(persons));
    EXPECT_TRUE(HasSmith(persons));
}

TEST_F(SqliteTest, testUnaryMinus)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, -COL(Person::cat_weight) > -1.5);

    ASSERT_EQ(persons.size(), 1);
    EXPECT_TRUE(HasSmith(persons));
}

TEST_F(SqliteTest, testBinaryNegation)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, ~COL(Person::age) == ~55);

    ASSERT_EQ(persons.size(), 1);
    EXPECT_TRUE(HasSmith(persons));
}

TEST_F(SqliteTest, TestNegation)
{
    SqlTransaction transaction;
    auto persons = Storable<Person>::fetchAll(transaction, !(COL(Person::name) == String("Smith")));

    ASSERT_EQ(persons.size(), 2);
    EXPECT_FALSE(HasSmith(persons));
}

// test a complex query
TEST_F(SqliteTest, allInOneSelectTest)
{
    SqlTransaction transaction;
    Storable<Person> person;

    SqlResultSet resultSet = person.select().innerJoin<City>().where((COL(Person::age).isNull() ||
            (coalesce(COL(Person::age), 0) + 2.5  * COL(Person::cat_weight)) > 250) &&
            COL(Person::cityId) == COL(City::id) &&
            !lower(COL(Person::name)).like("invalid_%") &&
            coalesce(COL(Person::birthday), DateTime::fromString("2000-01-01 00:00:00")) >= DateTime::fromString("2000-01-01 00:00:00"))
            .limit(10).orderAsc(COL(Person::name), COL(City::name)).orderDesc(COL(Person::age)).exec(transaction);

    Array<Person> persons;
    for (auto row : resultSet)
    {
        EXPECT_EQ(row, persons.size());
        persons.push_back(person);
    }

    transaction.commit();
}

TEST_F(SqliteTest, updateTest)
{
    {
        SqlTransaction transaction;
        Storable<Person> person;
        auto affected = person.update().ref<City>().set(COL(Person::name) = "Pupkin Jr")
                .where(COL(Person::cityId) == COL(City::id) && COL(City::name) == String("Moscow") &&
                       (COL(Person::age) == null || COL(Person::cat_weight) == null)).exec(transaction);
        EXPECT_EQ(affected, 1);

        transaction.commit();
    }

    {
        SqlTransaction transaction;

        EXPECT_EQ(Storable<Person>::fetchAll(transaction, COL(Person::name) == String("Pupkin Jr")).size(), 1);
        EXPECT_EQ(Storable<Person>::fetchAll(transaction, COL(Person::name) == String("Pupkin")).size(), 0);
    }
}

TEST_F(SqliteTest, insertTest)
{
    auto now = DateTime::now();
    Storable<Person> pupkin;
    {
        SqlTransaction transaction;
        auto result = pupkin.select().where(COL(Person::name) == String("Pupkin")).exec(transaction);
        ASSERT_TRUE(result.begin() != result.end());
    }
    {
        SqlTransaction transaction;
        pupkin.name = "Pupkin Jr.";
        pupkin.birthday = now;
        ASSERT_TRUE(pupkin.insertOne(transaction));
        transaction.commit();
    }
    {
        SqlTransaction transaction;

        auto persons = Storable<Person>::fetchAll(transaction);
        auto pupkins = Storable<Person>::fetchAll(transaction, COL(Person::name).like("Pupkin%"));

        ASSERT_EQ(persons.size(), 4);
        ASSERT_EQ(pupkins.size(), 2);
        auto pupkinSr = std::find_if(pupkins.begin(), pupkins.end(), [](const Person& p) { return p.name == "Pupkin"; });
        auto pupkinJr = std::find_if(pupkins.begin(), pupkins.end(), [](const Person& p) { return p.name == "Pupkin Jr."; });

        ASSERT_NE(pupkinSr, pupkins.end());
        ASSERT_NE(pupkinJr, pupkins.end());

        EXPECT_EQ(pupkinSr->birthday, nullptr);
        EXPECT_EQ(pupkinJr->birthday, now);
    }
}

TEST_F(SqliteTest, deleteTest)
{
    {
        SqlTransaction transaction;
        Storable<Person> person;
        auto affected = person.remove().ref<City>().where(COL(City::id) == COL(Person::cityId) &&
                                          COL(City::name) == String("Moscow") &&
                                          COL(Person::name) == String("Lenin")).exec(transaction);
        EXPECT_EQ(affected, 1);
        EXPECT_EQ(person.fetchAll(transaction, COL(Person::name) == String("Lenin")).size(), 0);

        transaction.commit();
    }
    {
        SqlTransaction transaction;
        auto persons = Storable<Person>::fetchAll(transaction);
        ASSERT_EQ(persons.size(), 2);
        EXPECT_FALSE(HasLenin(persons));
    }
}
#endif
