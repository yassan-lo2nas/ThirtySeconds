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
  また、ゴールするか、敵に追いつかれた時点でゲーム終了, 何かのキーを押すとプログラム終了

  3.制限事項,不具合点


 */

// 更新情報
// 「ver.0」（2018/01/05 ??:??）スケルトンを作成.
// 「ver.1」（2018/01/05 ??:??）試行錯誤し、HGWINDOW関連などの準備.
// 「ver.2」（2018/01/05 9:51 ）ESCKeyのキーイベントに対する終了条件を追加
// 「ver.3」（2018/01/05 10:10）リファクタリング. クリア条件をマクロに定義したりswitch文導入
// 「ver.4」（2018/01/05 11:10）自機を表示, かつ入力に対して, 方向も変更可能.
//                             移動可能条件と, 移動可能床, 終了条件等を追加していこう.
// 「ver.5」（2018/01/05 23:29）アイコンを「./img/dot.c」にて作成し、色ではなくドット絵の
//                           　アイコンでオブジェクトを表現
// 「ver.6」
// ~「ver.8」（2018/01/06 16:44）色々変更.


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
#define A_GRID 70.0 // １マスの幅（高さ）.
                    // これを変更すると見え方全体に支障を来してしまう.
                    // なぜなら, 画像の縮尺にて, 4.5の意味がわかっていないから.
#define STATUSBAR (A_GRID - 20.0)  // ゲームのステージの下に付随する情報の欄.
#define WIDTH ((double)GRID * A_GRID) // ウィンドウの幅.
#define HEIGHT (WIDTH + STATUSBAR) // ウィンドウの高さ.
/* キー操作に必要なマクロ定義.（一部は ライブラリ HG ~~ の中にあるのでそれを用いる.）*/
#define ESCKey 0x1B // ESCKeyのアスキーコード.
/* 終了条件に用いるマクロ定義. */
#define CONTINUE    0 // ゲーム続行.
#define GAMEOVER    1 // ゲーム終了条件, 死亡.
#define GAMECLEAR   2 // ゲーム終了条件, 成功.
#define INTERRUPTED 3 // ゲーム終了条件, 中断.
/* タイマイベント, および, 敵の追跡等に用いるマクロ. */
#define INTERVAL 0.25 // タイマイベントに使う周期.(波で言う f = 1/T のT)
#define TIMES_OF_SECOND (int)(1.0/INTERVAL) // １秒の単位.１秒につき, INTERVAL何回か
#define CHASE_NUM  2 // 敵が追いかけ始める操作回数.
#define CHASE_TIME 2.0 // ２秒毎に１マス追跡.
#define MAX_SECONDS 30 // 残り時間の最大値.
#define MAX_LIFE    30 // 残りライフの最大値.

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

// ステージの初期情報をグローバル変数の２次変数で保持（初期状態）.
int stage[GRID][GRID] = {
  {10, 10, 10, 10, 10, 10, 10},
  {10,  0,  0,  7, 10,  4, 10},
  {10,  0, 10,  5, 10,  0, 10},
  {10,  0, 10,  0, 10,  0, 10},
  {10,  0, 10,  0, 10,  0, 10},
  {10,  3, 10,  6,  0,  8, 10},
  {10, 10, 10, 10, 10, 10, 10}
};
// 流動するステージ情報を保持.
int nowStage[GRID][GRID] = {
  {10, 10, 10, 10, 10, 10, 10},
  {10,  0,  0,  7, 10,  4, 10},
  {10,  0, 10,  5, 10,  0, 10},
  {10,  0, 10,  0, 10,  0, 10},
  {10,  0, 10,  0, 10,  0, 10},
  {10,  3, 10,  6,  0,  8, 10},
  {10, 10, 10, 10, 10, 10, 10}
};

int ending = CONTINUE;//終了フラグ（0:続行, 1:game over, 2:game clear, 3:中断）
int passedSeconds = 0;
/*---------------------------------------->8----------------------------------*/

/* 登場人物の情報を構造体で定義 */
typedef struct Me{
  int x, y;
  double size; // xは左から 2列目, yは下から 0列目のマスにいる（初期）という意味合い.
  int direction;//向きの判定(0~3, 0:上向き)
  int life;
}Me;
struct Me me    = {2, 6, A_GRID/2.0, 0, 30}; // {j, i, radius, flag}
                                         //；i行j列に存在する半径rの円（自分）
struct Me enemy = {2, 6, A_GRID/2.0, 0,  0}; // {j, i, radius, flag}
                                         //；i行j列に存在する半径rの円（敵）

/*---------------------------------------->8----------------------------------*/
/* View周りの関数群. */

void showStatusBar(int layerID/*, int passedSeconds, int life*/){ // StatueBar 部分を表示する関数
  int leftTime = 30 - passedSeconds;
  char buf[100];
  sprintf(&buf[0],"%02d",leftTime); // 数値を文字列に変換
  printf("残り時間は ... %s\n",&buf[0]); // 文字列を表示
  if (leftTime == 0){
    ending = GAMEOVER;
  }

  //HgWSetFillColor(layerID, HG_WHITE);
  // TIME側
  HgWSetColor(layerID, HG_WHITE);
  HgWSetFillColor(layerID, HG_BLACK);
  HgWBoxFill(layerID,
         0        ,         0,
         WIDTH/2.0, STATUSBAR,
         1);

  // --------- ここに TIME を表示する処理を書く必要 ------------
  //

  HgWSetFont(layerID, HG_G, (int)STATUSBAR-20); // （フォント, 太さ）を予め指定.
  HgWSetColor(layerID, HG_GREEN);
  HgWText(layerID, WIDTH/20.0 - 2.0, 2.0, , 30);
  HgWSetColor(layerID, HG_WHITE);
  HgWText(layerID, WIDTH/20.0 - 2.0, 2.0, , 30);
  HgWSetColor(layerID, HG_GREEN);
  HgWText(layerID, WIDTH/20.0 - 2.0, 2.0, , 30);
  // ------------------------------------------------------

  // LIFE側
  HgWSetColor(layerID, HG_WHITE);
  HgWSetFillColor(layerID, HG_BLACK);
  HgWBoxFill(layerID,
         WIDTH/2.0,         0,
         WIDTH/2.0, STATUSBAR,
         1);

  // --------- ここに LIFE を表示する処理を書く必要 ------------
  HgWSetFont(layerID, HG_G, (int)STATUSBAR-20); // （フォント, 太さ）を予め指定.
  HgWSetColor(layerID, HG_RED);
  HgWText(layerID, WIDTH/20.0 + WIDTH/2.0 -2.0, 2.0, "LIFE（30 /30）", 30);
  HgWSetColor(layerID, HG_WHITE);
  HgWText(layerID, WIDTH/20.0 + WIDTH/2.0 -1.0, 1.0, "LIFE（30 /30）", 30);
  HgWSetColor(layerID, HG_RED);
  HgWText(layerID, WIDTH/20.0 + WIDTH/2.0     , 0.0, "LIFE（30 /30）", 30);
  // ------------------------------------------------------

  return;
}

void showStage(int layerID, int Stage[GRID][GRID]){ // ステージを表示する関数.
                                                    // この後に自機→敵機と表示してやる必要有.
                                                    // あえて, 引数にローカルな変数としてStageを用意している.
  int pic;
  double w, h;
  for(int i=0; i<GRID; i++){
    for(int j=0; j<GRID; j++){
      //printf("%d, %d : %d\n", i,j, Stage[i][j]); //確認用
      // マスのタイプに応じた操作
      switch (Stage[i][j]) {
        case WALL:
          HgWSetFillColor(layerID, HG_BLACK);
          pic = HgImageLoad("./img/WALL.png");
          break;
        case EMPTY:
          HgWSetFillColor(layerID, HG_WHITE);
          pic = HgImageLoad("./img/EMPTY.png");
          break;
        case ME:
          HgWSetFillColor(layerID, HG_RED);
          // 必要無いかも・・・
          break;
        case ENEMY:
          HgWSetFillColor(layerID, HG_BLUE);
          // 必要無いかも・・・
          break;
        case START:
          HgWSetFillColor(layerID, HG_GREEN);
          pic = HgImageLoad("./img/START.png");
          break;
        case GOAL:
          HgWSetFillColor(layerID, HG_YELLOW);
          pic = HgImageLoad("./img/GOAL.png");
          break;
        case CLOCK:
          HgWSetFillColor(layerID, HG_PURPLE);
          pic = HgImageLoad("./img/CLOCK.png");
          break;
        case STOP:
          HgWSetFillColor(layerID, HG_ORANGE);
          pic = HgImageLoad("./img/STOP.png");
          break;
        case FLIP:
          HgWSetFillColor(layerID, HG_BROWN);
          pic = HgImageLoad("./img/FLIP.png");
          break;
        case FLOP:
          HgWSetFillColor(layerID, HG_WHITE);
          pic = HgImageLoad("./img/FLOP.png");
          break;

        default:
          break;
      }
      // 以下６行は色付きのボックスで表示していたレガシー. 消しても良い.
      // マスを表示 （layerID, x, y, 幅, 高さ, stk）
      // （追記）stkってなんぞ？
      //HgWBoxFill(layerID,
      //            A_GRID*j, HEIGHT-(A_GRID*(i+1)),
      //            A_GRID,   A_GRID,
      //            0);

      // 読み込んだイメージ pic（ID）に応じた表示操作.
      HgImageSize(pic, &w, &h); //不要(?)
      HgWImagePut(layerID,
                  A_GRID*j +(A_GRID/2.0), HEIGHT-(A_GRID*(i+1)) +(A_GRID/2.0),
                  pic,
                  (w/GRID)/A_GRID/4.5, /* scale(縮尺あわせ), なぜ4.5がちょうどいい？ */
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
  if(ending == CONTINUE){ending = INTERRUPTED;}
  switch(ending){
    case INTERRUPTED:
      {char *say = "say ゲームを終了します";
        system(say);}
      printf("[Interrupted]\n");
      HgSetColor(HG_BLACK);
      HgText(WIDTH/4.0 +3.0, HEIGHT/2.0 -3.0, "Interrupted", 30);
      HgSetColor(HG_WHITE);
      HgText(WIDTH/4.0 +2.0, HEIGHT/2.0 -2.0, "Interrupted", 30);
      HgSetColor(HG_BLUE);
      HgText(WIDTH/4.0     , HEIGHT/2.0     , "Interrupted", 20);
      break;
    case GAMEOVER:
      printf("[Game Over！]\n");
      {char *say = "say その時、世界は闇に覆われた ";
        system(say);}
      HgSetColor(HG_BLACK);
      HgText(WIDTH/5.0 +3.0, HEIGHT/2.0 -3.0, "Game Over！", 30);
      HgSetColor(HG_WHITE);
      HgText(WIDTH/5.0 +2.0, HEIGHT/2.0 -2.0, "Game Over！", 30);
      HgSetColor(HG_RED);
      HgText(WIDTH/5.0     , HEIGHT/2.0     , "Game Over！", 20);
      break;
    case GAMECLEAR:
      printf("[Game Clear！]\n");
      HgSetColor(HG_BLACK);
      HgText(WIDTH/5.0 +3.0, HEIGHT/2.0 -3.0, "Game Clear！", 30);
      HgSetColor(HG_WHITE);
      HgText(WIDTH/5.0 +2.0, HEIGHT/2.0 -2.0, "Game Clear！", 30);
      HgSetColor(HG_GREEN);
      HgText(WIDTH/5.0     , HEIGHT/2.0     , "Game Clear！", 20);
    default: break;
  }
  return;
}

/*---------------------------------------->8----------------------------------*/
/* 自機操作に関する関数群 */

void showPac(double x, double y, double size, int direction, bool openflag, int layerID){//自機,敵機を表示する関数
  switch(direction){
    case 0:
      if(openflag==0)HgWFanFill(layerID, x, y, size, 0.75*M_PI, 2.25*M_PI, 0); //上向き
      else HgWFanFill(layerID, x, y, size, 0.51*M_PI, 2.50*M_PI, 0); //上向き(口閉)
      break;
    case 1:
      if(openflag==0)HgWFanFill(layerID, x, y, size, 0.25*M_PI, 1.75*M_PI, 0); //右向き
      else HgWFanFill(layerID, x, y, size, 0.01*M_PI, 2.00*M_PI, 0); //右向き(口閉)
      break;
    case 2:
      if(openflag==0)HgWFanFill(layerID, x, y, size, 1.75*M_PI, 3.25*M_PI, 0); //下向き
      else HgWFanFill(layerID, x, y, size, 1.51*M_PI, 3.50*M_PI, 0); //下向き(口閉)
      break;
    case 3:
      if(openflag==0)HgWFanFill(layerID, x, y, size, 1.25*M_PI, 0.75*M_PI, 0);//左向き
      else HgWFanFill(layerID, x, y, size, 1.01*M_PI, 3.00*M_PI, 0);//左向き(口閉)
      break;
  }
}

void move(hgevent *ev, int layerID){
  // ローカル変数に代入し、一時的に使用 （→ 不要なら消す）
  int x = me.x -1;
  int y = me.y -1;
  // 移動先のマスの情報, 一時的変数, 条件に使用
  int nextGRID = 0;
  //自機の座標移動
  switch (ev->ch) {//キー操作が以下の場合
    case HG_U_ARROW:
      me.direction = 0;
      // --------ここにキー押下に対する操作を書く-----------
      // 上下（yに対する演算）が逆になることに注意.
      // これは, 表示は左下基準.配列は左上基準となることに起因.
      if(y-1 < 0){break;} // マップ外は移動出来ないように除く.
      nextGRID = nowStage[y-1][x]; // 移動先の情報を代入
      if(nextGRID == WALL){printf("壁があるので進めません。\n");break;}
      else if(nextGRID == FLOP){printf("FLOPがあるので進めません。\n");break;}
      else{me.y-=1;}
      // ----------------------------------------------
      break;
    case HG_R_ARROW:
      me.direction = 1;
      // --------ここにキー押下に対する操作を書く-----------
      //
      if(x+1 > GRID){break;} // マップ外は移動出来ないように除く.
      nextGRID = nowStage[y][x+1]; // 移動先の情報を代入
      if(nextGRID == WALL){printf("壁があるので進めません。\n");break;}
      else if(nextGRID == FLOP){printf("FLOPがあるので進めません。\n");break;}
      else {me.x+=1;}
      // ----------------------------------------------
      break;
    case HG_D_ARROW:
      me.direction = 2;
      // --------ここにキー押下に対する操作を書く-----------
      //
      if(y+1 > GRID){break;} // マップ外は移動出来ないように除く.
      nextGRID = nowStage[y+1][x]; // 移動先の情報を代入
      if(nextGRID == WALL){printf("壁があるので進めません。\n");break;}
      else if(nextGRID == FLOP){printf("FLOPがあるので進めません。\n");break;}
      else{me.y+=1;} //以上の条件でなければ, 移動.
                     //移動不可能の条件判定を関数化してもよいかも.
      // ----------------------------------------------
      break;
    case HG_L_ARROW:
      me.direction = 3;
      // --------ここにキー押下に対する操作を書く-----------
      //
      if(x-1 < 0){break;} // マップ外は移動出来ないように除く.
      nextGRID = nowStage[y][x-1]; // 移動先の情報を代入
      if(nextGRID == WALL){printf("壁があるので進めません。\n");break;}
      else if(nextGRID == FLOP){printf("FLOPがあるので進めません。\n");break;}
      else{me.x-=1;}
      // ----------------------------------------------
      break;

    /*--- キーボード入力によるおまけ機能の追加 ----*/
    case 'v':
      ending = GAMEOVER;
      break;
    case 'c':
      {char *say = "say 勇者はチイトをしてクリアアした";
        system(say);}
      {char *say = "say どんどんぱふぱふうう";
        system(say);}
      ending = GAMECLEAR;
      break;
    case 'k':
      {char *say = "say かなやまはチイトをしてクリアアした";
        system(say);}
      ending = GAMEOVER;
      break;
    default:
      break;
  }

  /* 移動後の判定, ここから, 終わりまで関数化したほうが良い */
  printf("[自機]：(%d,%d), %d\n", me.x, GRID-me.y+1, nowStage[me.y-1][me.x-1]); //確認用
  if(nowStage[me.y-1][me.x-1] == CLOCK ||
     nowStage[me.y-1][me.x-1] == STOP)
  { //CLOCKorSTOPを踏んだ時の処理. 関数化してもいいかも.
    if(nowStage[me.y-1][me.x-1] == CLOCK){
      printf("時計ゲット!１０秒追加\n");
      /*　ここにタイマーを10秒増やす処理　 */
    }
    if(nowStage[me.y-1][me.x-1] == STOP){
      printf("STOPゲット！敵を５秒止める\n");
      /*　ここにタイマーを10秒増やす処理　 */
    }
    int pic = 0;
    double w, h;
    nowStage[me.y-1][me.x-1] = EMPTY;
    pic = HgImageLoad("./img/EMPTY.png");
    HgImageSize(pic, &w, &h); //不要(?)
    HgWImagePut(0,
                A_GRID*(me.x-1) +(A_GRID/2.0),
                HEIGHT-(A_GRID*(me.y-1+1)) +(A_GRID/2.0) +1,
                pic,
                (w/GRID)/A_GRID/4.5, /* scale(縮尺あわせ), なぜ4.5がちょうどいい？ */
                0);
  }
  // クリア判定.
  if(nowStage[me.y-1][me.x-1] == GOAL){

     ending = GAMECLEAR;
   }

  return;
}

/*---------------------------------------->8----------------------------------*/
/* main 関数記述*/

int main(void) {
  /* ローカル変数等の宣言 */
  int layerID = 0;
  hgevent* ev; //イベント取得のための変数宣言.
  int ch = 0; //キー入力に対応する整数値
  int countTimer = 0; // カウンタが何回発火したかを数える（整数）.
  //int passedSeconds = 0; // 経過秒数（整数）.→ グローバル変数に
  bool openflag = true; // bool={true: open, false:close} のフラグ.
                        // パックマンの口開閉のフラグ.
  double passedTime = 0; // 経過時間を保存する変数.

  // nowStageもグローバル変数で持っておいたほうが便利なので以降, 以下はレガシー.
  //int nowStage[GRID][GRID] = {{0}}; // 現在のステージ情報をローカル変数の２次変数で保持
  //                                  //（リアルタイムに変動）.
  //for(int i=0; i<GRID; i++){        // not shallow copy（deep copy）
  //  for(int j=0; j<GRID; j++){
  //    nowStage[i][j] = stage[i][j];
  //  }
  //}
  printf("[自機]：(%d,%d)\n", me.x, GRID-me.y+1); //確認用

  /* ウィンドウ表示 */
  int wid = HgOpen(WIDTH, HEIGHT);
  doubleLayer dlayers = HgWAddDoubleLayer(wid);

  /* ベースレイヤに背景（ステージ）を描画 */
  showStage(0, stage);

  /* ゲーム開始 */
  HgSetEventMask(HG_KEY_DOWN | HG_TIMER_FIRE); // イベント種別（キー入力 |タイマ発火）
  HgSetIntervalTimer(INTERVAL); // タイマ発火の周期を設定.
  do{
    /* 終了条件になれば break */
    if(ending!=0) {break;}

    /* ダブルバッファリングに伴う処理 */
    layerID = HgLSwitch(&dlayers);
    /* 画面を(消去&描画)更新 */
    HgLClear(layerID);

    /* ここ関数化ポイント */
    int pic = 0;
    double w,h;
    for(int i=0; i<GRID; i++){
      for(int j=0; j<GRID; j++){
        if(nowStage[i][j] == 7){
          HgWSetFillColor(layerID, HG_BROWN);
          pic = HgImageLoad("./img/FLIP.png");
          if (pic < 0){break;}
          HgImageSize(pic, &w, &h); //不要(?)
          HgWImagePut(layerID,
                      A_GRID*j +(A_GRID/2.0), HEIGHT-(A_GRID*(i+1)) +(A_GRID/2.0),
                      pic,
                      (w/GRID)/A_GRID/4.5, /* scale(縮尺あわせ), なぜ4.5がちょうどいい？ */
                      0);
        } else if(nowStage[i][j] == 8) { // ここは絶対にわけてはいけない条件
          HgWSetFillColor(layerID, HG_BROWN);
          pic = HgImageLoad("./img/FLOP.png");
          if (pic < 0){break;}
          HgImageSize(pic, &w, &h); //不要(?)
          HgWImagePut(layerID,
                      A_GRID*j +(A_GRID/2.0), HEIGHT-(A_GRID*(i+1)) +(A_GRID/2.0),
                      pic,
                      (w/GRID)/A_GRID/4.5, /* scale(縮尺あわせ), なぜ4.5がちょうどいい？ */
                      0);
        }
      }
    }
    /* ----関数化推奨---- */

    /* 自機 */
    HgWSetColor(layerID, HG_RED); // 赤色
    HgWSetFillColor(layerID, HG_RED);
    showPac((double)(me.x-1) *A_GRID + (A_GRID/2.0),
            (GRID-(double)(me.y)) *A_GRID + (A_GRID/2.0) + STATUSBAR,
            me.size,
            me.direction,
            openflag,
            layerID); //自機
    /* 敵機 */
    // -----------ここに敵機に関する操作を書く------------
    // // おそらく, ２回以上操作したら動き出すということと,
    // // ２秒毎に動くという条件付きの操作が必要.
    //
    // ----------------------------------------------

    /* イベント取得 */
    ev = HgEventNonBlocking();
    if(ev != NULL){
      switch(ev->type){
        case HG_KEY_DOWN:
          ch = ev->ch;
	        if(ch != ESCKey){ //ESC押す
            move(ev, layerID);
            break;
          }
        case HG_TIMER_FIRE:
          /* 秒数（整数）の処理 */
          countTimer++;
          if(countTimer % TIMES_OF_SECOND == 0){//(counter*INTERVAL)=1.0[s]なら
            //printf("%d\n", passedSeconds); // 確認用
            passedSeconds++;
            /* 関数化してもいいかも（開始） */
            for(int i=0; i<GRID; i++){ // 偶数秒ごとに床を切り替え, 関数化してもいいかも.
              for(int j=0; j<GRID; j++){
                if(nowStage[i][j] == 7){
                  nowStage[i][j] = 8;
                  /* このタイミングで, 自機が床を踏んでたらGAME OVER */
                  if(nowStage[me.y-1][me.x-1] == nowStage[i][j]){
                    ending = GAMEOVER;
                  }
                } else if(nowStage[i][j] == 8) { //※ここはifを絶対にわけてはいけない.
                  nowStage[i][j] = 7;
                }
                printf("%d, ", nowStage[i][j]); // 確認用 消しても良い
              }
              printf("\n"); // 確認用 消しても良い
            }
            /* 関数化してもいいかも（終了） */

            // --------ここにSTATUSBARのための操作を書く---------
            //
            showStatusBar(layerID);
            // ----------------------------------------------
          }
          if ((countTimer % 2) == 0){ // counterが２の倍数の時に口パク.
            openflag = !openflag;
          }
          /* 秒数（小数）の処理 */
          passedTime += INTERVAL; // 浮動小数点の単位で, 経過秒数を数える（INTERVAL毎に）
          break;

      }
    }
    HgSleep(0.02);
  } while(ch != ESCKey);

  /* ゲーム終了 */
  HgWSetColor(layerID, HG_RED); // 赤色
  HgWSetFillColor(layerID, HG_RED);
  showPac((double)(me.x-1) *A_GRID + (A_GRID/2.0),
          (GRID-(double)(me.y)) *A_GRID + (A_GRID/2.0) + STATUSBAR,
          me.size,
          me.direction,
          openflag,
          layerID); //自機
  showEnding(ending); // エンディングフラグに応じた表示をする関数.

  /* 終了動作 */
  HgGetChar();
  HgClose();

  /* 終了コード */
  return EXIT_SUCCESS;
}
