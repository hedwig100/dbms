#ifndef _EXECUTE_EXECUTE_H
#define _EXECUTE_EXECUTE_H

#include "result.h"
#include "transaction/transaction.h"

namespace execute {

using namespace result;

// Execute the SQL statement and return the result as a QueryResult
// (corresponding the argument `result`).
Result Execute(const std::string &sql, transaction::Transaction &transaction,
               execute::QueryResult &result);

}; // namespace execute

#endif // _EXECUTE_EXECUTE_H