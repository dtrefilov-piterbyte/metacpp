#ifndef SQLTEST_H
#define SQLTEST_H
#include <gtest/gtest.h>
#include "SqliteConnector.h"
#include "SqlTransaction.h"

class SqlTest : public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

    void selectTest();
    void updateTest();
    void insertTest();
private:
    void prepareSchema();
private:
    metacpp::sql::connectors::sqlite::SqliteConnector *m_conn;
};

#endif // SQLTEST_H
