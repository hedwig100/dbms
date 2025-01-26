# Design

This document goes into the implementation and discusses how the internal implementation is done.

## Overview

The implementation can be roughly divided into two parts: a storage engine that writes and reads to satisfy the ACID property, and a query engine that actually processes the query using the storage engine.
The query engine actually processes the query using the storage engine. 
In this document, the storage engine refers to the implementation below the transaction, and the query engine refers to the implementation that processes the query using the transaction.

[TODO: transaction and below are omitted for now.]

[TODO: Outline of query engine].


## Storage Engine 

[TODO: transaction and below are omitted]

## Query Engine

### `Scan` is a table or tables.

`Scan` is an abstract class for scanning a table or part of a table.
`Scan` points to a record and is navigated to by methods such as `Next()`.
This class can also be used to update or delete a table.

```cpp
class Scan {
    // move on to the next record.
    bool Next();

    int GetInt(std::string fieldname);
    std::string GetString(std::string fieldname);
    void Close();
}; 

class UpdateScan: public Scan {
    // set fieldname field to value.
    void Update(std::string fieldname, const data::Data& value);

    // insert the current record into the table and make it a new record.
    void Insert();

    // delete the current record from the table.
    void Delete(); 
}; 
```

### Full TableScan

`TableScan` is implemented as a derived class of `UpdateScan`.
TableScan` is a scan that traverses all records in a table.

### Relational Algebra
Implement the following class.

- `Select(Scan s, Predicate p): Scan`.
    - Return a `Scan` containing only those records from `s` for which `p` is true.
- `UpdateSelect(UpdateScan s, Predicate p): UpdateScan`.
    - return an `UpdateScan` containing only those `s` to `p` that are true.
- `Project(Scan s, Columns cols): Scan`.
    - Return a `Scan` containing only the columns from `s` to `cols`.
- `Product(Scan s1, Scan s2, Predicate p): Scan`
    - Return a `Scan` containing only the direct products of `s1` and `s2` for which `p` is true.