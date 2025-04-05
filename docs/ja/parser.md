# Parser

このドキュメントではSQLパーサーの実装について説明する。

## ファイル構成
パーサーは`src/parser`ディレクトリに実装されている。

- `parser.h, parser.cc`: パーサの実装がある。外から使う分にはこれを使えばよい。
- `execute/sql.h, execute/sql.cc`: パースした構文木を定義するための構造体類が定義されている。
    - これらは対応するSQLを実行する責任を担うので`src/parser`ディレクトリではなくて、`src/execute`ディレクトリに存在する
- `bison.y`: bisonを用いたパーサが定義されている。
- `flex.l`: Flexを用いた字句解析器の実装がある。

## Bison, Flexについて

ドキュメントは以下にある。
- [Flexのドキュメント](https://westes.github.io/flex/manual/index.html#Top)
- [Bisonのドキュメント](https://www.gnu.org/software/bison/manual/bison.html)

便利と思われるリンクやコメントを以下に書いておく。

- [Flexのパターンの書き方](https://westes.github.io/flex/manual/Patterns.html#Patterns)

- [Bisonにおけるトークン(終端記号)、非終端記号、優先順序を考慮した構文解析の定義など](https://www.gnu.org/software/bison/manual/bison.html#Decl-Summary)

- Bison, Flexともにreentrantである、すなわちパーサーがスレッドセーフである用にしている。reentrantではない場合はグローバル変数を使用しスレッドセーフにならないため、このオプションを指定する。
    - [Flex reentrant lexer](https://westes.github.io/flex/manual/Reentrant.html#Reentrant)
    - [Bison pure parser](https://www.gnu.org/software/bison/manual/bison.html#Pure-Decl)


