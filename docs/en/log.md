# Log format

Our dbms logs information for recovering commited transactions.
This document explains logging in this DBMS.

This database mainly uses Redo log. Durability is realized by writing logs before 
writing actual data into disks (Write Ahead Logging), and reproduce data 
using the logging data when recovering.

## Data Granularity

Outputs logs the following points;

- When a transaction starts,
- When an item is written to the disk,
- When a transaction is committed and rollbacked,
- When a checkpointing finished,

Log data is written by a data item (such as an integer, string).

## Log format

The detailed log format is the following. 
A checksum and length of the log is written at the head of each log,
and the log body is written after that. The length of a log is written 
in bytes.

```
| Checksum | log length | log body |
```

Length of loggings for each kind (log length, log body) is the following.

### Beginning of a transaction

```
| 00 | transaction_id |
```

### An operation to a data item

Let MANIP_TYPE be 

- 00: insert
- 01: update
- 10: delete.

Then, the log format is 

```
| 01 | transaction_id | MANIP_TYPE (2bits) | filename | offset | TYPE (4bits) | content | 
```

The offset is in bytes. TYPE represents the type of a data item such as an integer of char(N), and content is the value of the data item.

### End of a transaction

Let END_TYPE be

- 0: commit
- 1: rollback.

Then, the log format is 

```
| 10 | transaction_id | END_TYPE (1bit) |
```

### Checkpointing

This log implies that all updates before this log is written to the disk.

```
| 11 |
```

## Atomic write of log

A log needs to be added atomically. This is realized by using the checksum.

1. Computes the checksum of a log that will be written.
2. Checksum in the log record is set to the checksum computed at 1.

The integrity of the log record is assured by the checksum when reading a log record;
firstly read a log record, and then if the checksum computed is identical to the 
Checksum of the log record, the log record is integral.