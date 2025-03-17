# Specification

This document explains external specification of dbms (interface).

## Data types

This DBMS supports the following types.

- `INT`: a 32bit integer
- `CHAR(N)`: a string represented as ASCII code of length $N\ (0 \leq N < 256)$, trailing spaces are removed.

## DML

Supports the following DML;

- `SELECT`: read data
- `INSERT`: add data
- `UPDATE`: update data
- `DELETE`: delete data