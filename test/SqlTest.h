#ifndef SQLTEST_H
#define SQLTEST_H
#include <gtest/gtest.h>
#include "SqlTransaction.h"

class SqlTest : public testing::Test
{
public:
    void prepareSchema();
    void prepareData();
    void cleanData();
};

class SqliteTest : public SqlTest
{
public:
    void SetUp() override;
    void TearDown() override;

    static void SetUpTestCase();
    static void TearDownTestCase();
private:
    std::unique_ptr<metacpp::db::sql::connectors::SqlConnectorBase> m_conn;
};

#endif // SQLTEST_H
