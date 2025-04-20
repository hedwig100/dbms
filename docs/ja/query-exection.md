# クエリ実行

このドキュメントでは与えられたSQLクエリを実行する際のプロセスを示す。

プロセス全体は

![クエリ実行プロセス](images/query-execution.drawio.png)

上の写真のようになっている。


1. SQLの入力を受け取る
2. `src/parser`に実装されているパーサーがSQLを解析し、`src/execute/sql.h`に実装されている抽象構文木として返す。
3. 抽象構文木の根に対応するクラスはすべて`Statement`を基底クラスに持ち、`Statement.Execute(transaction::Transaction&, execute::QueryResult&, const execute::Environment&)` というメソッドを持つ。これを実行してクエリを実行する. 
    - `execute::Environment` は環境情報を持つ. すなわち`TableManager`などを持つ.
    - `Execute`メソッドは`src/scan.h`, `src/scans.h`などに実装されている`Scan, SelectScan`などを用いてクエリを実行している。
4. `Execute`によって得られた`QueryResult`が実行結果である。