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
and the log body is written after that. The length of a log is the length of the log body without checksum of other headers, written in bytes. The length is written at the tail of the log record so that the log file can be read from the bottom.

```
| Checksum | log length | log body | log length |
```

Length of loggings for each kind (log length, log body) is the following.

### Beginning of a transaction

Has the following log body.
```
| 0b00000000 | transaction_id |
```

### An operation to a data item

Has the following log body.
```
| 0b01000000 | transaction_id | filename length | filename | offset | previous_content | new_content | 
```

- filenam length is length of the filename in bytes.
- filename, offset is the place where the data item is written.
- previous_content and new_content must have the same length.

### End of a transaction

Has the following log body.
```
| 0b10{END_TYPE(1bit)}00000 | transaction_id |
```

- END_TYPE represents commit or rollback in 1bit.

### Checkpointing

This log implies that all updates before this log is written to the disk.

Has the following log body.
```
| 0b11000000 |
```

## Atomic writes of log

A log needs to be added atomically. This is realized by using the checksum.

1. Computes the checksum of a log that will be written.
2. Checksum in the log record is set to the checksum computed at 1.

The integrity of the log record is assured by the checksum when reading a log record;
firstly read a log record, and then if the checksum computed is identical to the 
Checksum of the log record, the log record is integral.