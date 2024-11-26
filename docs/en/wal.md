# WAL

## Overview
This document explains the necessity of Write-Ahead Loggin (WAL) and algorithms to 
achieve WAL. We start to explain the algorithm to recover the database content
from the log file, because the understanding of the algorithm is necessary to 
understand WAL.

## Recovery algorithm
Firsly, we explains the Recovery algorithm. The recovery algorithm is the 
following algorithm.

```
// The undo stage
For each log record (reading backwards from the end)
1. If the record is a commit record,
    Put the transaction to the committed transaction list.
2. If the log record is a rollback record,
    Put the transaction to the rollbacked transaction list.
3. If the log record is an update record of a transaction which is neither committed or rollbacked, 
    Undo(recover the old data) the update.

// The redo stage
For each log record (reading forwards from the beginning)
1. If the log record is an update record of a transaction which is committed
    Redo(recover the new data) the update.
```

The algorithm above removes all updates of transactions which are rollbacked or under execution (neither rollbacked or committed), and writes all updates of committed transactions.

## Write-Ahead Logging

Actually, the algorithm above assumes one following fact;

Assumption: All updates for an uncompleted transaction will have a
corresponding log record in the log file.

If the assumption above is not satisfied, there is a case that an update of an uncompleted transaction is written to the disk, but the corresponding log record is not on the log file. In this case, we cannot know the update is already written to the disk, the algorithm above (actually, any algorithm) cannot undo the update.
Therefore, writes to the disk must be done to make the assumption true.

## Algorithms that realize WAL

So, how should we write data to disk or log records to the log file to achieve WAL?
There are two ways.

1. Flushes the log file right after the update log record is written to the log file.
Flushes the record each time am update log record is written to the log file.
Then, the assumption is satisfied. However, we have to flush every time an update record is written to the disk and make the databse inefficient.

2. Flushes the log file when the updates are written to the disk.
It is possible that when a buffer containing an update data is written to the disk, we flush the log record. The standard way to implement this is that each buffer has LSN(Log Sequence Number) of the log record of the most recent updates
on the buffer, and flushes all log records up to the LSN when the buffer is written to the disk. By doing this, the flush of the log file does not frequently happen.

This databse uses the second algorithm.

## Citation
- Database Design and Implementation, Second Edition, Edward Sciore, Data-Centric Systems and Applications,