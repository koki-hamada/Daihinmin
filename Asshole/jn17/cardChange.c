/* cardChange.c : カード交換に出すカードを決定する */ 
/* Author       : Kengo Matsuta                    */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "cardChange.h"

//カードの中身がどんな組み合わせになっているか調べる
void checkCards(int select_cards[8][15], int my_cards[8][15], int select_qty){
  int kaidan_buf[8][15];
  int pair_buf[8][15];
  int my_cards_buf[8][15];
  int temp_cards[8][15];

  int i,j,k;
  int count, noJcount;

  for(i=0;i<8;i++){
    for(j=0;j<15;j++){
      kaidan_buf[i][j] = 0;
      pair_buf[i][j] = 0;
      select_cards[i][j] = 0;
      temp_cards[i][j] = 0;
      my_cards_buf[i][j] = my_cards[i][j];
    }
  }


  for(i=0;i<4;i++){            //各スート毎に走査し
    count=1;
    noJcount=0; 
    for(j=13;j>=0;j--){        //順番にみて
      if(my_cards_buf[i][j]==1){   //カードがあるとき
	count++;               //2つのカウンタを進める
	noJcount++;
      }
      else{                    //カードがないとき
	count=noJcount+1;      //ジョーカーありの階段の枚数にジョーカー分を足す 
	noJcount=0;            //ジョーカーなしの階段の枚数をリセットする
      }
      if((my_cards_buf[4][1]==2 ? count : noJcount) > 2){              //3枚以上のとき
	kaidan_buf[i][j]=(my_cards_buf[4][1]==2 ? count : noJcount);  //その枚数をテーブルに格納
      }
      else{
	kaidan_buf[i][j]=0;      //その他は0にする
      }
    }
  }

  int kaidan, joker_flag;

  //弱いほうの手札を優先して階段で消費するためにj->iの順でループ(ジョーカーを考えたとき)
  for(j=0;j<15;j++){
    for(i=0;i<5;i++){
      if(kaidan_buf[i][j]!=0){
	joker_flag=0;
	kaidan = kaidan_buf[i][j];
	for(k=j;k<j+kaidan;k++){
	  if(my_cards_buf[i][k]==0){
	    if(my_cards_buf[4][1]==2 && joker_flag==0){
	      //ジョーカー使用
	      joker_flag=1;
	    }else{
	      //ジョーカー使用済みのときは階段フラグをなかったことにする
	      kaidan_buf[i][j] = 0;
	    }
	  }
	}
	if(kaidan_buf[i][j]!=0){
	  if(joker_flag){	//ジョーカー使用
	    for(k=j;k<j+kaidan;k++){
	      my_cards_buf[i][k]=0;
	      kaidan_buf[i][j]=1;
	    }
	    my_cards_buf[4][1]=0;
	  }else{	//ジョーカー無しで階段が成立
	    for(k=j;k<j+kaidan;k++){
	      my_cards_buf[i][k]=0;
	      kaidan_buf[i][k]=1;
	    }
	  }
	}
      }
    }
  }

  //ここからペア
  for(i=1;i<=13;i++){  //それそれの強さのカードの枚数を数え，ジョーカーがあればその分を加える
    count=my_cards_buf[0][i]+my_cards_buf[1][i]+my_cards_buf[2][i]+my_cards_buf[3][i]+(my_cards_buf[4][1]==2);
    if(count>1){      //枚数が2枚以上のとき
      for(j=0;j<4;j++){
	if(my_cards_buf[j][i]==1){    //カードを持っている部分に
	  pair_buf[j][i]=count; //その枚数を格納
	}
      }
    }
  }

  int pair;
  //弱いほうの手札を優先してペアで消費するためにj->iの順でループ(ジョーカーを考えたとき)
  for(j=0;j<15;j++){
    for(i=0;i<5;i++){
      if(pair_buf[i][j]!=0){
	pair = pair_buf[i][j];
	if(my_cards_buf[0][j]+my_cards_buf[1][j]+my_cards_buf[2][j]+my_cards_buf[3][j]==(pair-1) && my_cards_buf[4][1]==2 && pair>2){
	  for(k=0;k<4;k++){
	    if(my_cards_buf[k][j]!=0){
	      my_cards_buf[k][j]=0;
	      pair_buf[k][j]=1;
	    }
	  }
	  my_cards_buf[4][1]=0;
	}else if(my_cards_buf[0][j]+my_cards_buf[1][j]+my_cards_buf[2][j]+my_cards_buf[3][j]==pair){
	  for(k=0;k<4;k++){
	    if(my_cards_buf[k][j]!=0){
	      my_cards_buf[k][j]=0;
	      pair_buf[k][j]=1;
	    }
	  }
	}else{
	  pair_buf[i][j]=0;
	}
      }
    }
  }

  for(i=0;i<8;i++){
    for(j=0;j<15;j++){
      temp_cards[i][j] = my_cards_buf[i][j];
    }
  }

  for(i=0;i<4;i++){
    if(my_cards_buf[i][6]==1){
      //			printf("solo 8 card\n");
      my_cards_buf[i][6]=0;
    }
  }

  //ここに入る可能性はとても低いが
  if(my_cards_buf[4][1]==2){
    //		printf("solo joker\n");
    my_cards_buf[4][1]=0;
  }

  //革命対策
  for(i=0;i<4;i++){
    if(my_cards_buf[i][1]==1){
      //			printf("solo 3 card\n");
      my_cards_buf[i][1]=0;
    }
  }

  for(i=0;i<4;i++){
    for(j=9;j<15;j++){
      if(my_cards_buf[i][j]==1){
	//				printf("solo %d card\n", j+2);
	my_cards_buf[i][j]=0;
      }
    }
  }

  int remains = 0;
  count = 0;

  //残りの枚数チェック
  for(i=0;i<5;i++)
    for(j=0;j<15;j++)
      remains += (my_cards_buf[i][j] > 0);

  if(remains==select_qty){			//同じならそのままコピー
    for(i=0;i<8;i++){
      for(j=0;j<15;j++){
	select_cards[i][j] = my_cards_buf[i][j];
      }
    }
  }else if(remains > select_qty){		//残りの枚数のほうが交換枚数より多ければ小さいカードからテーブルに追加
    for(j=0;j<15;j++){
      if(count == select_qty) break;
      for(i=0;i<4;i++){
	if(count==select_qty)break;
	if(my_cards_buf[i][j]!=0){
	  select_cards[i][j]=1;
	  count++;
	}
      }
    }
  }else if(remains < select_qty){		//交換枚数に達していなければ、少しずつ崩していく
    for(i=0;i<8;i++){
      for(j=0;j<15;j++){
	select_cards[i][j] = my_cards_buf[i][j];
	if(my_cards_buf[i][j]==1) temp_cards[i][j] = 0;
      }
    }
    count = remains;
    for(i=0;i<4;i++){				//最初は単品の3を出す
      if(temp_cards[i][1]!=0){
	select_cards[i][1]=1;
	temp_cards[i][1]=0;
	count++;
      }
      if(count == select_qty) break;
    }
    if(count != select_qty){
      for(i=0;i<4;i++){		//次は単品の8を出す
	if(temp_cards[i][6]!=0){
	  select_cards[i][6]=1;
	  temp_cards[i][6]=0;
	  count++;
	}
	if(count == select_qty) break;
      }
    }
    if(count != select_qty){	//次は単品の11～12を出す
      for(j=9;j<11;j++){
	if(temp_cards[i][j]!=0){
	  select_cards[i][j]=1;
	  temp_cards[i][j]=0;
	  count++;
	}
	if(count == select_qty) break;
      }
    }
    if(count != select_qty){	//次はペアを崩す
      for(i=0;i<8;i++){
	for(j=0;j<15;j++){
	  if(pair_buf[i][j]>0) temp_cards[i][j] = 1;
	}
      }
      for(j=0;j<15;j++){
	for(i=0;i<4;i++){
	  if(temp_cards[i][j] != 0){
	    for(k=0;k<4;k++){
	      if(temp_cards[k][j]!=0){
		select_cards[k][j]=1;
		temp_cards[k][j]=0;
		count++;
	      }
	      if(count == select_qty) break;
	    }
	  }
	  if(count == select_qty) break;
	}
	if(count == select_qty) break;
      }
    }
    if(count != select_qty){	//最後に階段を崩す
      for(i=0;i<8;i++){
	for(j=0;j<15;j++){
	  if(pair_buf[i][j]>0) temp_cards[i][j] = 1;
	}
      }
      for(j=0;j<15;j++){
	for(i=0;i<4;i++){
	  if(temp_cards[i][j]!=0){
	    select_cards[i][j]=1;
	    temp_cards[i][j]=0;
	    count++;
	  }
	  if(count == select_qty) break;
	}
	if(count == select_qty) break;
      }
    }
  }
}


void checkCards2(int select_cards[8][15], int my_cards[8][15], int select_qty){
  int s, r;
  for(r=1; r<14; r++){
    for(s=0;s<4;s++){			
      if(my_cards[s][r]!=0){
	select_cards[s][r]=1;
        goto out;
      }
    }
  }
 out:
  return;
}
