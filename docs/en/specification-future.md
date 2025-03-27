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

This dbms supports the following sql statements.
`*` means repeat more than 0 times, `|` means either of the side, `?` means 0 or 1 expression.

```
<statement> =( <select-statement> | <insert-statement> | <update-statement> | <delete-statement> ) ";"
<select-statement> = "SELECT" <fieldnames> "FROM" <table> <where>?
<insert-statement> = "INSERT INTO" <tablename> "VALUES" <values>
<update-statment> = "UPDATE" <tablename> "SET" <update-values> <where>?
<delete-statement> = "DELETE" "FROM" <tablename> <where>? 

<where> = "WHERE" <predicate>

<update-values> = (<columnname> "=" <value>) ("," <columnname> "=" <value>)*
<values> = "(" <value> ("," <value>)* ")"
<value> = '{any characters}' | {any numbers}

<table> = <tablename> <join>?
<join> = "INNTER JOIN" <tablename> "ON" <predicate>

<predicate> = <fieldname> "=" <fieldname>
<fieldnames> = <fieldname> ("," <fieldname>)*
<fieldname> = (<tablename> ".")? <name>
<tablename> = <name>
<name> = [_A-Za-z] ([0-9A-Za-z])
```

For example, the following statements satisfy the grammer.
```
SELECT a, b FROM A;
SELECT A.a, B.b FROM A INNER JOIN B ON A.b = B.a;

INSERT INTO C VALUES (0, "c", 2);

UPDATE D SET col = "2a3";
UPDATE E SET col1 = "243", col2 = 2 WHERE e = 0;

DELETE FROM F;
DELETE FROM F WHERE f = 0;
```