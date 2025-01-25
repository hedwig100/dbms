# Specification

このドキュメントではこのDBMSのインターフェースについて説明する.

## 型
このDBMSは次のような型をサポートする. 

- `INT`: 32ビット整数型
- `CHAR(N)`: 長さN(0 <= N < 256)のASCIIコードで表現される文字列

## DML

以下のDMLをサポートする.

- `SELECT`: データを読む.
- `INSERT`: データを追加する. 
- `UPDATE`: データを更新する. 
- `DELETE`: データを消す.

SQLステートメントとしては以下をサポートする.
`*`は0回以上の繰り返し, `|` はいずれか一つ, `?`はそれが0個か1個あることを示す.
`{any characters}, {any numbers}`はそれぞれ任意の文字列, 任意の数字を表す.

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
<name> = [A-Za-z] ([0-9A-Za-z])
```

上の条件を満たすステートメントは例えば, 
```
SELECT a, b FROM A;
SELECT A.a, B.b FROM A INNER JOIN B ON A.b = B.a;

INSERT INTO C VALUES (0, "c", 2);

UPDATE D SET col = "2a3";
UPDATE E SET col1 = "243", col2 = 2 WHERE e = 0;

DELETE FROM F;
DELETE FROM F WHERE f = 0;
```
などがある.