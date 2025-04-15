#ifndef _EXECUTE_EXECUTE_H
#define _EXECUTE_EXECUTE_H

#include "execute/environment.h"
#include "execute/query_result.h"
#include "metadata.h"
#include "result.h"
#include "transaction/transaction.h"

namespace execute {

using namespace result;

// Execute the SQL statement and return the result as a QueryResult
// (corresponding the argument `result`).
Result Execute(const std::string &sql, execute::QueryResult &result,
               transaction::Transaction &transaction, const Environment &env);

}; // namespace execute

#endif // _EXECUTE_EXECUTE_H