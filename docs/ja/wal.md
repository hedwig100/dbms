# WAL

## Overview
Write-Ahead logging(WAL)の必要性, 実現するアルゴリズムについて説明する.
WALについて理解するためにはログデータからデータベースをリカバリするための
アルゴリズムを理解する必要があるためそこから説明を始める. 

## Recoveryアルゴリズム
まず, Recoveryアルゴリズムについて説明する. Recoveryアルゴリズムは
次のようなアルゴリズムである. 

```
// The undo stage
For each log record (reading backwards from the end)
1. もしレコードがコミットレコードなら
    そのトランザクションをコミット済みトランザクションのリストに入れる. 
2. もしレコードがロールバックレコードなら
    そのトランザクションをロールバック済みトランザクションのリストに入れる. 
3. もしレコードがコミット済みでもなくロールバック済みでもないトランザクションの更新レコードなら
    Undo(古いデータを復元)する.

// The redo stage
For each log record (reading forwards from the beginning)
1. もしレコードが更新レコードでそのトランザクションがコミット済みトランザクションのリストに入っていたら, 
    Redo(新しいデータを回復)する.
```

上のアルゴリズムによってロールバック済みもしくは実行途中のトランザクションが
更新したデータはすべてデータディスクからは取り除かれる. またコミット済みのトランザクションの更新したデータはすべてデータディスクに書き込まれる. 

## Write-Ahead Logging

実は上に書いたアルゴリズムはあることを前提としている. それは以下の仮定である.

仮定: 完了していないトランザクションに対するすべての更新はその対応するログレコードがディスクに書き込まれている.

もし上の仮定が正しくない場合, たとえば完了していないトランザクションの更新が行われているが, 対応するログレコードがディスク上にないということが起こり得る. このとき, 更新があったことがわからないため, 上のアルゴリズムでは(どんなアルゴリズムでも)このトランザクションによる更新をUndoすることはできない. ゆえに上の仮定は常に成立するようなアルゴリズムである必要がある.

## WALを実現するアルゴリズム

では上の仮定を満たすようにデータの更新やログの書き込みを行うにはどのようにすればよいのだろうか. 二つの方法が存在する.

1. 更新ログレコードをすぐにflushする.
毎回更新用ログレコードをログファイルに書き込むたびに, そのレコードをflushする. 
すると仮定は正しく満たされる. しかし, この方法は毎回flushを行う必要があるため, 
かなりデータベースの効率を下げる.

2. 更新がディスクに反映されるタイミングでログファイルをflushする.
更新データが含まれるバッファがディスクに書き込まれるタイミングでログレコードをflushする
こともできる. これを実装するための標準的な方法はそれぞれのバッファがそのバッファを変更
したもっとも最近の更新に対応するログレコードのLSN(Log Sequence Number)をもっておき,
そのバッファがディスクに書き込まれるタイミングで, そのLSNより前のログをすべてflushするという
ようにすることで行える. こうすることでログファイルのflushはそれほど頻繁に発生しなくなる.

本データベースでは2の方法をとる.

## 参考
- Database Design and Implementation, Second Edition, Edward Sciore, Data-Centric Systems and Applications,