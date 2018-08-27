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
// 「ver.2」（2018/01/05 9:51）ESCKeyのキーイベントに対する終了条件を追加

/*---------------------------------------->8----------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <handy.h>
#include <time.h>

#define GRID 7
#define STATUSBAR 50.0
#define WIDTH ((double)GRID * 70.0)
#define HEIGHT (WIDTH + STATUSBAR)
#define A_GRID (WIDTH/(double)GRID)
#define ESCKey 0x1B
#define INTERVAL 0.5

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
// 自機の情報を定義
typedef struct Me{
  double x, y, size; // xは左から 2列目, yは下から 0列目
  int flag;//向きの判定(0~3, 0:上向き)
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
  // LIFE側
  HgWBox(layerID,
         HEIGHT/2.0, 0,
         HEIGHT,     STATUSBAR
         );
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
      // マスを表示
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
  HgSetFont(HG_M, 50); // （フォント, 太さ指定）
  // 以下エンドフラグ別の文字列表示.
  // 敢えて, 似たような動作（HgText）を ifの外に出していない.
  // 条件毎の動作を個別に指定したい（後の拡張の）ため.
  if(ending == 2) {
    printf("[Game Clear！]\n");
    HgSetColor(HG_WHITE);
    HgText(WIDTH/5.0 +2, HEIGHT/2.0 -2, "Game Clear！", 30);
    HgSetColor(HG_ORANGE);
    HgText(WIDTH/5.0   , HEIGHT/2.0   , "Game Clear！", 20);
  }
  else if(ending == 1) {
    printf("[Game Over！]\n");
    HgSetColor(HG_WHITE);
    HgText(WIDTH/5.0 +2, HEIGHT/2.0 -2, "Game Over！", 30);
    HgSetColor(HG_RED);
    HgText(WIDTH/5.0   , HEIGHT/2.0   , "Game Over！", 20);
  }
  else {
    printf("[Interrupted]\n");
    HgSetColor(HG_WHITE);
    HgText(WIDTH/4.0 +2, HEIGHT/2.0 -2, "Interrupted", 30);
    HgSetColor(HG_BLUE);
    HgText(WIDTH/4.0   , HEIGHT/2.0   , "Interrupted", 20);
  }
  return;
}

void move (hgevent *ev){
  //自機の座標移動
  switch (ev->ch) {//キー操作が以下の場合
    case HG_U_ARROW:
      break;
    case HG_R_ARROW:
      break;
    case HG_D_ARROW:
      break;
    case HG_L_ARROW:
      break;
    default:
      break;
  }
  return;
}

/*---------------------------------------->8----------------------------------*/

int main(void) {
  int ending = 0;//終了フラグ（0:中断, 1:game over, 2:game clear）
  int layerID = 0;
  hgevent* ev;//イベント取得
  int wid = HgOpen(WIDTH, HEIGHT);
  doubleLayer dlayers = HgWAddDoubleLayer(wid);
  int ch = 0;//キー入力に対応する整数値

  /* ベースレイヤに背景（ステージ）を描画 */
  showStage(layerID);

  /* ゲーム開始 */
  HgSetEventMask(HG_KEY_DOWN | HG_TIMER_FIRE);//キー入力, タイマ発火
  HgSetIntervalTimer(INTERVAL); // タイマの設定
  do{
    layerID = HgLSwitch(&dlayers);
    HgLClear(layerID);// 画面を(消去&描画)更新
    /* イベント取得 */
    ev = HgEventNonBlocking();
    if(ev != NULL){
      switch(ev->type){
        case HG_KEY_DOWN:
	        if( (ch = ev->ch) != ESCKey){//ESC押す
            break;
          }
        case HG_TIMER_FIRE:
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
