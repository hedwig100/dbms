# Query Execution

This document outlines the process for executing a given SQL query.

The entire process is illustrated in the diagram below:

![Query Exectuion Process](images/query-execution.drawio.png)


The steps are as follows:

1. The SQL input is received.
2. The parser implemented in `src/parser` analyzes the SQL and returns it as an abstract syntax tree implemented in `src/execute/sql.h`.
3. The class corresponding to the root of the abstract syntax tree inherits from the base class `Statement`, and has a method `Statement::Execute(transaction::Transaction&, execute::QueryResult&, const execute::Environment&)`. This method is called to execute the query.
    - This method uses components such as `Scan` implemented in files like `src/scan.h` to perform the actual query execution.
    - `Execute` method is implemnted in `src/scan.h, src/scans.h`.
5. The `QueryResult` obtained from `Execute` is the result of the query execution.