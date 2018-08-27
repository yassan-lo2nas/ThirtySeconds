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
#define ESC 0x1B


/*---------------------------------------->8----------------------------------*/

// ステージ情報のenum型を定義
// typedef enum Type{EMPTY=0, ME, ENEMY, START, GOAL, CLOCK, STOP, FLIP, FLOP, WALL=10}Type;
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
struct Me me1 = {2, 0, A_GRID/2.0, 0};// {x, y, size, flag}

/*---------------------------------------->8----------------------------------*/
void showStatusBar(int lid){
  HgWSetColor(lid, HG_BLACK);
  //HgWSetFillColor(lid, HG_WHITE);
  // TIME側
  HgWBox(lid,
         0,          0,
         HEIGHT/2.0, STATUSBAR
         );
  // LIFE側
  HgWBox(lid,
         HEIGHT/2.0, 0,
         HEIGHT,     STATUSBAR
         );
  return;
}

void showStage(int lid){ // ステージを表示, この後に自機→敵機と表示してやる必要有.
  for(int i=0; i<GRID; i++){
    for(int j=0; j<GRID; j++){
      printf("%d, %d : %d\n", i,j,stage[i][j]); //確認用
      // マスのタイプに応じた操作
      switch (stage[i][j]) {
        case WALL:
          HgWSetFillColor(lid, HG_BLACK);
          break;
        case EMPTY:
          HgWSetFillColor(lid, HG_WHITE);
          break;
        case ME:
          HgWSetFillColor(lid, HG_RED);
          break;
        case ENEMY:
          HgWSetFillColor(lid, HG_BLUE);
          break;
        case START:
          HgWSetFillColor(lid, HG_GREEN);
          break;
        case GOAL:
          HgWSetFillColor(lid, HG_YELLOW);
          break;
        case CLOCK:
          HgWSetFillColor(lid, HG_PURPLE);
          break;
        case STOP:
          HgWSetFillColor(lid, HG_ORANGE);
          break;
        case FLIP:
          HgWSetFillColor(lid, HG_BROWN);
          break;
        case FLOP:
          HgWSetFillColor(lid, HG_WHITE);
          break;

        default:
          break;
      }
      // マスを表示
      HgWBoxFill(lid,
                  A_GRID*j, HEIGHT-(A_GRID*(i+1)),
                  A_GRID,   A_GRID,
                  0);
    }
  }
  showStatusBar(lid);
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
  int lid = 0;
  hgevent* ev;//イベント取得
  int wid = HgOpen(WIDTH, HEIGHT);
  doubleLayer dlayers = HgWAddDoubleLayer(wid);
  int ch = 0;//キー入力に対応する整数値

  showStage(lid);

  HgGetChar();
  HgClose();
  return 0;
}
