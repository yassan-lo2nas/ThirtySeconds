/*
 ドットエディタ
 白,黒,に赤色を追加した
 課題の条件にはないが、終了ボックスも表示した
*/

#include <stdio.h>
#include <handy.h>
#define SIZE 400.0
#define EXIT 5.0
#define SPLIT 16//およそ60以上にすると1回の処理に約1[s]かかる

int main() {
  hgevent *event;
  double x, y;
  int i, j, color[SPLIT][SPLIT]={{0}};

  HgOpen(SIZE, SIZE);
  HgSetWidth(1.0);

  HgSetColor(HG_RED);
  HgSetFillColor(HG_RED);
  HgBoxFill(0.0, 0.0, EXIT, EXIT, 1);

  HgSetEventMask(HG_MOUSE_DOWN);
  for(;;) {
    event = HgEvent();
    if (event->type == HG_MOUSE_DOWN) {
      x=event->x;
      y=event->y;

      /*終了条件のボックス内をクリックしたら、終了*/
      if ((0.0 <= event->x && event->x <= EXIT) &&
	  (0.0 <= event->y && event->y <= EXIT)) {
	break;
      }
      /*クリックされた領域の配列の中身変化*/
      for (i=0; i<SPLIT; i++) {
	for (j=0; j<SPLIT; j++) {
	  if((SIZE/(double)SPLIT *(double)i< event->x &&
	      event->x <=SIZE/(double)SPLIT *((double)(i+1)))
	     &&
	     (SIZE/(double)SPLIT *(double)j< event->y &&
	      event->y <=SIZE/(double)SPLIT *((double)(j+1)))){
	       if (color[i][j]==0) {
		 color[i][j]=1;
	       } else if(color[i][j]==1) {
		 color[i][j]=2;
	       } else {
		 color[i][j]=0;
	       }
	  }
	}
      }
      /*ボックス作成*/
      HgSetColor(HG_WHITE);//外縁は白
      for (i = 0; i < SPLIT; i++) {
	for (j= 0; j < SPLIT; j++) {
	  switch (color[i][j]) {
	  case 0:
	    HgSetFillColor(HG_WHITE);
	    break;
	  case 1:
	    HgSetFillColor(HG_BLACK);
	    break;
	  case 2:
	    HgSetFillColor(HG_RED);
	  break;
	  }
	  HgBoxFill(SIZE/(double)SPLIT * (double)i,
		    SIZE/(double)SPLIT * (double)j,
		    SIZE/(double)SPLIT,
		    SIZE/(double)SPLIT,
		    1);
	}
      }
      /*上のボックス表示するたびに、終了条件ボックス表示*/
      HgSetColor(HG_WHITE);
      HgSetFillColor(HG_WHITE);
      HgBoxFill(0.0, 0.0, EXIT, EXIT, 1);
    }
  }


  /* 終了動作 */
  HgSave("icon.png");
  HgGetChar();
  HgClose();
  return 0;
}
