# Specification

This document explains external specification of dbms (interface).

## Data types

This DBMS supports the following types.

- `INT`: a 32bit integer
- `CHAR(N)`: a string represented as ASCII code of length $N\ (0 \leq N < 256)$, trailing spaces are removed.

## DML

Supports the following DML;

- `SELECT`: read data

This dbms supports the following sql statements.
`*` means repeats more than 0 times, `|` means either of the side, `?` means 0 or 1 expression.

```
<statement> = ( <select-statement> ) ";"
<select-statement> = "SELECT" <columns> "FROM" <table>

<columns> = '*' | <column> | <columns> ',' <column>
<column> = <integer> | <id>
<table> = <id>
<integer> = [0-9]+
<id> = [_a-zA-Z][_a-zA-Z0-9]*
```

For example, the following statements satisfy the grammer.
```
SELECT a FROM table;
SELECT 2 FROM tab;
```