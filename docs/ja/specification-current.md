# Specification

このドキュメントではこのDBMSのインターフェースについて説明する.

## 型
このDBMSは次のような型をサポートする. 

- `INT`: 32ビット整数型
- `CHAR(N)`: 長さN(0 <= N < 256)のASCIIコードで表現される文字列、末尾のスペースは削除される。

## DML

以下のDMLをサポートする.

- `SELECT`: データを読む.

SQLステートメントとしては以下をサポートする.
`*`は0回以上の繰り返し, `|` はいずれか一つ, `?`はそれが0個か1個あることを示す.

```
<statement> = ( <select-statement> ) ";"
<select-statement> = "SELECT" <columns> "FROM" <table> <where-clause>

<columns> =  '*' | <select-expr> | <columns> ',' <select-expr>
<select-expr> = <column> | <expr>
<expr> = <boolean_primary>

<where-clause> = ( "WHERE" <boolean_primary> )?
<boolean_primary> = <column> <comparison-operator> <column>
<comparison-operator> = '=' | '<' | '>' | '<=' | '>='

<column> = <integer> | <id>
<table> = <id>
<integer> = [0-9]+
<id> = [_a-zA-Z][_a-zA-Z0-9]*
```

上の条件を満たすステートメントは例えば, 
```
SELECT a FROM table;
SELECT 2 FROM tab;
SELECT a, 2 FROM table WHERE a <= 5;
```
などがある.