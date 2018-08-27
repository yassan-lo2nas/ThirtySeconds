/*
  秋学期 基礎プログラミング演習 II 最終課題
  「ThirtySeconds.c」

  1.プログラムの内容


  2.起動・操作方法
  起動時点で操作可能。操作は
  ↑：上方向に移動
  →：右方向に移動
  ↓：下方向に移動
  ←：左方向に移動
  ESC：ゲームを強制終了(もう一度適当にキーを押して終了)
  また、ゴールするか、敵に追いつかれた時点でゲーム終了,何かのキーを押すとプログラム終了

  3.制限事項,不具合点

 */

// 「ver.0」
// 「ver.2」（2018/01/05 9:51 ）ESCKeyのキーイベントに対する終了条件を追加
// 「ver.3」（2018/01/05 10:10）リファクタリング. クリア条件をマクロに定義したりswitch文導入
// 「ver.4」（2018/01/05 11:10）自機を表示, かつ入力に対して, 方向も変更可能.
//                             移動可能条件と, 移動可能床, 終了条件等を追加していこう.

/*---------------------------------------->8----------------------------------*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <handy.h>
#include <time.h>

/*---------------------------------------->8----------------------------------*/
/* ウィンドウ情報に必要なマクロ定義. */
#define GRID 7  // マップのグリッドの個数. 幅と高さを同じとして設計していることに注意.
#define STATUSBAR 50.0  // ゲームのステージの下に付随する情報の欄.
#define WIDTH ((double)GRID * 70.0) // ウィンドウの幅.
#define HEIGHT (WIDTH + STATUSBAR) // ウィンドウの高さ.
#define A_GRID (WIDTH/(double)GRID) // １マスの幅（高さ）.
/* キー操作に必要なマクロ定義.（一部は ライブラリ HG ~~ の中にあるのでそれを用いる.）*/
#define ESCKey 0x1B // ESCKeyのアスキーコード.
/* 終了条件に用いるマクロ定義. */
#define INTERRUPTED 0 // ゲーム終了条件, 中断.
#define GAMEOVER    1 // ゲーム終了条件, 死亡.
#define GAMECLEAR   2 // ゲーム終了条件, 成功.
/* タイマイベント, および, 敵の追跡等に用いるマクロ. */
#define INTERVAL 0.25 // タイマイベントに使う周期.
#define CHASE_NUM  2 // 敵が追いかけ始める操作回数.
#define CHASE_TIME 2.0 // ２秒毎に１マス追跡.

/*---------------------------------------->8----------------------------------*/

// ステージ情報のenum型を定義
// typedef enum Type{EMPTY=0, ME, ENEMY, START, GOAL,
//                   CLOCK, STOP, FLIP, FLOP, WALL=10}Type;
// 上の行の記述では上手く enum 型で参照出来なかったので, 下のようにマクロ定義.
#define EMPTY 0
#define ME    1
#define ENEMY 2
#define START 3
#define GOAL  4
#define CLOCK 5
#define STOP  6
#define FLIP  7
#define FLOP  8
#define WALL  10

// ステージをグローバル変数の２次変数で保持（初期状態）
int stage[GRID][GRID] = {
  {10, 10, 10, 10, 10, 10, 10},
  {10,  0,  0,  7, 10,  4, 10},
  {10,  0, 10,  0, 10,  0, 10},
  {10,  0, 10,  5, 10,  0, 10},
  {10,  0, 10,  0, 10,  0, 10},
  {10,  0, 10,  0, 10,  0, 10},
  {10,  3, 10,  6,  0,  7, 10},
};

/*---------------------------------------->8----------------------------------*/

/* 登場人物の情報を定義 */
typedef struct Me{
  double x, y, size; // xは左から 2列目, yは下から 0列目
  int direction;//向きの判定(0~3, 0:上向き)
}Me;
struct Me me1 = {2, 0, A_GRID/2.0, 0}; // {j, i, radius, flag}
                                       //；i行j列に存在する半径rの円（自分）
struct Me en1 = {2, 0, A_GRID/2.0, 0}; // {j, i, radius, flag}
                                       //；i行j列に存在する半径rの円（敵）

/*---------------------------------------->8----------------------------------*/

void showStatusBar(int layerID){ // StatueBar 部分を表示する関数
  HgWSetColor(layerID, HG_BLACK);
  //HgWSetFillColor(layerID, HG_WHITE);
  // TIME側
  HgWBox(layerID,
         0,          0,
         HEIGHT/2.0, STATUSBAR
         );

  // --------- ここに TIME を表示する処理を書く必要 ------------
  //
  // ------------------------------------------------------

  // LIFE側
  HgWBox(layerID,
         HEIGHT/2.0, 0,
         HEIGHT,     STATUSBAR
         );

  // --------- ここに LIFE を表示する処理を書く必要 ------------
  //
  // ------------------------------------------------------

  return;
}

void showStage(int layerID){ // ステージを表示, この後に自機→敵機と表示してやる必要有.
  for(int i=0; i<GRID; i++){
    for(int j=0; j<GRID; j++){
      //printf("%d, %d : %d\n", i,j,stage[i][j]); //確認用
      // マスのタイプに応じた操作
      switch (stage[i][j]) {
        case WALL:
          HgWSetFillColor(layerID, HG_BLACK);
          break;
        case EMPTY:
          HgWSetFillColor(layerID, HG_WHITE);
          break;
        case ME:
          HgWSetFillColor(layerID, HG_RED);
          break;
        case ENEMY:
          HgWSetFillColor(layerID, HG_BLUE);
          break;
        case START:
          HgWSetFillColor(layerID, HG_GREEN);
          break;
        case GOAL:
          HgWSetFillColor(layerID, HG_YELLOW);
          break;
        case CLOCK:
          HgWSetFillColor(layerID, HG_PURPLE);
          break;
        case STOP:
          HgWSetFillColor(layerID, HG_ORANGE);
          break;
        case FLIP:
          HgWSetFillColor(layerID, HG_BROWN);
          break;
        case FLOP:
          HgWSetFillColor(layerID, HG_WHITE);
          break;

        default:
          break;
      }
      // マスを表示 （layerID, x, y, 幅, 高さ, stk）
      // （追記）stkってなんぞ？
      HgWBoxFill(layerID,
                  A_GRID*j, HEIGHT-(A_GRID*(i+1)),
                  A_GRID,   A_GRID,
                  0);
    }
  }
  showStatusBar(layerID); // StatueBar 部分を表示する関数を呼び出し.
  return;
}

void showEnding(int ending){
  HgSetFont(HG_M, 50); // （フォント, 太さ）を予め指定.
  // 以下エンドフラグ別の文字列表示.
  // 敢えて, 似たような動作（HgText）を ifの外に出していない.
  // 条件毎の動作を個別に指定したい（後の拡張の）ため.
  switch(ending){
    case INTERRUPTED:
      printf("[Interrupted]\n");
      HgSetColor(HG_WHITE);
      HgText(WIDTH/4.0 +2, HEIGHT/2.0 -2, "Interrupted", 30);
      HgSetColor(HG_BLUE);
      HgText(WIDTH/4.0   , HEIGHT/2.0   , "Interrupted", 20);
      break;
    case GAMEOVER:
      printf("[Game Over！]\n");
      HgSetColor(HG_WHITE);
      HgText(WIDTH/5.0 +2, HEIGHT/2.0 -2, "Game Over！", 30);
      HgSetColor(HG_RED);
      HgText(WIDTH/5.0   , HEIGHT/2.0   , "Game Over！", 20);
      break;
    case GAMECLEAR:
      printf("[Game Clear！]\n");
      HgSetColor(HG_WHITE);
      HgText(WIDTH/5.0 +2, HEIGHT/2.0 -2, "Game Clear！", 30);
      HgSetColor(HG_ORANGE);
      HgText(WIDTH/5.0   , HEIGHT/2.0   , "Game Clear！", 20);
    default: break;
  }
  return;
}

/*---------------------------------------->8----------------------------------*/

void showPac(double x, double y, double size, int direction, int seconds, int layerID)
{//自機,敵機を表示する関数
  switch(direction){
  case 0:
    if(seconds==0)HgWFanFill(layerID, x, y, size, 0.75*M_PI, 2.25*M_PI, 0); //上向き
    else HgWFanFill(layerID, x, y, size, 0.51*M_PI, 2.50*M_PI, 0); //上向き(口閉)
    break;
  case 1:
    if(seconds==0)HgWFanFill(layerID, x, y, size, 0.25*M_PI, 1.75*M_PI, 0); //右向き
    else HgWFanFill(layerID, x, y, size, 0.01*M_PI, 2.00*M_PI, 0); //右向き(口閉)
    break;
  case 2:
    if(seconds==0)HgWFanFill(layerID, x, y, size, 1.75*M_PI, 3.25*M_PI, 0); //下向き
    else HgWFanFill(layerID, x, y, size, 1.51*M_PI, 3.50*M_PI, 0); //下向き(口閉)
    break;
  case 3:
    if(seconds==0)HgWFanFill(layerID, x, y, size, 1.25*M_PI, 0.75*M_PI, 0);//左向き
    else HgWFanFill(layerID, x, y, size, 1.01*M_PI, 3.00*M_PI, 0);//左向き(口閉)
    break;
  }
}

void move (hgevent *ev){
  //自機の座標移動
  switch (ev->ch) {//キー操作が以下の場合
    case HG_U_ARROW:
      me1.direction = 0;
      break;
    case HG_R_ARROW:
      me1.direction = 1;
      break;
    case HG_D_ARROW:
      me1.direction = 2;
      break;
    case HG_L_ARROW:
      me1.direction = 3;
      break;
    default:
      break;
  }
  return;
}

/*---------------------------------------->8----------------------------------*/

int main(void) {
  /* ローカル変数等の宣言 */
  int ending = 0;//終了フラグ（0:中断, 1:game over, 2:game clear）
  int layerID = 0;
  hgevent* ev; //イベント取得のための変数宣言.
  int ch = 0; //キー入力に対応する整数値
  int countTimer = 0; // カウンタが何回発火したかを数える.
  int seconds = 0; // 経過秒数.
  bool openflag = true; // bool={true: open, false:close} のフラグ.
  double passedTime = 0; // 経過時間を保存する変数.

  /* ウィンドウ表示 */
  int wid = HgOpen(WIDTH, HEIGHT);
  doubleLayer dlayers = HgWAddDoubleLayer(wid);

  /* ベースレイヤに背景（ステージ）を描画 */
  showStage(layerID);

  /* ゲーム開始 */
  HgSetEventMask(HG_KEY_DOWN | HG_TIMER_FIRE); // イベント種別（キー入力 |タイマ発火）
  HgSetIntervalTimer(INTERVAL); // タイマ発火の周期を設定.
  do{
    /* ダブルバッファリングに伴う処理 */
    layerID = HgLSwitch(&dlayers);

    /* 画面を(消去&描画)更新 */
    HgLClear(layerID);

    /* 自機 */
    HgWSetColor(layerID, HG_RED); // 赤色
    HgWSetFillColor(layerID, HG_RED);
    showPac((double)(me1.x-1) *A_GRID + (A_GRID/2.0),
            (double)(me1.y)   *A_GRID + (A_GRID/2.0) + STATUSBAR,
            me1.size,
            me1.direction,
            openflag,
            layerID); //自機

    /* イベント取得 */
    ev = HgEventNonBlocking();
    if(ev != NULL){
      switch(ev->type){
        case HG_KEY_DOWN:
          ch = ev->ch;
	        if(ch != ESCKey){ //ESC押す
            move(ev);
            break;
          }
        case HG_TIMER_FIRE:
          /* 秒数（整数）の処理 */
          countTimer++;
          if(countTimer % (int)(1.0/INTERVAL) == 0){
            seconds++;
          }
          if ((countTimer % 2) == 0){
            openflag = !openflag;
          }
          /* 秒数（小数）の処理 */
          passedTime += INTERVAL;
          break;
      }
    }
  } while(ch != ESCKey);

  /* ゲーム終了 */
  showEnding(ending); // エンディングフラグに応じた表示をする関数.

  /* 終了動作 */
  HgGetChar();
  HgClose();

  /* 終了コード */
  return EXIT_SUCCESS;
}
