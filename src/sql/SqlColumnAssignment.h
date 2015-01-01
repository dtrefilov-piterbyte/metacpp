#ifndef SQLCOLUMNASSIGNMENT_H
#define SQLCOLUMNASSIGNMENT_H
#include "String.h"

namespace metacpp
{
namespace sql
{

class SqlColumnAssignmentBase
{
public:
    SqlColumnAssignmentBase();
    ~SqlColumnAssignmentBase();

    /** SQL expression of left-hand side of an assignment */
    virtual String lhs() const = 0;
    /** SQL expression of right-hand side of an assignment */
    virtual String rhs() const = 0;
};

} // namespace sql
} // namespace metacpp

#endif // SQLCOLUMNASSIGNMENT_H
