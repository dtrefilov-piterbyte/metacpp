#ifndef SQLTEST_H
#define SQLTEST_H
#include <gtest/gtest.h>
#include "SqlTransaction.h"

class SqlTest : public testing::Test
{
public:
    virtual void prepareSchema();
    virtual void prepareData();
    virtual void clearData();
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
