# Concurrency control

In our database system, multiple transactions are running in parallel. Their operations must satisfy the ACID properties. Isolation, one of the ACID properties, means that the result must be the same as if the transactions had been executed in series (serializability). In order to satisfy this serializability, its is necessary to restrict the read/write of transactions, and we describe here how to manage this.

## 用語

- Serializability: When multiple transactions are executed in parallel, the execution order is said to have serializability when the results of their read/write operations match the results of executing them in a certain order in series.
- Exclusive lock: A type of lock in which no other transaction can acquire any lock when this lock is acquired.
- Shared lock: A type of lock in which, when this lock is acquired, other transactions can acquire the shared lock but not the exclusive lock.
- Two-phase lock: A locking scheme in which each transaction cannot lock after the transaction unlocks any locks.

## 概要

A transaction locks a disk block by block. When you want to read data on a certain block, you apply a shared lock. When you want to read data on a certain block, a shared lock is applied. When you want to write data on a block, an exclusive lock is applied.

There are several isolation levesl, but for now, we will implement the one that achieves the strongest isolation level, Serializability.

In order to avoid deadlocks, the lock acquisition is considered to have failed after a certain periodo of time has elapsed while trying to acquire a lock. If the lock acquisition faild, whether to acquire the lock again of to roll back is determined probabilistically.

## 実装

We implement two class named `LockTable`, `ConcurecyManager`. 

- `LockTable`
    - Shared with transactions (thread-safe object).
    - Manages information on which block is locked.
    - Fail to acquire lock when deadlock occurs.

- `ConcurrencyManager`
    - Owned by each transaction.
    - Have responsibilities on locking and unlocking.


