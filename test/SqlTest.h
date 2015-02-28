#ifndef SQLTEST_H
#define SQLTEST_H
#include <gtest/gtest.h>
#include "SqlTransaction.h"

class SqlTest : public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;
    static void SetUpTestCase();
    static void TearDownTestCase();

    void prepareSchema();
    void prepareData();
    void clearData();
private:
    std::unique_ptr<metacpp::db::sql::connectors::SqlConnectorBase> m_conn;
};

#endif // SQLTEST_H
