#ifndef _EXECUTE_EXECUTE_H
#define _EXECUTE_EXECUTE_H

#include "result.h"
#include "transaction/transaction.h"

namespace execute {

using namespace result;

Result Execute(const std::string &sql, transaction::Transaction &transaction);

}; // namespace execute

#endif // _EXECUTE_EXECUTE_H