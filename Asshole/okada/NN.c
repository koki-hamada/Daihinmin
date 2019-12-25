/*---------------------------------------------------------*/
/*                        NN.c                             */
/*      　バックプロパゲーションによるニューラルネット 　      */
/*---------------------------------------------------------*/

//ヘッダファイルのインクルード
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "NN.h"

/*-----------------------------*/
//IN[100]OUT[53]説明
//入力数100,出力数53
//IN[0~52].....提出プレイヤが所持する手札。[52]はジョーカー
//IN[53~57]....各プレイヤ階級(入力時に正規化)
//IN[58~62]....各プレイヤ座席(同じく正規化)
//IN[63~67]....各プレイヤ手札枚数(同じく正規化)
//IN[68~71]....しばり(全て0でしばり無し)
//IN[72].......革命
//IN[73~78]....場の枚数([73]=1で空場,[74]=1で一枚といった具合)
//IN[79~82]....場の役(0で空場,1で単体,2でペア,3で階段)
//IN[83~95]....場の強さ(3が最弱なので1として格納する)
//IN[96~99]....場のスート
//OUT[100~53]...教師データ(提出したカード情報)[152]=1でジョーカー単体,[153]=1でパス
/*-----------------------------*/

/*----------------*/
/*     NN()関数    */
/*----------------*/
void NN(double IN[INPUTNO],double OUT[OUTPUTNO],double wh[HIDDENNO][INPUTNO+1],double wo[OUTPUTNO][HIDDENNO+1]){
  double hi[HIDDENNO+1];//中間層の出力
  int i,j;//繰り返しの制御

//  printweight(wh,wo);//重みの出力

  /*i個の出力層に対応*/
  for(i=0;i<OUTPUTNO;++i){
    OUT[i]=forward(wh,wo[i],hi,IN);//順方向の計算
  }
  /*
  for(i=0;i<54;i++){
    fprintf(stderr,"%lf\n",OUT[i]);
    }*/
}

/*-------------------*/
/*  forward()関数     */
/*  順方向の計算       */
/*-------------------*/
double forward(double wh[HIDDENNO][INPUTNO+1],double wo[HIDDENNO+1],double hi[],double e[100]){
  int i,j;//繰り返しの制御
  double u;//重み付き和の計算
  double o;//出力の計算

  /*hiの計算*/
  for(i=0;i<HIDDENNO;++i){
    u=0;//重み付き和を求める
    for(j=0;j<INPUTNO;++j)
      u+=e[j]*wh[i][j];
    u-=wh[i][j];//しきい値の処理
    hi[i]=s(u);
  }
   /*出力oの計算*/
  o=0;
  for(i=0;i<HIDDENNO;++i) o+=hi[i]*wo[i];
  o-=wo[i];//しきい値の処理
  return s(o);
}

/*--------------------*/
/*  printweight()関数 */
/*   結果の出力        */
/*-------------------*/
void printweight(double wh[HIDDENNO][INPUTNO+1],double wo[OUTPUTNO][HIDDENNO+1]){
  int i,j;//繰り返しの制御

  for(i=0;i<HIDDENNO;++i)
    for(j=0;j<INPUTNO+1;++j)
      printf("%lf ",wh[i][j]);
    printf("\n");
  for(i=0;i<OUTPUTNO;++i){
    for(j=0;j<HIDDENNO+1;++j)
      printf("%lf ",wo[i][j]);
  }
  printf("\n");
}

/*------------------*/
/* s()関数          */
/* シグモイド関数     */
/*-----------------*/
double s(double u){
  return 1.0/(1.0+exp(-u));
}
