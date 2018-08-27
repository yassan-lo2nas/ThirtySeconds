# ThirtySeconds
30秒以内に迷路を脱出するプログラム。迷路は手動で指定。

## 0, 概要
　当プログラムは, 基礎プログラミング演習 II の講義の最終課題のために作成したプログラムである。
　HandyGraphicsをインストールしてから実行してください。
　MacOSX HighSierraにて動作確認。
　ゲームに登場するアイコン等は「./img」フォルダ直下の「dot.c」プログラムにて作成した。

## 1, 実行上の注意
　「img」フォルダが「ThirtySeconds.c」と同一階層のディレクトリにあることを確認する。
　その後、「Handy Graphics」がインストールされた上で
「$(hgcc ThirtySeconds12.c; ./a.out;)」
　を実行し, プログラム開始。()を含んだ 「$ ~~ 」以下をコピペで実行して下さい。

## 2, 権利について
　本プログラムの権利についてはすべて作成者に帰属しております。が、本プログラムを二次利用する場合は、参照元を明記して頂くと有り難いです。これは強制ではありません。

## 3, その他, 操作方法など

  1.プログラムの内容
　　パックマンを操作しゴールまでたどり着け！
　　制限時間は30秒！ 追跡者から逃げ生きのびろ！

  2.起動・操作方法
  　起動時点で操作可能。
　　操作は,

  	 ↑ ：上方向に移動
  	 → ：右方向に移動
  	 ↓ ：下方向に移動
  	 ← ：左方向に移動
  	ESC：ゲームを中断
　	   C ：GAMECLEAR のチート
　	   V ：GAMEOVER  の自爆

　　の通り.
   また、ゴールするか、敵に追いつかれた時点でゲーム終了. 
　　その後何かのキーを押すとプログラム終了.

  3.制限事項,不具合点
    未確認.