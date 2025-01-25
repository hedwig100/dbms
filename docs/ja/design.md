# Design

このドキュメントでは実装にまで踏み込んで, どのように内部の実装が行われるかについて議論する.

## Overview

実装は大きく分けてACID特性を満たすように書き込み, 読み込みを行うストレージエンジンとストレージエンジンを用いて,
実際にクエリを処理するクエリエンジンに別れる. このドキュメントにおいてはストレージエンジンはトランザクション以下の実装を指し,
クエリエンジンはトランザクションを用いてクエリを処理する部分の実装を指すこととする.

[TODO: transaction以下についてはいったん略]

[TODO: query engineについての概略]


## Storage Engine 

[TODO: transaction以下についてはいったん略]

## Query Engine

### `Scan`

`Scan`はテーブルもしくはテーブルの一部をスキャンすることを表す抽象クラス.
`Scan`はあるレコードを指しており, `Next()`などのメソッドで移動する.
このクラスでテーブルの更新, 削除などもできるようにする.

```cpp
class Scan {
    // 次のレコードに進む.
    bool Next();

    int GetInt(std::string fieldname);
    std::string GetString(std::string fieldname);
    void Close();
};

class UpdateScan: public Scan {
    // fieldnameのフィールドをvalueにする.
    void Update(std::string fieldname, const data::Data& value);

    // 現在のレコードをテーブルに挿入し, 新しいレコードにする.
    void Insert();

    // 現在のレコードをテーブルから消す.
    void Delete();
};
```

### フルテーブルスキャン

`TableScan`は`UpdateScan`の派生クラスとして実装する.
`TableScan`はテーブルにあるすべてのレコードを走査するスキャンである.

### 関係代数
以下のクラスを実装する.

- `Select(Scan s, Predicate p): Scan`
    - `s`から`p`が真になるもののみ含む`Scan`を返す.
- `UpdateSelect(UpdateScan s, Predicate p): UpdateScan`
    - `s`から`p`が真になるもののみ含む`UpdateScan`を返す.
- `Project(Scan s, Columns cols): Scan`
    - `s`から`cols`のカラムのみ含む`Scan`を返す.
- `Product(Scan s1, Scan s2, Predicate p): Scan`
    - `s1`, `s2`の直積から`p`が真になるもののみ含む`Scan`を返す.