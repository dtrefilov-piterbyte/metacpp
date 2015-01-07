/****************************************************************************
* Copyright 2014-2015 Trefilov Dmitrij                                      *
*                                                                           *
* Licensed under the Apache License, Version 2.0 (the "License");           *
* you may not use this file except in compliance with the License.          *
* You may obtain a copy of the License at                                   *
*                                                                           *
*    http://www.apache.org/licenses/LICENSE-2.0                             *
*                                                                           *
* Unless required by applicable law or agreed to in writing, software       *
* distributed under the License is distributed on an "AS IS" BASIS,         *
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
* See the License for the specific language governing permissions and       *
* limitations under the License.                                            *
****************************************************************************/
#ifndef SQLSTORABLE_H
#define SQLSTORABLE_H
#include <cstdint>
#include <memory>
#include "Object.h"
#include "SqlStatement.h"
#include "SqlColumnConstraint.h"

namespace metacpp
{
namespace sql
{
    /** Base interface for persistable objects */
    class SqlStorable
    {
    protected:
        SqlStorable();
    public:
        virtual ~SqlStorable();

        virtual const MetaField *primaryKey() const = 0;
        virtual Object *record() = 0;

        SqlStatementSelect select();
        SqlStatementDelete remove();    // delete is reserved
        SqlStatementUpdate update();

        /** Insert record into the database */
        bool insertOne(SqlTransaction& transaction);
        /** Persist changes on the record by primary key */
        bool updateOne(SqlTransaction& transaction);
        /** Delete the record by primary key */
        bool removeOne(SqlTransaction& transaction);

        String fieldValue(const MetaField *field) const;
    protected:
        static void createSchema(SqlTransaction& transaction, const MetaObject *metaObject,
                                 const Array<SqlConstraintBasePtr>& constraints);
    private:
        String whereId();
        static void createSchemaSqlite(SqlTransaction& transaction, const MetaObject *metaObject,
                                       const Array<SqlConstraintBasePtr>& constraints);
        static void createSchemaPostgreSQL(SqlTransaction &transaction, const MetaObject *metaObject,
                                    const Array<SqlConstraintBasePtr> &constraints);
    };

    template<typename TObj>
    class Storable : public SqlStorable, public TObj
    {
    public:
        Storable() : m_pkey(nullptr) {
        }

        const MetaField *primaryKey() const override {
            //return getMetaField(&TObj::id);
//            if (m_pkey) return m_pkey;
            for (size_t i = 0; i < ms_constraints.size(); ++i)
                if (ms_constraints[i]->type() == SqlConstraintTypePrimaryKey)
                    return m_pkey = ms_constraints[i]->metaField();
            return nullptr;
        }

        static SqlConstraintBasePtr getConstraint(size_t i) {
            return ms_constraints[i];
        }

        static size_t numConstraints() {
            return ms_constraints.size();
        }

        static void createSchema(SqlTransaction& transaction)
        {
            SqlStorable::createSchema(transaction, TObj::staticMetaObject(), ms_constraints);
        }

    private:

        Object *record() override {
            return this;
        }
    private:
        mutable const MetaField *m_pkey;
        static const Array<SqlConstraintBasePtr> ms_constraints;
    };

#define DECLARE_STORABLE(TObj, ...) \
template<> const Array<SqlConstraintBasePtr> Storable<TObj>::ms_constraints = { __VA_ARGS__ };

} // namespace sql
} // namespace metacpp
#endif // SQLSTORABLE_H
