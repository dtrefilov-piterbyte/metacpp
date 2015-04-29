#include "DateTimeTest.h"

using metacpp::DateTime;

TEST_F(DateTimeTest, testInvalidDateTime)
{
    DateTime dt;
    ASSERT_FALSE(dt.valid());
}

TEST_F(DateTimeTest, testValidDateTime)
{
    DateTime dt = DateTime::now();
    ASSERT_TRUE(dt.valid());
}

TEST_F(DateTimeTest, testEquality)
{
    DateTime dt1, dt2;
    ASSERT_NO_THROW(dt1 = DateTime::fromString("2004-02-01 14:25:16"));
    ASSERT_NO_THROW(dt2 = DateTime::fromString("2004-02-01 14:25:16"));
    EXPECT_EQ(dt1, dt2);
}

TEST_F(DateTimeTest, testIncompleteIsoString)
{
    EXPECT_THROW(DateTime::fromString("2004"), std::invalid_argument);
    EXPECT_THROW(DateTime::fromString("2004-02"), std::invalid_argument);
    EXPECT_THROW(DateTime::fromString("2004-02-01"), std::invalid_argument);
    EXPECT_THROW(DateTime::fromString("2004-02-01 14"), std::invalid_argument);
    EXPECT_THROW(DateTime::fromString("2004-02-01 14:25"), std::invalid_argument);
    EXPECT_NO_THROW(DateTime::fromString("2004-02-01 14:25:16"));
}

TEST_F(DateTimeTest, testInvalidIsoString)
{
    EXPECT_THROW(DateTime::fromString("time: 2004-02-01 14:25:16"), std::invalid_argument);
    EXPECT_THROW(DateTime::fromString("2004-02-01 14:25:16 Sun"), std::invalid_argument);
    EXPECT_NO_THROW(DateTime::fromString("2004-02-01 14:25:16"));
    EXPECT_THROW(DateTime::fromString("2004-02-32 14:25:16"), std::invalid_argument);
    EXPECT_THROW(DateTime::fromString("2004-00-01 14:25:16"), std::invalid_argument);
    EXPECT_THROW(DateTime::fromString("2004-13-01 14:25:16"), std::invalid_argument);
    EXPECT_THROW(DateTime::fromString("2004-02-01 24:25:16"), std::invalid_argument);
    EXPECT_THROW(DateTime::fromString("2004-02-01 14:60:16"), std::invalid_argument);
    EXPECT_THROW(DateTime::fromString("2004-02-01 14:25:62"), std::invalid_argument);
    EXPECT_NO_THROW(DateTime::fromString("2004-12-31 23:59:59"));
}

TEST_F(DateTimeTest, testValidIsoString)
{
    DateTime dt;
    ASSERT_NO_THROW(dt = DateTime::fromString("2004-02-01 14:25:16"));
    EXPECT_EQ(dt.year(), 2004);
    EXPECT_EQ(dt.month(), metacpp::February);
    EXPECT_EQ(dt.day(), 1);
    EXPECT_EQ(dt.hours(), 14);
    EXPECT_EQ(dt.minutes(), 25);
    EXPECT_EQ(dt.seconds(), 16);
}

TEST_F(DateTimeTest, testSetDay)
{
    DateTime dt;
    ASSERT_NO_THROW(dt = DateTime::fromString("2004-02-01 14:25:16"));
    dt.setDay(28);
    EXPECT_EQ(dt.day(), 28);
#ifndef _MSC_VER
    EXPECT_EQ(dt.dayOfWeek(), metacpp::Sataday);
#endif
    EXPECT_EQ(dt, DateTime::fromString("2004-02-28 14:25:16"));
}

TEST_F(DateTimeTest, testSetMonth)
{
    DateTime dt;
    ASSERT_NO_THROW(dt = DateTime::fromString("2004-02-01 14:25:16"));
    dt.setMonth(metacpp::January);
	EXPECT_EQ(dt.month(), metacpp::January);
#ifndef _MSC_VER
    EXPECT_EQ(dt.dayOfWeek(), metacpp::Thursday);
#endif
    EXPECT_EQ(dt, DateTime::fromString("2004-01-01 14:25:16"));
}

TEST_F(DateTimeTest, testSetYear)
{
    DateTime dt;
    ASSERT_NO_THROW(dt = DateTime::fromString("2004-02-01 14:25:16"));
	dt.setYear(2005);
#ifndef _MSC_VER
    EXPECT_EQ(dt.dayOfWeek(), metacpp::Tuesday);
#endif
    EXPECT_EQ(dt, DateTime::fromString("2005-02-01 14:25:16"));
}

TEST_F(DateTimeTest, testSetYMD)
{
    DateTime dt;
    ASSERT_NO_THROW(dt = DateTime::fromString("2004-02-01 14:25:16"));
	dt.setYMD(2006, metacpp::April, 15);
#ifndef _MSC_VER
    EXPECT_EQ(dt.dayOfWeek(), metacpp::Sataday);
#endif
    EXPECT_EQ(dt, DateTime::fromString("2006-04-15 14:25:16"));

}

TEST_F(DateTimeTest, testSetHours)
{
    DateTime dt;
    ASSERT_NO_THROW(dt = DateTime::fromString("2004-02-01 14:25:16"));
    ASSERT_THROW(dt.setHours(24), std::invalid_argument);
    dt.setHours(12);
    EXPECT_EQ(dt.hours(), 12);
    EXPECT_EQ(dt, DateTime::fromString("2004-02-01 12:25:16"));
}

TEST_F(DateTimeTest, testSetMinutes)
{
    DateTime dt;
    ASSERT_NO_THROW(dt = DateTime::fromString("2004-02-01 14:25:16"));
    ASSERT_THROW(dt.setMinutes(60), std::invalid_argument);
    dt.setMinutes(12);
    EXPECT_EQ(dt.minutes(), 12);
    EXPECT_EQ(dt, DateTime::fromString("2004-02-01 14:12:16"));
}

TEST_F(DateTimeTest, testSetSeconds)
{
    DateTime dt;
    ASSERT_NO_THROW(dt = DateTime::fromString("2004-02-01 14:25:16"));
    ASSERT_THROW(dt.setSeconds(61), std::invalid_argument);
    dt.setSeconds(12);
    EXPECT_EQ(dt.seconds(), 12);
    EXPECT_EQ(dt, DateTime::fromString("2004-02-01 14:25:12"));
}

TEST_F(DateTimeTest, testSetHMS)
{
    DateTime dt;
    ASSERT_NO_THROW(dt = DateTime::fromString("2004-02-01 14:25:16"));
    dt.setHMS(0, 12, 23);
    EXPECT_EQ(dt, DateTime::fromString("2004-02-01 0:12:23"));
}

TEST_F(DateTimeTest, testAddYears)
{
    DateTime dt;
    ASSERT_NO_THROW(dt = DateTime::fromString("2004-02-01 14:25:16"));
    dt.addYears(-5);
#ifndef _MSC_VER
    EXPECT_EQ(dt.dayOfWeek(), metacpp::Monday);
#endif
    EXPECT_EQ(dt, DateTime::fromString("1999-02-01 14:25:16"));
}

TEST_F(DateTimeTest, testAddMonths)
{
    DateTime dt;
    ASSERT_NO_THROW(dt = DateTime::fromString("2004-02-01 14:25:16"));
	dt.addMonths(-5);
#ifndef _MSC_VER
    EXPECT_EQ(dt.dayOfWeek(), metacpp::Monday);
#endif
    EXPECT_EQ(dt, DateTime::fromString("2003-09-01 14:25:16"));
	dt.addMonths(5);
#ifndef _MSC_VER
    EXPECT_EQ(dt.dayOfWeek(), metacpp::Sunday);
#endif
    EXPECT_EQ(dt, DateTime::fromString("2004-02-01 14:25:16"));
}

TEST_F(DateTimeTest, testAddDays)
{
    DateTime dt;
    ASSERT_NO_THROW(dt = DateTime::fromString("2004-02-01 14:25:16"));
	dt.addDays(-64);
#ifndef _MSC_VER
    EXPECT_EQ(dt.dayOfWeek(), metacpp::Sataday);
#endif
    EXPECT_EQ(dt, DateTime::fromString("2003-11-29 14:25:16"));
}

TEST_F(DateTimeTest, testAddHours)
{
    DateTime dt;
    ASSERT_NO_THROW(dt = DateTime::fromString("2004-02-01 14:25:16"));
	dt.addHours(26);
#ifndef _MSC_VER
    EXPECT_EQ(dt.dayOfWeek(), metacpp::Monday);
#endif
    EXPECT_EQ(dt, DateTime::fromString("2004-02-02 16:25:16"));
}

TEST_F(DateTimeTest, testAddMinutes)
{
    DateTime dt;
    ASSERT_NO_THROW(dt = DateTime::fromString("2004-02-01 14:25:16"));
    dt.addMinutes(68);
    EXPECT_EQ(dt, DateTime::fromString("2004-02-01 15:33:16"));
}

TEST_F(DateTimeTest, testAddSeconds)
{
    DateTime dt;
    ASSERT_NO_THROW(dt = DateTime::fromString("2004-02-01 14:25:16"));
    dt.addSeconds(68);
    EXPECT_EQ(dt, DateTime::fromString("2004-02-01 14:26:24"));
}
