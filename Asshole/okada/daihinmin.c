/*daifugo*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "daihinmin.h"

extern const int g_logging;

void getState(int cards[8][15]){
  /*
    カードテーブルから得られる情報を読み込む
    引数は手札のカードテーブル
    情報は広域変数stateに格納される
  */
  int i;
  //状態
  if(cards[5][4]>0) state.onset=1; //場にカードがないとき 1
  else              state.onset=0;
  if(cards[5][6]>0) state.rev=1;   //革命状態の時 1 
  else              state.rev=0;
  if(cards[5][5]>0) state.b11=1;   //11バック時 1 未使用
  else              state.b11=0;
  if(cards[5][7]>0) state.lock=1;  //しばり時 1
  else              state.lock=0;

  if(state.onset==1){   //新たな場のとき札の情報をリセット
    state.qty=0;
    state.ord=0;
    state.lock=0;
    for(i=0;i<5;i++)state.suit[i]=0;
  }
  
  for(i=0;i<5;i++) state.player_qty[i]=cards[6][i];   //手持ちのカード
  for(i=0;i<5;i++) state.player_rank[i]=cards[6][5+i];//各プレーヤのランク
  for(i=0;i<5;i++) state.seat[i]=cards[6][10+i];      //誰がどのシートに座っているか
                                                      //シートiにプレーヤ STATE.SEAT[I]が座っている

  if(cards[4][1]==2) state.joker=1;     //Jokerがある時 1
  else               state.joker=0;

  
}

void getField(int cards[8][15]){
  /*
    場に出たカードの情報を得る。
    引数は場に出たカードのテーブル
    情報は広域変数stateに格納される
  */
  int i,j,count=0;
  i=j=0;
  
  //カードのある位置を探す
  while(j<15&&cards[i][j]==0){
    state.suit[i]=0;
    i++;
    if(i==4){
      j++;
      i=0;
    }
  }
  //階段が否か
  if(j<14){
    if(cards[i][j+1]>0) state.sequence=1;
    else state.sequence=0;
  }
  //枚数を数える また強さを調べる
  if(state.sequence==0){
    //枚数組
    for(;i<5;i++){
      if(cards[i][j]>0){
	count++;
	state.suit[i]=1;
      }else{
	state.suit[i]=0;
      }
    }
    if(j==0||j==14){
      if(state.rev==0){
	state.ord=14;
      }else{
	state.ord=0;
      }
    }else{
      state.ord=j;
    }
  }else{
    //階段
    while(j+count<15 && cards[i][j+count]>0){
      count++;
    }
    if((state.rev==0 && state.b11==0 )||( state.rev==1 && state.b11==1 )){
      state.ord=j+count-1;
    }else{
      state.ord=j;
    }
    state.suit[i]=1;
    }
  //枚数を記憶
  state.qty=count;
 
  if(state.qty>0){ //枚数が0より大きいとき 新しい場のフラグを0にする
    state.onset=0;
  }
}


void showState(struct state_type *state){
  /*引数で渡された状態stateの内容を表示する*/
  int i;
  printf("state rev   : %d\n",state->rev);
  printf("state lock  : %d\n",state->lock);
  printf("state joker : %d\n",state->joker);
  
  printf("state qty   : %d\n",state->qty);
  printf("state ord   : %d\n",state->ord);
  printf("state seq   : %d\n",state->sequence);
  printf("state onset : %d\n",state->onset);
  printf("state suit :");
  for(i=0;i<4;i++)printf("%d ",state->suit[i]);
  printf("\n"); printf("state player qty :");
  for(i=0;i<5;i++)printf("%d ",state->player_qty[i]);
  printf("\n"); printf("state player rank :");
  for(i=0;i<5;i++)printf("%d ",state->player_rank[i]);
  printf("\n"); printf("state player_num on seat :");
  for(i=0;i<5;i++)printf("%d ",state->seat[i]);
  printf("\n");
}

//それぞれカードの和 共通 差分 逆転 をとる
void cardsOr(int cards1[8][15],int cards2[8][15]){
  /*
    cards1にcards2にあるカードを加える
  */
  int i,j;
  
  for(i=0;i<15;i++)
    for(j=0;j<5;j++)
      if(cards2[j][i]>0)cards1[j][i]=1; 
}

void cardsAnd(int cards1[8][15],int cards2[8][15]){ 
  /*
    cards1のカードのうち、cards2にあるものだけをcards1にのこす。
  */
  int i,j;
  
  for(i=0;i<15;i++)
    for(j=0;j<5;j++)
      if(cards1[j][i]==1&&cards2[j][i]==1) cards1[j][i]=1;
      else cards1[j][i]=0;
}

void cardsDiff(int cards1[8][15],int cards2[8][15]){ 
  /*
    cards1からcards2にあるカードを削除する
  */
  int i,j;
  
  for(i=0;i<15;i++)
    for(j=0;j<5;j++)
      if(cards2[j][i]==1) cards1[j][i]=0;
}
void cardsNot(int cards[8][15]){ 
  /*
    カードの有無を反転させる
  */
  int i,j;
  
  for(i=0;i<15;i++)
    for(j=0;j<5;j++)
      if(cards[j][i]==1) cards[j][i]=0;
      else cards[j][i]=1;
}


void outputTable(int table[8][15]){ 
  /*
    引数で渡されたカードテーブルを出力する
  */
  int i,j;
  for(i=0;i<8;i++){
    for(j=0;j<15;j++){
      printf("%i ",table[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}

void copyTable(int dest_table[8][15], int org_table[8][15]){ 
  /*
    引数で渡されたカードテーブルorg_tableを
    カードテーブルdest_tableにコピーする
  */
  int i,j;
  for(i=0;i<8;i++){
    for(j=0;j<15;j++){
      dest_table[i][j]=org_table[i][j];
    }
  }
} 

void copyCards(int dest_cards[8][15],int org_cards[8][15]){ 
  /*
    引数で渡されたカードテーブルorg_cardsのカード情報の部分を
    カードテーブルdest_cardsにコピーする
  */
  int i,j;
  for(i=0;i<5;i++){
    for(j=0;j<15;j++){
      dest_cards[i][j]=org_cards[i][j];
    }
  }
}


void clearCards(int cards[8][15]){  
  /*
    引数で渡されたカードテーブルcardsのカード情報の部分を全て0にし、カードを一枚も無い状態にする。
  */
  int s,t;
  
  for(s=0;s<5;s++){
    for(t=0;t<15;t++){
      cards[s][t]=0;
    }   
  }
}

void clearTable(int cards[8][15]){ 
  /*
    引数で渡されたカードテーブルcardsを全て0にする。
  */
  int s,t;
  
  for(s=0;s<8;s++){
    for(t=0;t<15;t++){
      cards[s][t]=0;
    }   
  }
}

int beEmptyCards(int cards[8][15]){  
  /*
    引数で渡されたカードテーブルcardsの含むカードの枚数が0のとき1を、
    それ以外のとき0を返す
  */
  int i,j,f=1;
  
  for(i=0;i<5;i++){
    for(j=0;j<15;j++){
      if(cards[i][j]>0)f=0;
    }
  }
  return f;
}

int qtyOfCards(int cards[8][15]){  
  /*
    引数で渡されたカードテーブルcardsの含むカードの枚数を返す
  */
  int i,j,count=0;
  
  for(i=0;i<5;i++)
    for(j=0;j<15;j++)
      if(cards[i][j]>0)count++;
  
  return count;
}



void makeJKaidanTable(int tgt_cards[][15], int my_cards[][15]){
  /*
    渡されたカードテーブルmy_cardsから、ジョーカーを考慮し階段で出せるかどうかを解析し、
    結果をテーブルtgt_cardsに格納する。
  */
  int i,j;
  int count,noJcount;    //ジョーカーを使用した場合のカードの枚数,使用しない枚数
  
  clearTable(tgt_cards);         //テーブルのクリア
  if(state.joker==1){            //jokerがあるとき
    for(i=0;i<4;i++){            //各スート毎に走査し
      count=1;
      noJcount=0; 
      for(j=13;j>=0;j--){        //順番にみて
	if(my_cards[i][j]==1){   //カードがあるとき
	  count++;               //2つのカウンタを進める
	  noJcount++;
	}
	else{                    //カードがないとき
	  count=noJcount+1;      //ジョーカーありの階段の枚数にジョーカー分を足す 
	  noJcount=0;            //ジョーカーなしの階段の枚数をリセットする
	}
	
	if(count>2){              //3枚以上のとき
	  tgt_cards[i][j]=count;  //その枚数をテーブルに格納
	}
	else{
	  tgt_cards[i][j]=0;      //その他は0にする
	}
      }
    }
  }

  if(g_logging==1){
    printf("make Joker kaidan \n");
    outputTable(tgt_cards);
  }
}

void makeKaidanTable(int tgt_cards[][15], int my_cards[][15]){
  /*
    渡されたカードテーブルmy_cardsから、階段で出せるかどうかを解析し、
    結果をテーブルtgt_cardsに格納する。
  */
  int i,j;
  int count;
  
  clearTable(tgt_cards);
  for(i=0;i<4;i++){             //各スート毎に走査し
    for(j=13,count=0;j>0;j--){  //順番にみて
      if(my_cards[i][j]==1){    //カードがあるとき
	count++;                //カウンタを進め
      }
      else{
	count=0;                //カードがないときリセットする
      }
      
      if(count>2){              //3枚以上のときその枚数をテーブルに格納
	tgt_cards[i][j]=count;   
      }
      else{
	tgt_cards[i][j]=0;     //その他は0にする
      }
    }
  }
  if(g_logging==1){
    printf("make kaidan \n");
    outputTable(tgt_cards);
  }
}

void makeGroupTable(int tgt_cards[][15], int my_cards[][15]){
  /*
    渡されたカードテーブルmy_cardsから、2枚以上の枚数組で出せるかどうかを解析し、
    結果をテーブルtgt_cardsに格納する。
  */
  int i,j;
  int count;
  
  
  clearTable(tgt_cards);
  for(i=0;i<15;i++){  //それそれの強さのカードの枚数を数え
    count=my_cards[0][i]+my_cards[1][i]+my_cards[2][i]+my_cards[3][i];
    if(count>1){      //枚数が2枚以上のとき
      for(j=0;j<4;j++){
	if(my_cards[j][i]==1){    //カードを持っている部分に
	  tgt_cards[j][i]=count; //その枚数を格納
	}
      }
    }
  }
  if(g_logging==1){
    printf("make group \n");
    outputTable(tgt_cards);
  }
}

void makeJGroupTable(int tgt_cards[][15], int my_cards[][15]){
  /*
    渡されたカードテーブルmy_cardsから、
    ジョーカーを考慮し2枚以上の枚数組で出せるかどうかを解析し、
    結果をテーブルtgt_cardsに格納する。
  */
  int i,j;
  int count;
 
  clearTable(tgt_cards);
  if(state.joker!=0){ 
    for(i=0;i<14;i++){ //それそれの強さのカードの枚数を数え ジョーカーの分を加える
      count=my_cards[0][i]+my_cards[1][i]+my_cards[2][i]+my_cards[3][i]+1;
      if(count>1){     //枚数が2枚以上のとき
	for(j=0;j<4;j++){
	  if(my_cards[j][i]==1){   //カードを持っている部分に
	    tgt_cards[j][i]=count; //その枚数を格納
	  }
	}
      }
    }
  }
  if(g_logging==1){
    printf("make Joker group \n");
    outputTable(tgt_cards);
  }
}


void lowCards(int out_cards[8][15],int my_cards[8][15],int threshold){
  /*
    渡されたカードテーブルmy_cardsのカード部分を
    threshold以上の部分は0でうめ,thresholdより低い部分をのこし、
    カードテーブルout_cardsに格納する。
  */
  int i;
  copyTable(out_cards,my_cards); //my_cardsをコピーして
  for(i=threshold;i<15;i++){    //thresholdから15まで
    out_cards[0][i]=0;          //0でうめる
    out_cards[1][i]=0;
    out_cards[2][i]=0;
    out_cards[3][i]=0;
  }
}


void highCards(int out_cards[8][15],int my_cards[8][15],int threshold){
  /*
    渡されたカードテーブルmy_cardsのカード部分を
    threshold以下の部分は0でうめ,thresholdより高い部分をのこし
    カードテーブルout_cardsに格納する
  */
  int i;
  copyTable(out_cards,my_cards); //my_cardsをコピーして
  for(i=0;i<=threshold;i++){    //0からthresholdまで
    out_cards[0][i]=0;          //0でうめる
    out_cards[1][i]=0;
    out_cards[2][i]=0;
    out_cards[3][i]=0;
  } 
}
int nCards(int n_cards[8][15],int target[8][15],int n){
  /*
    n枚のペアあるいは階段のみをn_cards にのこす。このときテーブルにのる数字はnのみ。
    カードが無いときは0,あるときは1をかえす。
  */
  int i,j,flag=0;         
  clearTable(n_cards);          //テーブルをクリア
  for(i=0;i<4;i++)           
    for(j=0;j<15;j++)           //テーブル全体を走査し
      if(target[i][j]==(int)n){ //nとなるものをみつけたとき
      	n_cards[i][j]=n;
	flag=1;                 //フラグをたて
      }else{                    //n以外の場所は
	n_cards[i][j]=0;        //0で埋める。
      }
  return flag;
}

void lockCards(int target_cards[8][15],int suit[5]){
  /*
    大域変数state.suitの１が立っているスートのみカードテーブルtarget_cardsに残す。
  */
  int i,j;
  for(i=0;i<4;i++)
    for(j=0;j<15;j++)
      target_cards[i][j]*=suit[i]; //suit[i]==1 のときはそのまま,==0のとき0である。
}

void lowGroup(int out_cards[8][15],int my_cards[8][15],int group[8][15]) {
  /*
    渡された枚数組で出せるカードの情報をのせたgroupとカードテーブルmy_cardsから
    最も低い枚数組を探し、見つけたらカードテーブルout_cardsにそのカードを載せる。
  */
  int i, j;					//カウンタ
  int count = 0,qty=0;				//カードの枚数,総数
  
  clearTable(out_cards);
  for(j=1; j<14; j++) {	                        //ランクが低い順に探索する 
    for(i=0; i<4; i++) {
      if(group[i][j] >1 ) {	        	//groupテーブルに2以上の数字を発見したら
	out_cards[i][j] = 1;			//out_cardsにフラグを立てる
	count++;
	qty=group[i][j];
      }
    }
    if(count >0) break;			//ループ脱出用フラグが立っていたら
  }
  
  for(i=0; count<qty; i++) {
    if(my_cards[i][j] == 0 && (state.lock==0||state.suit[i]==1)){	
      out_cards[i][j] = 2;		//ジョーカー用フラグを立てる 
      count++;
    }
  }
}

void highGroup(int out_cards[8][15],int my_cards[8][15],int group[8][15]) {
  /*
    渡された枚数組で出せるカードの情報をのせたgroupとカードテーブルmy_cardsから
    最も高い枚数組を探し、見つけたらカードテーブルout_cardsにそのカードを載せる。
  */
  int i, j;					//カウンタ
  int count = 0,qty=0;				//カードの枚数,総数
  
  clearTable(out_cards);
  for(j=13; j>0; j--) {	                        //ランクが低い順に探索する 
    for(i=0; i<4; i++) {
      if(group[i][j] > 1) {	        	//groupテーブルに2以上の数字を発見したら
	out_cards[i][j] = 1;			//out_cardsにフラグを立てる
	count++;
	qty=group[i][j];
      }
    }
    if(count >0) break;			//ループ脱出用フラグが立っていたら
  }
  
  for(i=0; count<qty; i++) {
    if(my_cards[i][j] == 0 && (state.lock==0||state.suit[i]==1)){
      out_cards[i][j] = 2;		//ジョーカー用フラグを立てる 
      count++;
    }
  }
}


void lowSequence(int out_cards[8][15],int my_cards[8][15],int sequence[8][15]){
  /*
    渡された階段で出せるカードの情報をのせたgroupとカードテーブルmy_cardsから
    最も低い階段を探し、見つけたらカードテーブルout_cardsにそのカードを載せる。
  */
  int i,j,lowvalue,lowline=0,lowcolumn=0;
  
  lowvalue = 0;
  
  clearTable(out_cards);
  i = 0;
  
  //lowsequenceの発見
  while((i < 15) && (lowvalue == 0)){ //階段テーブル中に階段が見つかるまで繰り返し
    j = 0;
    while(j < 4){
      if(sequence[j][i] != 0){      //低い数字から調べ,階段テーブルが0以外だったら分岐
	if(sequence[j][i] > lowvalue){ //同じ数字を起点として作られる階段の中で最長か否か
	  lowvalue = sequence[j][i];   //最長だったら値と場所を保存
	  lowline = j;
	  lowcolumn = i;
	}
      }
      j++;
    }
    if(lowvalue == 0){
      i++;
    }
  }
  
  //out_cardsへの書出し
  if(lowvalue != 0){              //階段が見つからなかったらout_cardsには書出さない
    for(i = lowcolumn; i < (lowcolumn+lowvalue); i++){
      if(my_cards[lowline][i] == 1){
	out_cards[lowline][i] = 1;   //普通の手札として持っていたら1を立てる
      }
      else{
	out_cards[lowline][i] = 2;        //持っていなかったらジョーカーなので2を立てる
      }
    }
  }
}

void highSequence (int out_cards[8][15],int my_cards[8][15],int sequence[8][15]){
  /*
    渡された階段で出せるカードの情報をのせたgroupとカードテーブルmy_cardsから
    最も高い階段を探し、見つけたらカードテーブルout_cardsにそのカードを載せる。
  */
  int i,j,k,highvalue,highline=0,highcolumn=0,prevalue;
  highvalue = 0;

  clearTable(out_cards);
  i = 14;
  
  //highsequenceの発見
  while((i > 0) && (highvalue == 0)){  //階段テーブル中に階段が見つかるまで繰り返し
    j = 0;
    while(j < 4){
      k = -1;
      if((sequence[j][i] != 0) && (my_cards[j][i] != 0)){//高い数字から調べ,階段テーブルが0以外だったら分岐
	do{                       //見つけた階段の最高値から,最長の階段を探す
	  if(sequence[j][i-k] >= highvalue){ //同じ最高値を持つ階段の中で最長か否か
	    highvalue = sequence[j][i-k];  //最長だったら記録
	    highline = j;
	    highcolumn = i-k;
	  }
	  prevalue = sequence[j][i-k];
	  k++;
	}while(prevalue <= sequence[j][i-k]);
	
      }	
      j++;	
    }
    if(highvalue == 0){
      i--;
    }
  }

  //out_cardsへの書出し
  for(i = highcolumn; i < (highcolumn+highvalue); i++){
    if(my_cards[highline][i] == 1){
      out_cards[highline][i] = 1; //普通の手札として持っていたら1を立てる
    }
    else{
      out_cards[highline][i] = 2;     //持っていなかったらジョーカーなので2を立てる
    }
  } 
}

//my_cards(手札)からペア,階段等の役のカードを除去したものをout_cardに格納する 
void removeGroup(int out_cards[8][15],int my_cards[8][15],int group[8][15]){
  /*
    渡された枚数組で出せるカードの情報をのせたgroupとカードテーブルmy_cardsから
    枚数組以外のカードを探し、カードテーブルout_cardsにそのカードを載せる。
  */
  int i,j;
  
  for (i = 0; i < 15; i++){
    for(j = 0; j < 4; j++){
      if((my_cards[j][i] == 1) && (group[j][i] == 0)){
	out_cards[j][i] = 1;         //mycardsに存在し,かつ役テーブルにない場合1
      }
      else{
	out_cards[j][i] = 0;             //それ以外(mycardsにないか,役テーブルにある)の場合0
      }
    }
  }
}

void removeSequence(int out_cards[8][15],int my_cards[8][15],int sequence[8][15]){
  /*
    渡された階段で出せるカードの情報をのせたgroupとカードテーブルmy_cardsから
    階段以外のカードを探し、カードテーブルout_cardsにそのカードを載せる。
  */
  int i,j,k;
  
  for(j = 0; j < 4; j++){
    for (i = 0; i < 15; i++){
      if((my_cards[j][i] == 1) && (sequence[j][i] == 0)){
	out_cards[j][i] = 1;           //mycardsに存在し,かつ役テーブルにない場合1
      }else if(sequence[j][i] > 2){
      	for(k=0;k < sequence[j][i];k++){
      	  out_cards[j][i+k] = 0;
      	}
      	i += k-1;
      } 
      else {
	out_cards[j][i] = 0;      //それ以外(mycardsにないか,役テーブルにある)の場合0
      }
    }
  }
}


void lowSolo(int out_cards[8][15],int my_cards[8][15],int joker_flag){
  /*
    低い方から探して,最初に見つけたカードを一枚out_cardsにのせる。
    joker_flagが1のとき,カードが見つからなければ,jokerを一枚out_cardsにのせる。
  */
  int i,j,find_flag=0;

  clearTable(out_cards);                  //テーブルをクリア
  for(j=1;j<14&&find_flag==0;j++){        //低い方からさがし
    for(i=0;i<4&&find_flag==0;i++){
      if(my_cards[i][j]==1){              //カードを見つけたら               
	find_flag=1;                      //フラグを立て
	out_cards[i][j]=my_cards[i][j];   //out_cardsにのせ,ループを抜ける。
      }
    }
  }
  if(find_flag==0&&joker_flag==1){       //見つからなかったとき
    out_cards[0][15]=2;                  //ジョーカーをのせる
  }
}

void highSolo(int out_cards[8][15],int my_cards[8][15],int joker_flag){
  /*
    高い方から探して,最初に見つけたカードを一枚out_cardsにのせる。
    joker_flagがあるとき,カードが見つからなければ,jokerを一枚out_cardsにのせる。
  */
  int i,j,find_flag=0;
  
  clearTable(out_cards);                 //テーブルをクリア
  for(j=13;j>0&&find_flag==0;j--){       //高い方からさがし
    for(i=0;i<4&&find_flag==0;i++){
      if(my_cards[i][j]==1){              //カードを見つけたら
	find_flag=1;                      //フラグを立て
	out_cards[i][j]=my_cards[i][j];   //out_cardsにのせ,ループを抜ける。
      }
    }
  }
  if(find_flag==0&&joker_flag==1){       //見つからなかったとき
    out_cards[0][0]=2;                   //ジョーカーをのせる
  }
}

void change(int out_cards[8][15],int my_cards[8][15],int num_of_change){
  /*
    カード交換時のアルゴリズム
    大富豪あるいは富豪が、大貧民あるいは貧民にカードを渡す時のカードを
    カードテーブルmy_cardsと交換枚数num_of_changeに応じて、
    低いほうから選びカードテーブルout_cardsにのせる
  */
  int count=0;
  int one_card[8][15];
  
  clearTable(out_cards);
  while(count<num_of_change){
    lowSolo(one_card,my_cards,0);
    cardsDiff(my_cards,one_card);
    cardsOr(out_cards,one_card);
    count++;
  }
}

void lead(int out_cards[8][15],int my_cards[8][15]){
  /*
    新しくカードを提出するときの選択ルーチン
    カードテーブルmy_cardsから階段=>ペア=>一枚の順で枚数の多いほうから走査し,
    低いカードからみて、はじめて見つけたものを out_cardsにのせる。
  */
  int group[8][15];           //枚数組を調べるためのテーブル
  int sequence[8][15];        //階段を調べるためのテーブル
  int temp[8][15];            //一時使用用のテーブル
  int i,find_flag=0;          //手札が発見したか否かのフラグ

  clearTable(group);
  clearTable(sequence);
  clearTable(temp);
  if(state.joker==1){                      //ジョーカーがあるとき,ジョーカーを考慮し,
    makeJGroupTable(group,my_cards);       //階段と枚数組があるかを調べ,
    makeJKaidanTable(sequence,my_cards);   //テーブルに格納する
  }else{
    makeGroupTable(group,my_cards);        //ジョーカーがないときの階段と枚数組の
    makeKaidanTable(sequence,my_cards);    //状況をテーブルに格納する
  }

  for(i=15;i>=3&&find_flag==0;i--){         //枚数の大きい方から,見つかるまで
    find_flag=nCards(temp,sequence,i);      //階段があるかをしらべ,
    
    if(find_flag==1){                       //見つかったとき
      lowSequence(out_cards,my_cards,temp); //そのなかで最も低いものをout_cards
    }                                       //にのせる         
  }
  for(i=5;i>=2&&find_flag==0;i--){          //枚数の大きい方から,見つかるまで
    find_flag=nCards(temp,group,i);         //枚数組があるかを調べ,
    if(find_flag==1){                       //見つかったとき
      lowGroup(out_cards,my_cards,temp);    //そのなかで最も低いものをout_cards
    }                                       //のせる
  }
  if(find_flag==0){                          //まだ見つからないとき
    lowSolo(out_cards,my_cards,state.joker); //最も低いカードをout_cardsにのせる。
  }
}

void leadRev(int out_cards[8][15],int my_cards[8][15]){
  /*
    革命時用の新しくカードを提出するときの選択ルーチン
    カードテーブルmy_cardsから階段=>ペア=>一枚の順で枚数の多いほうから走査し,
    高いカードからみて、はじめて見つけたものを out_cardsにのせる。
  */
  int group[8][15];           //枚数組を調べるためのテーブル
  int sequence[8][15];        //階段を調べるためのテーブル
  int temp[8][15];            //一時使用用のテーブル
  int i,find_flag=0;          //手札が発見したか否かのフラグ
  //clearTable(group);
  //clearTable(sequence);
  //clearTable(temp);
  if(state.joker==1){                        //ジョーカーがあるとき,ジョーカーを考慮し,
    makeJGroupTable(group,my_cards);         //階段と枚数組があるかを調べ,
    makeJKaidanTable(sequence,my_cards);     //テーブルに格納する
  }else{
    makeGroupTable(group,my_cards);          //ジョーカーがないときの階段と枚数組の
    makeKaidanTable(sequence,my_cards);      //状況をテーブルに格納する
  }		
  for(i=15;i>=3&&find_flag==0;i--){          //枚数の大きい方から,見つかるまで
    
    find_flag=nCards(temp,sequence,i);       //階段があるかをしらべ,
    
    if(find_flag==1){                        //見つかったとき
      highSequence(out_cards,my_cards,temp); //そのなかで最も高いものをout_cards
    }                                        //にのせる
  }
  for(i=5;i>=2&&find_flag==0;i--){           //枚数の大きい方から,見つかるまで
    find_flag=nCards(temp,group,i);          //枚数組があるかを調べ,
    if(find_flag==1){                        //見つかったとき
      highGroup(out_cards,my_cards,temp);    //そのなかで最も高いものをout_cards
    }                                        //にのせる       
  }
  if(find_flag==0){                          //まだ見つからないとき
    highSolo(out_cards,my_cards,state.joker);//最も高いカードをout_cardsにのせる
  }
}
 
void followSolo(int out_cards[8][15],int my_cards[8][15],int joker_flag){
  /*
    他のプレーヤーに続いてカードを一枚で出すときのルーチン
    joker_flagが1の時ジョーカーを使おうとする
    提出するカードはカードテーブルout_cardsに格納される
  */
  int group[8][15];       //枚数組を調べるためのテーブル
  int sequence[8][15];    //階段を調べるためのテーブル
  int temp[8][15];        //一時使用用のテーブル
  
  makeGroupTable(group,my_cards);           //枚数組を書き出す
  makeKaidanTable(sequence,my_cards);       //階段を書き出す
  
  removeSequence(temp,my_cards,sequence);   // 階段を除去
  removeGroup(out_cards,temp,group);        // 枚数組を除去
  
  highCards(temp,out_cards,state.ord);      // 場のカードより弱いカードを除去

  if(state.lock==1){               
    lockCards(temp,state.suit);             //ロックされているとき出せないカードを除去
  }
  lowSolo(out_cards,temp,state.joker);      //残ったカードから弱いカードを抜き出す
}

void followGroup(int out_cards[8][15],int my_cards[8][15],int joker_flag){
  /*  
    他のプレーヤーに続いてカードを枚数組で出すときのルーチン
    joker_flagが1の時ジョーカーを使おうとする
    提出するカードはカードテーブルout_cardsに格納される
  */
  int group[8][15];
  int ngroup[8][15];
  int temp[8][15];
  
  highCards(temp,my_cards,state.ord);          //場より強いカードを残す 
  if(state.lock==1){                           //ロックされているとき
    lockCards(temp,state.suit);                //出せないカードを除去
  }
  makeGroupTable(group,temp);                  //残ったものから枚数組を書き出す
  if(nCards(ngroup,group,state.qty)==0&&state.joker==1){
    //場と同じ枚数の組が無いときジョーカーを使って探す
    makeJGroupTable(group,temp);               
    nCards(ngroup,group,state.qty);     //場と同じ枚数の組のみのこす。 
  }
  lowGroup(out_cards,my_cards,ngroup);  //一番弱い組を抜き出す
}

void followSequence(int out_cards[8][15],int my_cards[8][15],int joker_flag){
  /*
    他のプレーヤーに続いてカードを階段で出すときのルーチン
    joker_flagが1の時ジョーカーを使おうとする
    提出するカードはカードテーブルout_cardsに格納される
  */
  int seq[8][15];
  int nseq[8][15];
  int temp[8][15];
  
  highCards(temp,my_cards,state.ord);          //場より強いカードを残す
  if(state.lock==1){                           //ロックされているとき
    lockCards(temp,state.suit);                //出せないカードを除去
  }
  makeKaidanTable(seq,temp);                   //階段を書き出す
  if(nCards(nseq,seq,state.qty)==0&&state.joker==1){
    //場と同じ枚数の階段が無いときジョーカーを使って探す
    makeJKaidanTable(seq,temp);
    nCards(nseq,seq,state.qty);          //場と同じ枚数の組のみのこす。
  }
  lowSequence(out_cards,my_cards,nseq);  //一番弱い階段を
}

void followSoloRev(int out_cards[8][15],int my_cards[8][15],int joker_flag){
  /*
    革命状態のときに他のプレーヤーに続いてカードを一枚で出すときのルーチン
    joker_flagが1の時ジョーカーを使おうとする
    提出するカードはカードテーブルout_cardsに格納される
  */
  int group[8][15];
  int sequence[8][15];
  int temp[8][15];
  
  makeGroupTable(group,my_cards);            //枚数組を書き出す
  makeKaidanTable(sequence,my_cards);        //階段を書き出す
  
  removeSequence(temp,my_cards,sequence);    // 階段を除去
  removeGroup(out_cards,temp,group);         // 枚数組を除去
  lowCards(temp,out_cards,state.ord);        // 場のカードより強いカードを除去
  if(state.lock==1){
    lockCards(temp,state.suit);          //ロックされているとき出せないカードを除去
  }
  highSolo(out_cards,temp,state.joker);  //残ったカードから強いカードを抜き出す
}

void followGroupRev(int out_cards[8][15],int my_cards[8][15],int joker_flag){
  /*
    革命状態のときに他のプレーヤーに続いてカードを枚数組で出すときのルーチン
    joker_flagが1の時ジョーカーを使おうとする
    提出するカードはカードテーブルout_cardsに格納される
  */
  int group[8][15];
  int ngroup[8][15];
  int temp[8][15];
  
  lowCards(temp,my_cards,state.ord);          //場より弱いカードを残す
  if(state.lock==1){                          //ロックされているとき
    lockCards(temp,state.suit);               //出せないカードを除去
  }
  makeGroupTable(group,temp);                 //枚数組を書き出す
  if(nCards(ngroup,group,state.qty)==0&&state.joker==1){
    //場と同じ枚数の組が無いときジョーカーを使って探す
    makeJGroupTable(group,temp);
    nCards(ngroup,group,state.qty);      //場と同じ枚数の組のみのこす。
  }
  highGroup(out_cards,my_cards,ngroup);  //残ったものから一番強い組を抜き出す
}

void followSequenceRev(int out_cards[8][15],int my_cards[8][15],int joker_flag){
  /*
    革命状態のときに他のプレーヤーに続いてカードを階段で出すときのルーチン
    joker_flagが1の時ジョーカーを使おうとする
    提出するカードはカードテーブルout_cardsに格納される
  */
  int seq[8][15];
  int nseq[8][15];
  int temp[8][15];
  
  lowCards(temp,my_cards,state.ord);          //場より弱いカードを残す
  if(state.lock==1){                          //ロックされているとき
    lockCards(temp,state.suit);               //出せないカードを除去
  }
  makeKaidanTable(seq,temp);                  //階段を書き出す
  if(nCards(nseq,seq,state.qty)==0&&state.joker==1){
    //場と同じ枚数の階段が無いときジョーカーを使って探す
    makeJKaidanTable(seq,temp);
    nCards(nseq,seq,state.qty);          //場と同じ枚数の階段のみのこす。
  } 
  highSequence(out_cards,my_cards,nseq); //残ったものから一番強い組を抜き出す
}

void follow(int out_cards[8][15],int my_cards[8][15]){
  /*
    他のプレーヤーに続いてカードを出すときのルーチン
    場の状態stateに応じて一枚、枚数組、階段の場合に分けて
    対応すれる関数を呼び出す
    提出するカードはカードテーブルout_cardsに格納される
  */
  clearTable(out_cards);
  if(state.qty==1){
    followSolo(out_cards,my_cards,state.joker);    //一枚のとき
  }else{
    if(state.sequence==0){
      followGroup(out_cards,my_cards,state.joker);  //枚数組のとき
    }else{
      followSequence(out_cards,my_cards,state.joker); //階段のとき
    }
  }
}

void followRev(int out_cards[8][15],int my_cards[8][15]){
  /*
    他のプレーヤーに続いてカードを出すときのルーチン
    場の状態stateに応じて一枚、枚数組、階段の場合に分けて
    対応すれる関数を呼び出す
    提出するカードはカードテーブルout_cardsに格納される
  */
  clearTable(out_cards);
  if(state.qty==1){
    followSoloRev(out_cards,my_cards,state.joker);    //一枚のとき
  }else{
    if(state.sequence==0){
      followGroupRev(out_cards,my_cards,state.joker);  //枚数組のとき
    }else{
      followSequenceRev(out_cards,my_cards,state.joker); //階段のとき
    }
  }
}

int cmpCards(int cards1[8][15],int  cards2[8][15]){
  /*
    カードテーブルcards1、cards2のカード部分を比較し、
    異なっていれば1、一致していれば0を返す
  */
  int i,j,flag=0;
  
  for(i=0;i<5;i++)
    for(j=0;j<15;j++)
      if(cards1[i][j]!=cards2[i][j])
	flag=1;
  
  return flag;
}

int cmpState(struct state_type* state1,struct state_type* state2){
  /*
    状態を格納するstate1とstate2を比較し、一致すれば0を、
    異なっていればそれ以外を返す
  */
  int i,flag=0;
  if(state1->ord != state2->ord) flag+=1;
  if(state1->qty != state2->qty) flag+=2;
  if(state1->sequence != state2->sequence) flag+=4;
  for(i=0;i<5;i++)
    if(state1->suit[i]!=state2->suit[i]) flag+=8;
  if(state1->onset != state2->onset) flag+=16;
  return flag;
}

int getLastPlayerNum(int ba_cards[8][15]){
  /*
    最後パス以外のカード提出をしたプレーヤーの番号を返す。
    この関数を正常に動作させるためには、
    サーバから場に出たカードをもらう度に
    この関数を呼び出す必要がある。
  */
  static struct state_type last_state;
  static int last_player_num=-1;
  
  if(g_logging==1){  //ログの表示
    printf("Now state \n");
    showState(&state);
    printf(" Last state \n");
    showState(&last_state);
  }
  
  if(cmpState(&last_state,&state)!=0){ //場の状態に変化が起きたら
    last_player_num =ba_cards[5][3];   //最後のプレーヤと
    last_state=state;                  //最新の状態を更新する
  }
  
  if(g_logging==1){ //ログの表示
    printf("last player num : %d\n",last_player_num);
  }
  
  return last_player_num;
}

//----------------------------------------------------------------------------------
//--------------------------------ここから自作の関数--------------------------------
//----------------------------------------------------------------------------------

int setmake(int out_cards[][15],int my_cards[8][15],int joker_flag){
//できるだけ組数が少なくできるよう階段を作り、階段を格納したものと、合計の組数を返す
//out_cards[0]~[3]には階段で出せる場合は右端の場所に階段の枚数が入り、階段以外のカードは1が入る
//out_cards[4]には階段に使用していないカードの枚数が入る
  int tgt_cards[8][15];//テーブル
  int i,j,k;
  int count=0;
  //out_cards[4][14]=0;
  clearTable(out_cards);
  clearTable(tgt_cards);
  for(i=0;i<4;i++){
    for(j=1;j<=13;j++){
      tgt_cards[5][j]+=my_cards[i][j];//各数字の枚数を記録
    }
  }
  for(i=0;i<4;i++){             //各スート毎に走査し
    for(j=1;j<=12;j++){         //Aまで順番にみて
      if(my_cards[i][j]==1){    //カードがあるとき
	count++;                //カウンタを進め
      }
      else{
	count=0;                //カードがないときリセットする
      }
      if(count>2){              //3枚以上のときその枚数をテーブルに格納
	tgt_cards[i][j]=count;
      }
      else{
	tgt_cards[i][j]=0;      //その他は0にする
      }
    }
    count=0;
  }
  count=0;
  //階段ができた場合
  //例えばスペード5～7の場合tgt_cardsの[0][5]は3,[0][3],[0][4]は0となっている
  //スペード5～8の場合tgt_cardsの[0][6]は4,[0][5]は3,[0][3],[0][4]は0となっている
  for(i=0;i<4;i++){
    for(j=13;j>0;j--){
      if(tgt_cards[i][j]>=3){//階段を発見したとき
        count=0;
        for(k=0;k<tgt_cards[i][j];k++){//その階段の数字をみて
          if(tgt_cards[5][j-k]>=2 && j!=6){//8のカード以外で階段以外にもその数字があれば
            count++;//カウンタを進める
          }
        }
        if(j>=6 && j-tgt_cards[i][j]<6){//8が含まれている場合
          if(count+1==tgt_cards[i][j]){//8のカード以外の階段の数字がすべてペアとなる場合、階段を崩す
             out_cards[i][j]=my_cards[i][j];//カードを残す
          }
          else{
            out_cards[i][j]=tgt_cards[i][j];//階段を残す(作る)
            for(k=0;k<tgt_cards[i][j];k++){
              tgt_cards[5][j-k]-=1;//その階段の数字の枚数を減らす
            }
            j-=k;//階段の左端の1つ左に移動
          }
        }
        else{//8が含まれてない場合
          if(count==tgt_cards[i][j]){//階段の数字がすべてペアとなる場合、階段を崩す
            out_cards[i][j]=my_cards[i][j];//カードを残す
          }
          else{
            out_cards[i][j]=tgt_cards[i][j];//階段を残す(作る)
            for(k=0;k<tgt_cards[i][j];k++){
              tgt_cards[5][j-k]-=1;//その階段の数字の枚数を減らす
            }
            j-=k;//階段の左端の1つ左に移動
          }
        }
      }//階段を発見したときの判定ここまで
      else{//階段でない
        out_cards[i][j]=my_cards[i][j];//カードを残す
      }
    }
  }
  count=0;
  //out_cardsに階段でない部分に1,階段の部分の右端にその枚数の数字が入る
  //スペード6～8の場合out_cardsの[0][6]は3,[0][4],[0][5]は0となっている
  //スペード6～9の場合out_cardsの[0][7]は4,[0][4],[0][5],[0][6]は0となっている

  //ジョーカーがあるとき,階段を捜し,判定
  clearCards(tgt_cards);         //カード部分リセット
  k=0;
  if(joker_flag==1){             //jokerがあるとき
    for(i=0;i<4;i++){            //各スート毎に走査し
      for(j=1;j<=12;j++){        //Aまで順番にみて
	if(my_cards[i][j]==1 && (tgt_cards[5][j]==1 || j==6)){//カードがあり、単体または8のカードのとき
	  count++;               //カウンタを進める
	}
	else if(k==0 && count!=0 && my_cards[i][j]!=1){//ジョーカー未使用でカードがないとき
	  count++;               //カウンタを進める
	  k=j;                   //ジョーカーの場所を記録
	}
	else{//探索後、階段ができるか判定
          if(count>2){           //3枚以上のとき階段を作り,ループから抜ける
            out_cards[i][j-1]=count;//右端に枚数格納
            if(k!=j-1){//ジョーカーの位置=階段の右端(2に近い方)でないとき
              tgt_cards[5][j-1]-=1;//その階段の数字の枚数を減らす(下の判定では右端を調べないため)
            }
            for(;count>1;count--){
              out_cards[i][j-count]=0;
              if(k!=j-count){
                tgt_cards[5][j-count]-=1;//その階段の数字の枚数を減らす
              }
            }
            out_cards[4][14]=2;
            i=4;
            j=13;
          }
          else{//2枚以下ならリセット
            count=0;
            k=0;
          }
        }
	if(j==12){//Aまで調べて端についたとき
          if(count>2){//以降、ループ内の階段判定と同じ処理
            out_cards[i][j]=count;
            if(k!=j){
              tgt_cards[5][j]-=1;
            }
            for(count=count-1;count>0;count--){
              out_cards[i][j-count]=0;
              if(k!=j-count){
                tgt_cards[5][j-count]-=1;
              }
            }
            out_cards[4][14]=2;
            i=4;
            j=13;
          }
          else{
            count=0;
            k=0;
          }
        }
      }//数字ループここまで
    }//スートループここまで
  }
  //スペード6,8とジョーカーで作った場合out_cardsの[0][6]は3,[0][4],[0][5]は0,k=5となっている
  if(out_cards[4][14]!=2){//ジョーカーで階段を作らない場合、ジョーカーを格納
    out_cards[4][14]=state.joker;
  }
  //組数を計算
  count=0;
  for(i=0;i<4;i++){            //各スート毎に走査し
    for(j=1;j<=13;j++){        //順番にみて
      if(out_cards[i][j]>2){   //階段があれば
        count++;               //カウント
      }
    }
  }
  for(j=1;j<=13;j++){
    out_cards[4][j]=0;
    for(i=0;i<4;i++){
      if(out_cards[i][j]==1){
        out_cards[4][j]+=1;//各数字の枚数を記録
      }
    }
  }
  for(j=1;j<=13;j++){
    if(out_cards[4][j]>0){  //階段以外でカードがあれば
      count++;              //カウント
    }
  }
  return count;
}

int kaidanhand(int select_cards[][15],int search[8][15],int own_cards[8][15]){
//階段をsearchから探し、結果をselect_cardsに格納する
  clearTable(select_cards);
  int i,j,k;
  for(j=3;j<=13;j++){
    for(i=0;i<4;i++){
      if(search[i][j]>=3){
        for(k=0;k<search[i][j];k++){
          if(own_cards[i][j-k]==0){
            select_cards[i][j-k]=2;
          }
          else{
            select_cards[i][j-k]=1;
          }
        }
        return j;//カードの強さを返す
      }
    }
  }
  return 0;//なければ0を返す
}

int grouphand(int select_cards[][15],int search[8][15],int n){
//指定の枚数の枚数組を探し、結果をselect_cardsに格納する
  clearTable(select_cards);
  int i,j;
  if(state.rev==0){
    for(j=1;j<=13;j++){
      if(n!=0){
        //枚数が指定されている場合
        if(search[4][j]==n){
          for(i=0;i<=3;i++){
            if(search[i][j]==1){
              select_cards[i][j]=1;
            }
          }
          return j;//カードの強さを返す
        }
      }
      else{
        //枚数が指定されていない場合
        if(search[4][j]>0){
          for(i=0;i<=3;i++){
            if(search[i][j]==1){
              select_cards[i][j]=1;
            }
          }
          return j;//カードの強さを返す
        }
      }
    }
  }
  else{
    //革命時も同じように選ぶ
    for(j=13;j>=1;j--){
      if(n!=0){
        if(search[4][j]==n){
          for(i=0;i<=3;i++){
            if(search[i][j]==1){
              select_cards[i][j]=1;
            }
          }
          return j;//カードの強さを返す
        }
      }
      else{
        if(search[4][j]>0){
          for(i=0;i<=3;i++){
            if(search[i][j]==1){
              select_cards[i][j]=1;
            }
          }
          return j;//カードの強さを返す
        }
      }
    }
  }
  return 0;//カードがなければ0を返す
}

void kou_lead(int select_cards[][15],int own_cards[8][15],int max,int used_cards[8][15],int tes[3][4][3]){
//新しくカードを出すときの思考
  int i,j,k,l,m;
  int search[8][15];
  clearTable(search);
  int pattern[13][7]={{0}};
  pat_make(pattern,own_cards,max,used_cards,state.joker, tes);
  if(own_cards[4][1]==2){//ジョーカーがある場合
    int target_pattern[13][7]={{0}};//手札組の候補を入れる
    int count[15]={0};//カードの枚数 or ジョーカーペアを判断するか(しない場合-1)
    for(i=0;i<4;i++){
      for(j=1;j<=13;j++){
        if(state.rev==0 && max<=j){
          count[j]=-1;
        }
        else if(state.rev==1 && max>=j){
          count[j]=-1;
        }
        else if(own_cards[i][j]==1){
          count[j]++;
        }
      }
    }
    for(i=0;i<4;i++){
      for(j=0;j<=14;j++){
        k=0;
        if(own_cards[i][j]==0){//ジョーカーを代用するか判定
          if(j<=11){
            if(own_cards[i][j+1]==1 && own_cards[i][j+2]==1){//右2箇所にカードがある
              k=1;
            }
          }
          if(j>=3){
            if(own_cards[i][j-1]==1 && own_cards[i][j-2]==1){//左2箇所にカードがある
              k=1;
            }
          }
          if(k==0 && own_cards[i][j-1]==1 && own_cards[i][j+1]==1){//左右にカードがある
            k=1;
          }
          if(k==0 && count[j]>=1){//同じ数字のカードが1枚以上(1回のみ)
            k=2;
            count[j]=-1;
          }
        }//ジョーカーを代用するか判定ここまで
        if(k>=1){//判定する場合,判定前のpatternと比較して良いかどうか判断
          //変換
          own_cards[i][j]=1;
          pat_make(target_pattern,own_cards,max,used_cards,0,tes);
          own_cards[i][j]=0;
          //条件式
          if(target_pattern[11][3]>=STRONG){//あがり手の場合
            if(target_pattern[11][3]>pattern[11][3]){//2minが高い
              k=3;
            }
          }
          else if(target_pattern[11][0]<pattern[11][0] && k==1){//組が減る
            k=3;
          }
          else if(target_pattern[11][0]==pattern[11][0] && target_pattern[11][4]<pattern[11][4] && k==1){//組数同じでzが小さくなる
            k=3;
          }
        }
        for(l=0;l<13;l++){
          for(m=0;m<7;m++){
            if(k==3){
              pattern[l][m]=target_pattern[l][m];
            }
            target_pattern[l][m]=0;
          }
        }
        if(pattern[11][3]>=100){//確実に勝てる場合は判定終了
          i=4;
          j=15;
        }
      }//jループ
    }//iループ
  }//ジョーカーの使用位置決定ここまで
  for(i=0;i<pattern[11][0];i++){//革命手があるかどうか
    if(pattern[i][1]>=5 || (pattern[i][1]==4 && (pattern[i][3]==0 || pattern[i][3]==14) ) ){
      pattern[11][1]=1;
    }
  }
  if(pattern[11][2]==-1){
    //もし予想外の動作が起きた場合,defaultの関数を使う
    if(state.rev==0){
      lead(select_cards,own_cards);    //通常時の提出用
    }else{
      leadRev(select_cards,own_cards); //革命時の提出用
    }
  }
  else{
    if(pattern[11][1]==0){//革命手がないとき,またはあがり判定の時は通常の判定に移る
      //あがり手札と判定していない場合
      j=0;
      if(pattern[11][3]<STRONG && pattern[11][0]<=4){
        for(i=0;i<pattern[11][0];i++){
          if(pattern[i][1]>=3){
            j++;
          }
        }
      }
      if(j!=0 && pattern[11][0]-j<=3){//組の数-階段の組の数<=3であれば階段優先
        for(i=0;i<pattern[11][0];i++){
          if(pattern[i][1]>=3){
            if(state.rev==0 && pattern[i][0]+2<max){
              pattern[i][6]=90;
            }
            else if(state.rev==1 && pattern[i][0]-2>max){
              pattern[i][6]=90;
            }
          }
        }
      }
      k=2;//通常判定に移る
    }
    else if(pattern[11][0]<=2){
      k=1;//組が2以下のときは革命を起こす
    }
    else{
      j=1;
      for(i=0;i<pattern[11][0];i++){
        if(pattern[i][4]<=100 && pattern[i][3]!=14 && pattern[i][1]<=2 && j<pattern[i][4]){
          j=pattern[i][4];//j=2枚以下の組でxが最大のものを記録
        }
      }
      if(j<=90 && pattern[11][4]>=pattern[11][5]){
        k=1;//強いカードなし、革命時のほうが強い
      }
      else if(j<=90 && pattern[11][4]<pattern[11][5]){
        k=2;//強いカードなし、革命時のほうが弱い
      }
      else if(j>=91 && pattern[11][4]>=pattern[11][5]){
        k=3;//強いカードあり、革命時のほうが強い
      }
      else{
        k=4;//強いカードあり、革命時のほうが弱い
      }
    }
    if(k==1){
      //革命を起こす
      for(i=0;i<pattern[11][0];i++){
        if(pattern[i][1]>=5 || (pattern[i][1]==4 && (pattern[i][3]==0 || pattern[i][3]==14) ) ){
          pattern[i][6]=90;
        }
        else if(pattern[i][4]>=STRONG && pattern[i][4]<=100 && pattern[i][3]!=14){
          pattern[i][6]=91;
        }
      }
    }
    else if(k==4){
      //革命を起こさない
      for(i=0;i<pattern[11][0];i++){
        if(pattern[i][1]>=5 || (pattern[i][1]==4 && (pattern[i][3]==0 || pattern[i][3]==14) ) ){
          pattern[i][6]=8;
        }
      }
    }
    else if(k==3){
      //弱いカードを出さないよう考える(この時点では起こさない)
      for(i=0;i<pattern[11][0];i++){
        if(pattern[i][0]<=5){
          pattern[i][6]=7;
        }
        if(pattern[i][1]>=5 || (pattern[i][1]==4 && (pattern[i][3]==0 || pattern[i][3]==14) ) ){
          pattern[i][6]=8;
        }
      }
    }
    else{//k==2
      //特に変更なし
    }

    //表示
        if(PRINT_PAT==2){
          for(i=0;i<11;i++){
            if(pattern[i][1]==0){
              i=11;
            }
            else{
              fprintf( stderr, "[ord%2d][qty%d][sui%2d][Jok%2d][x%3d][rx%3d][y%2d]\n",pattern[i][0],pattern[i][1],pattern[i][2],pattern[i][3],pattern[i][4],pattern[i][5],pattern[i][6]);
            }
          }
          fprintf( stderr, "[set%2d][rev%d][1min%3d][2min%3d][z%3d][rz%3d][r2m%3d]\n",pattern[11][0],pattern[11][1],pattern[11][2],pattern[11][3],pattern[11][4],pattern[11][5],pattern[11][6]);
          fprintf( stderr, "\n");
        }

    pattern[12][5]=-1;
    for(i=0;i<pattern[11][0];i++){
      //手札の組からyが一番大きいものを選ぶ
      if(pattern[12][6]<pattern[i][6]){
        pattern[12][0]=pattern[i][0];
        pattern[12][1]=pattern[i][1];
        pattern[12][2]=pattern[i][2];
        pattern[12][3]=pattern[i][3];
        pattern[12][4]=pattern[i][4];
        pattern[12][5]=pattern[i][5];
        pattern[12][6]=pattern[i][6];
      }
    }
    if(pattern[11][3]==201 && pattern[12][3]==0){//残り1組
      j=0;
      for(i=0;i<pattern[11][0];i++){
        if(pattern[i][3]==14 && pattern[i][2]==0){
          j=1;
        }
      }
      if(j==1){//ジョーカーを含めて出す
        pattern[12][1]+=1;
        pattern[12][3]=14;
        if(pattern[12][2]%2!=1){
          pattern[12][2]+=1;
        }
        else if(pattern[12][2]%4<=1){
          pattern[12][2]+=2;
        }
        else if(pattern[12][2]%8<=3){
          pattern[12][2]+=4;
        }
        else if(pattern[12][2]<8){
          pattern[12][2]+=8;
        }
      }
    }
    else if(pattern[12][4]>=STRONG && pattern[12][4]<=100 && pattern[12][3]==0 && pattern[12][1]==1){//あがり手選択の場合(単体)
      j=0;
      pattern[12][1]=0;
      pattern[12][2]=0;
      for(i=0;i<pattern[11][0];i++){
        if(pattern[i][0]==pattern[12][0] && pattern[i][3]==0){
          pattern[12][1]+=1;
          pattern[12][2]+=pattern[i][2];
        }
        if(pattern[i][3]==14 && pattern[i][2]==0){//ジョーカーを階段に使用していない場合
          j=1;
        }
      }
      if(pattern[12][1]==4){//4枚あるときはスペードとハートの2枚のみ出す
        pattern[12][1]=2;
        pattern[12][2]=3;
      }
      if(pattern[12][1]!=3 && j==1){
        //ジョーカーを含めて出す
        pattern[12][1]+=1;
        pattern[12][3]=14;
        if(pattern[12][2]%2!=1){
          pattern[12][2]+=1;
        }
        else if(pattern[12][2]%4<=1){
          pattern[12][2]+=2;
        }
        else if(pattern[12][2]%8<=3){
          pattern[12][2]+=4;
        }
        else if(pattern[12][2]<8){
          pattern[12][2]+=8;
        }
      }
    }
    //patternからselect_cardsに提出手を入れる
    if(pattern[12][3]==14 && pattern[12][2]==0){//ジョーカーの場合
      select_cards[0][0]=2;
    }
    else if(pattern[12][3]==14 && pattern[12][2]!=0){//ジョーカー込の枚数組
      if(pattern[12][2]%2==1){
        select_cards[0][pattern[12][0]]=1;
        if(own_cards[0][pattern[12][0]]==0){
          select_cards[0][pattern[12][0]]=2;
        }
      }
      if(pattern[12][2]%4>=2){
        select_cards[1][pattern[12][0]]=1;
        if(own_cards[1][pattern[12][0]]==0){
          select_cards[1][pattern[12][0]]=2;
        }
      }
      if(pattern[12][2]%8>=4){
        select_cards[2][pattern[12][0]]=1;
        if(own_cards[2][pattern[12][0]]==0){
          select_cards[2][pattern[12][0]]=2;
        }
      }
      if(pattern[12][2]>=8){
        select_cards[3][pattern[12][0]]=1;
         if(own_cards[3][pattern[12][0]]==0){
          select_cards[3][pattern[12][0]]=2;
        }
      }
    }
    else if(pattern[12][3]!=0){//階段組
      if(pattern[12][2]==1){
        k=0;
      }
      else if(pattern[12][2]==2){
        k=1;
      }
      else if(pattern[12][2]==4){
        k=2;
      }
      else{
        k=3;
      }
      for(j=0;j<pattern[12][1];j++){
        if(own_cards[k][pattern[12][0]-j]==1){
          select_cards[k][pattern[12][0]-j]=1;
        }
        else{
          select_cards[k][pattern[12][0]-j]=2;
        }
      }
    }
    else{//ジョーカーなしの枚数組
      if(pattern[12][2]%2==1){
        select_cards[0][pattern[12][0]]=1;
      }
      if(pattern[12][2]%4>=2){
        select_cards[1][pattern[12][0]]=1;
      }
      if(pattern[12][2]%8>=4){
        select_cards[2][pattern[12][0]]=1;
      }
      if(pattern[12][2]>=8){
        select_cards[3][pattern[12][0]]=1;
      }
    }
    for(i=0;i<4;i++){
      for(j=0;j<=14;j++){
        if(select_cards[i][j]==1 && own_cards[i][j]==0){
          select_cards[i][j]=2;
        }
      }
    }
  }
}

void kou_followgroup(int select_cards[][15],int own_cards[8][15],int max,int used_cards[8][15],int tes[3][4][3]){
//続いてカードを枚数組で出すときの思考
  int i,j,k;
  int p=0;//プレイヤ数
  int ord=state.ord;//現在見ているカードの強さ
  int card_flag=1;//探索終了まで1
  int target_cards[8][15];//出すカードの優先候補を入れる
  int use_own_cards[8][15];//出した後の状態を入れる
  int search[8][15];
  int suit[5]={0};//スート(suit[4]はスートの組み合わせパターンの順番)
  int pattern[13][7]={{0}};//カードを選ぶ前の手札のパターン
  int use_pattern[13][7]={{0}};//カードを選び出した後の手札のパターン
  int own_num[15]={0};//自分のカードの枚数を記録
  for(i=1;i<=13;i++){
    own_num[i]=own_cards[0][i]+own_cards[1][i]+own_cards[2][i]+own_cards[3][i];
  }
  pat_make(pattern,own_cards,max,used_cards,state.joker,tes);//現在の手札の組を作る
  if(state.lock==1){//縛りのとき
    for(i=0;i<4;i++){
      suit[i]=state.suit[i];
    }
  }
  for(i=0;i<5;i++){//あがっていない人数カウント
    if(own_cards[6][i]>0){
      p++;
    }
  }
  if(state.rev==0){
    ord++;
  }
  else{
    ord--;
  }
  //場のカードの次に強いカードの強さから探す
  clearTable(select_cards);
  clearTable(target_cards);
  suit[4]=0;
  if(ord>13 || ord<1){
    card_flag=0;
  }
  while(card_flag==1)//フラグが1の間
  {
    //選ぶスートのパターンをすべて調べる
    clearTable(target_cards);
    copyCards(search,own_cards);//searchにownを入れる
    if(state.lock==1){//縛りの場合は1パターンのみ
      for(i=0;i<4;i++){
        suit[i]=state.suit[i];
      }
    }
    else{//すべての出せるパターンを調べる
      if(state.qty==1){
        suit[0]=0;
        suit[1]=0;
        suit[2]=0;
        suit[3]=0;
        suit[suit[4]]=1;
      }
      if(state.qty==3){
        suit[0]=1;
        suit[1]=1;
        suit[2]=1;
        suit[3]=1;
        suit[suit[4]]=0;
      }
      if(state.qty==2){
        if(suit[4]<=2){
          suit[0]=1;
        }
        else{
          suit[0]=0;
        }
        if(suit[4]==0 || suit[4]==3 || suit[4]==4){
          suit[1]=1;
        }
        else{
          suit[1]=0;
        }
        if(suit[4]%2==1){
          suit[2]=1;
        }
        else{
          suit[2]=0;
        }
        if(suit[4]==2 || suit[4]==4 || suit[4]==5){
          suit[3]=1;
        }
        else{
          suit[3]=0;
        }
      }
      if(state.qty==4){
        suit[0]=1;
        suit[1]=1;
        suit[2]=1;
        suit[3]=1;
      }
    }//スートのパターンの探索ここまで
    if(pattern[12][2]>=1){
      pattern[12][2]=1;
    }
    //出せるかどうかを判定する
    for(i=0;i<4;i++){
      if(suit[i]==1 && own_cards[i][ord]==0){
        if(pattern[12][2]==0){//ジョーカーなし
          i=6;
        }
        else if(pattern[12][2]==1){//ジョーカーがある場合
          if(state.qty==1){//場の枚数が1枚
            i=6;//ジョーカー単体は別の判定をするので、ここでは選ばない
          }
          else{
            pattern[12][2]=3;//使おうと考える
          }
        }
        else if(pattern[12][2]==3){//持っていないスートが2つ以上
          pattern[12][2]=1;
          i=6;
        }
      }
    }
    if(i<=5)//出せる場合
    {
      for(i=0;i<4;i++){
        if(suit[i]==1 && own_cards[i][ord]==1){
          target_cards[i][ord]=1;
        }
        else if(suit[i]==1 && own_cards[i][ord]==0){
          target_cards[i][ord]=2;
        }
      }
      clearTable(use_own_cards);
      copyCards(use_own_cards,own_cards);//use_ownにownを入れる
      cardsDiff(use_own_cards,target_cards);//use_ownからtarget(選択した手)を除く
      clearTable(search);
      copyCards(search,used_cards);//searchにused_cards(すでに場に出たカード)を入れる
      for(i=0;i<=4;i++){//searchに出すカードを入れる(選択した手を提出した場合の場に出たカード)
        for(j=0;j<=14;j++){
          if(target_cards[i][j]==1){
            search[i][j]=1;
          }
          if(target_cards[i][j]==2){
            search[4][14]=1;
          }
        }
      }
      for(j=1;j<=13;j++){
        search[4][j]=search[0][j]+search[1][j]+search[2][j]+search[3][j];
      }
      for(i=0;i<5;i++){
        use_own_cards[6][i]=own_cards[6][i];//手札枚数の情報を入れる
      }
      use_pattern[12][2]=pattern[12][2];//ジョーカーを使用したかどうかを入れる
      pat_make(use_pattern,use_own_cards,max,search,pattern[12][2], tes);//提出した場合の手札の組を作る
      //縛りの判定
      if(ord==6){//8切りの場合
        i=6;
      }
      else if(state.rev==0 && ord>=max){//場で一番強いカードの場合
        i=6;
      }
      else if(state.rev==1 && ord<=max){
        i=6;
      }
      else if(state.lock==1){//縛り状態の場合
        i=5;//縛りの判定をする
      }
      else{//縛りでない場合
        for(i=0;i<4;i++){//出す場合,縛りになるかどうか調べる
          if(state.suit[i]!=suit[i]){//縛りにならない場合
            i=6;
          }
        }
      }
      /*
      i=5…縛りの場合の判定をする
      i=6…通常(縛りでない場合)の判定をする
      */
      if(i<=5 && state.qty!=1 && p>=3){//縛り状態か縛りの場合で場が2枚以上、相手が2人以上なら積極的に出す
        i=5;
      }
      else if(i<=5){//縛り状態か縛りにする場合
        if(state.rev==0){//通常時
          for(j=max;j>ord;j--){
            for(i=0;i<4;i++){
              if(suit[i]>0 && used_cards[i][j]+own_cards[i][j]!=0){//相手が出せる可能性があるかどうか調べる
                i=6;
              }
            }
            if(i<=5){//相手が出せる可能性がある場合
              //if(state.lock==1)
                i=6;//縛り状態のときは出す優先度は変えない
              //else
              //  i=7;//積極的には出さない
              j=0;//相手が出せる可能性があるときはすぐループを抜ける
            }
            else{//相手が出せる可能性がない場合
              for(i=0;i<4;i++){
                if(suit[i]>0 && own_cards[i][j]==0){//自分が同じスートで強いカードを持っていない場合
                  i=6;
                }
              }
              if(i<=5){//自分が同じスートで強いカードを持っている場合
                if(use_pattern[12][2]==3){
                  i=6;//ジョーカー使用の場合は出す優先度は変えない
                }
                else{
                  i=5;//積極的に出す
                }
                j=0;//ループを抜ける
              }
              else if(ord==j-1){//同じスートで強いカードがない場合(出すカードがそのスートで一番強い場合)
                i=5;//積極的に出す
                j=0;//ループを抜ける
              }
            }
          }
        }
        else{//革命時も同じように判断
          for(j=max;j<ord;j++){
            for(i=0;i<4;i++){
              if(suit[i]>0 && used_cards[i][j]+own_cards[i][j]!=0){
                i=6;
              }
            }
            if(i<=5){
              //if(state.lock==1)
                i=6;
              //else
              //  i=7;
              j=14;
            }
            else{
              for(i=0;i<4;i++){
                if(suit[i]>0 && own_cards[i][j]==0){
                  i=6;
                }
              }
              if(i<=5){
                if(use_pattern[12][2]==3){
                  i=6;
                }
                else{
                  i=5;
                }
                j=14;
              }
              else if(ord==j+1){
                i=5;
                j=14;
              }
            }
          }
        }
      }
      else{//縛りでない場合
        i=6;
      }//縛りの判定ここまで
      /*
      i=5…複数枚で縛る場合か,1枚で縛る場合で一番強いカードを持っている場合
      i=7…1枚で縛る場合で一番強いカードを持っていない場合(削除)
      i=6…上記以外
      */
      //選んだカードの評価値(x')の決定
      if(i==6){
        use_pattern[12][0]=100;//基準
        if(ord==6){//8切り
          use_pattern[12][0]=101;
        }
        else if(state.qty==1){//単体の場合、相手が出せる組があるとき-30する
          if(state.rev==0){
            for(j=max;j>ord;j--){
              if((4-used_cards[4][j]-own_num[j])>=1){
                use_pattern[12][0]-=30;
              }
            }
          }
          else{
            for(j=max;j<ord;j++){
              if((4-used_cards[4][j]-own_num[j])>=1){
                use_pattern[12][0]-=30;
              }
            }
          }
          if(used_cards[4][14]==0 && state.joker==0){
            use_pattern[12][0]-=1;
          }
        }//単体の場合の判定ここまで
        else{//複数枚の場合
          if(state.rev==0){
            for(j=max;j>ord;j--){
              if((4-used_cards[4][j]-own_num[j])==(state.qty-1) && used_cards[4][14]==0 && state.joker==0){//ジョーカーを含めると出される場合
                use_pattern[12][0]-=1;//1だけ減らす
              }
              else if((4-used_cards[4][j]-own_num[j])<=(state.qty-1)){//出されるパターンなし
                //減らさない
              }
              else{
                k=(4-used_cards[4][j]-own_num[j])-state.qty-p+5;
                if(p==2){
                  use_pattern[12][0]-=30;
                }
                else if(k<=0){use_pattern[12][0]-=4;}
                else if(k==1){use_pattern[12][0]-=9;}
                else if(k==2){use_pattern[12][0]-=15;}
                else{use_pattern[12][0]-=24;}
              }
            }
          }
          else{
            for(j=max;j<ord;j++){
              if((4-used_cards[4][j]-own_num[j])==(state.qty-1) && used_cards[4][14]==0 && state.joker==0){//ジョーカーを含めると出される場合
                use_pattern[12][0]-=1;//1だけ減らす
              }
              else if((4-used_cards[4][j]-own_num[j])<=(state.qty-1)){//出されるパターンなし
                //減らさない
              }
              else{
                k=(4-used_cards[4][j]-own_num[j])-state.qty-p+5;
                if(p==2){
                  use_pattern[12][0]-=30;
                }
                else if(k<=0){use_pattern[12][0]-=4;}
                else if(k==1){use_pattern[12][0]-=9;}
                else if(k==2){use_pattern[12][0]-=15;}
                else{use_pattern[12][0]-=24;}
              }
            }
          }
        }//複数枚の場合の判定ここまで
        if(use_pattern[12][0]<=0){
          use_pattern[12][0]=1;
        }
      }
      else if(i==5){//複数枚で縛る場合か,1枚で縛る場合で一番強いカードを持っている場合はx'=STRONG
        use_pattern[12][0]=STRONG;
      }
      else{//もし予想外の動作が起きた場合
        use_pattern[12][0]=1;//x'=1にする
      }
      //出した後の全体評価値(z')の決定
      use_pattern[12][1]=use_pattern[11][4];
      //x'によってz'を変化
      if(use_pattern[12][0]>=STRONG){//場を流せる可能性が高い場合1減らす
        use_pattern[12][1]-=1;
      }
      if(i!=5 && use_pattern[12][0]<STRONG && use_pattern[11][0]>=pattern[11][0]){//場を流せる可能性が低く、組数が減らない場合1増やす
        use_pattern[12][1]+=1;
      }
      //出すかどうか判定
      j=1;
      if(use_pattern[12][0]>=STRONG && use_pattern[11][3]>=STRONG){
        //x>=STRONG,2min>=STRONG →出す(あがりの形の場合は出す)
        j=1;
      }
      else if(use_pattern[12][0]==101 && use_pattern[11][0]<=pattern[11][0]){
        //8を含む組で組数が減るか同じ →出す(8の場合、組数が増えない場合は出す)
        j=1;
      }
      else if(use_pattern[12][0]>STRONG && use_pattern[11][0]>=4 && use_pattern[11][0]-1<=use_pattern[12][1]){
        //x>=STRONG,組数4以上,組数-1<=z'→出さない(場を流せる場合で、他に強い組がない場合出さない)
        j=0;
      }
      else if(use_pattern[11][2]<=50 && use_pattern[11][3]==201 && use_pattern[12][0]<=60){
        j=0;//1min<=50,2min=201,x'<=60 →出さない(残り1組の場合で場を流せる可能性が低い場合は出さない)
      }
      else if(use_pattern[12][2]==3 && use_pattern[12][0]<=STRONG ){
        //ジョーカー使用の場合でx<=STRONG(2min>=STRONG,x=STRONGは出す) →出さない
        j=0;
      }
      else if(use_pattern[11][3]>=STRONG){
        //2min>=STRONG →出す(↑の場合以外で、あがりの形になる場合は出す)
        j=1;
      }
      //else if(state.qty==1 && use_pattern[12][0]==31 && use_pattern[12][1]<=pattern[11][4]){
      //  j=2;//縛りで強いカードを持っていない場合は出すが、優先度は低くする
      //}
      else if(state.qty==1 && use_pattern[12][1]>=4 && use_pattern[12][0]>=40 && use_pattern[12][0]<=70){
        j=0;//qty=1,z'>=4,40<=x'<=70 →出さない(手札が多いうちは単体のKやAは出さない)
      }
      else if(use_pattern[12][1]>pattern[11][4]){
        //z'>z →出さない(↑以外で、zが大きくなる場合は出さない)
        j=0;
      }
      else{
        j=1;//組数が減り,zが大きくならない場合は大体出す
      }
      if(PRINT_PAT==2){
        fprintf( stderr, "ord%2d,suit%4d ",ord,(suit[0]*1000)+(suit[1]*100)+(suit[2]*10)+suit[3]);
        fprintf( stderr, "1min%3d,2min%3d ",use_pattern[11][2],use_pattern[11][3],use_pattern[11][4]);
        fprintf( stderr, "x'%3d\n",use_pattern[12][0]);
        fprintf( stderr, "pair%2d->%2d,z'%3d->%3d [check%2d]\n",pattern[11][0],use_pattern[11][0],pattern[11][4],use_pattern[12][1],j);
      }
      if(beEmptyCards(select_cards)==1 && j>=1){//これまでの判定で出す候補がない場合
          copyCards(select_cards,target_cards);//出す判定
          pattern[12][0]=use_pattern[12][0];
          pattern[12][1]=use_pattern[12][1];//+j-1
      }
      else if(j>=1){//出す候補がある場合
        if(use_pattern[11][3]>=STRONG && pattern[12][0]>=STRONG && use_pattern[12][0]<STRONG){
          //2min>=STRONG,変更前の手のx>=STRONG 変更後の手のx<STRONG→出さない
        }
        else if(use_pattern[11][3]>=STRONG && use_pattern[12][0]>=STRONG && pattern[12][0]<STRONG){
          //2min>=STRONG,変更後の手のx>=STRONG,変更前の手のx<STRONG →出す
          clearCards(select_cards);
          copyCards(select_cards,target_cards);//出す判定
          pattern[12][0]=use_pattern[12][0];
          pattern[12][1]=use_pattern[12][1];
        }
        else if(use_pattern[11][3]>=STRONG && use_pattern[12][0]>STRONG && pattern[12][0]==STRONG){
          //2min>=STRONG,変更後の手のx>STRONG,変更前の手のx=STRONG →出す(あがり判定のときは「縛り」ではなく、強いカードで場を流す)
          clearCards(select_cards);
          copyCards(select_cards,target_cards);//出す判定
          pattern[12][0]=use_pattern[12][0];
          pattern[12][1]=use_pattern[12][1];
        }
        else if((use_pattern[12][1]+j-1)<pattern[12][1]){
          //変更後の手のz<変更前の手のz →出す
          clearCards(select_cards);
          copyCards(select_cards,target_cards);//出す判定
          pattern[12][0]=use_pattern[12][0];
          pattern[12][1]=use_pattern[12][1];
        }
      }
    }//選んだカードが出せる場合の判定ここまで
//次のスートの組み合わせを出す
    clearTable(target_cards);
    suit[4]++;
    if(state.lock==1){
      suit[4]=0;
    }
    else if(state.qty==1 || state.qty==3){
      if(suit[4]==4){
        suit[4]=0;
      }
    }
    else if(state.qty==2 && suit[4]==6){
      suit[4]=0;
    }
    else if(state.qty==4){
      suit[4]=0;
    }
    if(suit[4]==0 && state.rev==0){//スートの組み合わせを調べ終わったら次の数字にする
      ord++;
    }
    else if(suit[4]==0){
      ord--;
    }
    if(ord>13 || ord<1){//出せる組み合わせを調べ終わったらループを抜ける
      card_flag=0;
    }
  }//ジョーカー単体以外の判定終了

//場のカードが1枚のときジョーカー単体について考える
  if(state.qty==1 && state.joker==1){
    //use_pattern[12][2]=3;//ジョーカー使用
    pat_make(use_pattern,own_cards,max,used_cards,0, tes);//ジョーカー使用時のパターンを作る
    if((pattern[11][0]>=use_pattern[11][0] && state.ord==max) || use_pattern[11][3]>=STRONG){//場の数字が最大で組数が増えない または 2min>=STRONG
      if(used_cards[0][1]+own_cards[0][1]==1){//スペード3が出ていない
        if(beEmptyCards(select_cards)==1 && j>=1){//これまでの判定で出す候補がない場合
          clearTable(select_cards);
          select_cards[0][0]=2;//出す
        }
        else if(pattern[12][0]<STRONG){
          //選択していた手のx<STRONG →出す
          clearTable(select_cards);
          select_cards[0][0]=2;//出す
        }
      }
    }
  }
}

void kou_followsequence(int select_cards[][15],int own_cards[8][15],int max,int used_cards[8][15],int tes[3][4][3]){
//階段で出すときの思考
  int i,j;
  int suit,ord;//現在見ているスート,強さ
  int count;
  int jk=-1;//ジョーカー位置
  int card_flag=1;//出せるカードがある間1
  int target_cards[8][15];//出すカードの優先候補を入れる
  int use_own_cards[8][15];//出した後の状態を入れる
  int search[8][15];
  int pattern[13][7]={{0}};//カードを選ぶ前の手札のパターン
  int use_pattern[13][7]={{0}};//カードを選び出した後の手札のパターン
  pat_make(pattern,own_cards,max,used_cards,state.joker,tes);//現在の手札のパターンを作る
  int seq_max;
  int seq_min;
  if(state.rev==0){
    seq_max=14;
    seq_min=state.ord+state.qty;
  }
  else{
    seq_max=state.ord-1;
    seq_min=state.qty-1;
  }
  clearTable(select_cards);
  clearTable(target_cards);
  clearTable(search);

    for(suit=0;suit<4;suit++){
      if(state.lock==1){//縛りのとき
        while(state.suit[suit]!=1 && suit<4){
          suit++;
        }
      }
      for(ord=seq_max;ord>=seq_min && suit<4;ord--){
        jk=-1;
        count=0;
        clearTable(target_cards);
        while(count!=state.qty && (ord-count)>=0){
          if(own_cards[suit][ord-count]==1){
            target_cards[suit][ord-count]=1;
            count++;
          }
          else if(jk==-1 && own_cards[4][1]==2){
            target_cards[suit][ord-count]=2;
            jk=ord-count;
            count++;
          }
          else{
            break;
          }
        }
        if(count==state.qty){//階段発見の場合

      clearTable(use_own_cards);
      copyCards(use_own_cards,own_cards);//use_ownにownを入れる
      cardsDiff(use_own_cards,target_cards);//use_ownからtarget(選択した手)を除く
      clearTable(search);
      copyCards(search,used_cards);//searchにused_cards(すでに場に出たカード)を入れる
      for(i=0;i<=4;i++){//searchに出すカードを入れる(選択した手を提出した場合の場に出たカード)
        for(j=0;j<=14;j++){
          if(target_cards[i][j]==1){
            search[i][j]=1;
          }
          if(target_cards[i][j]==2){
            search[4][14]=1;
          }
        }
      }
      for(j=1;j<=13;j++){
        search[4][j]=search[0][j]+search[1][j]+search[2][j]+search[3][j];
      }
      for(i=0;i<5;i++){
        use_own_cards[6][i]=own_cards[6][i];//手札枚数の情報を入れる
      }
      if(jk!=-1){//ジョーカーを使用した場合
        use_pattern[12][2]=3;
      }
      pat_make(use_pattern,use_own_cards,max,search,use_pattern[12][2],tes);//提出した場合の手札のパターンを作る
        //出すかどうか判定
        j=0;
        if(use_pattern[11][3]>=STRONG){
          //2min>=STRONG →出す(あがりの形になる場合は出す)
          j=1;
        }
        else if(use_pattern[11][4]<=pattern[11][4]+1){
          j=1;
        }
        if(beEmptyCards(select_cards)==1 && j>=1){//これまでの判定で出す候補がない場合
          copyCards(select_cards,target_cards);//出す判定
          //pattern[12][0]=99;
          pattern[12][1]=use_pattern[11][4];
        }
        else if(j>=1){//出す候補がある場合
          if(use_pattern[12][1]<pattern[12][1]){
            //変更後の手のz<変更前の手のz →出す
            clearCards(select_cards);
            copyCards(select_cards,target_cards);//出す判定
            //pattern[12][0]=99;
            pattern[12][1]=use_pattern[11][4];
          }
        }

        }//階段発見の場合ここまで
      }
    }
}

void kou_change(int out_cards[8][15],int my_cards[8][15],int num_of_change,int tes[3][4][3]){
//カード交換の判定
  int i,j,k;
  int search[8][15];
  int cards[15][3]={{0}};//[0]強さ[1]スート(0,1,2,3)[2]手札から選んだカードを除いた場合のz
  int used_cards[8][15]={{0}};//提出後のカード 何も入ってないので0
  int pattern[13][7]={{0}};
  clearTable(out_cards);
  clearTable(search);
  int c;
  k=0;
  for(j=1;j<=12;j++){//3からAまで
    for(i=0;i<4;i++){
      if(my_cards[i][j]==1){
        cards[k][0]=j;
        cards[k][1]=i;
        k++;
      }
    }
  }
  for(i=0;i<4;i++){
    my_cards[i][13]=0;
  }
  for(i=0;i<k;i++){
    clearTable(search);
    copyTable(search,my_cards);
    search[cards[i][1]][cards[i][0]]=0;
    pat_make(pattern,search,13,used_cards,state.joker,tes);
    cards[i][2]=(pattern[11][4]*10)+cards[i][0];
    if(cards[i][0]==1){
      cards[i][2]+=5;
      if(cards[i][1]==0 || cards[i][2]==2){
        cards[i][2]+=20;
      }
    }
    if(cards[i][0]==6){
      cards[i][2]+=30;
    }
    if(cards[i][0]==11){
      cards[i][2]+=15;
    }
    if(cards[i][0]==12){
      cards[i][2]+=30;
    }
  }
  for(i=0;i<k;i++){
    for(j=i;j>0;j--){
      if(cards[j][2]<cards[j-1][2]){
        cards[14][0]=cards[j][0];
        cards[14][1]=cards[j][1];
        cards[14][2]=cards[j][2];
        cards[j][0]=cards[j-1][0];
        cards[j][1]=cards[j-1][1];
        cards[j][2]=cards[j-1][2];
        cards[j-1][0]=cards[14][0];
        cards[j-1][1]=cards[14][1];
        cards[j-1][2]=cards[14][2];
      }
    }
  }
  out_cards[cards[0][1]][cards[0][0]]=1;
  if(num_of_change==2){
    out_cards[cards[1][1]][cards[1][0]]=1;
  }
}

void pat_make(int pattern[][7],int own_cards[8][15],int max,int used_cards[8][15],int joker_flag,int tes[3][4][3]){
//手札の組作りと評価値の決定
            int i,j,k;
            int select_cards[8][15];
            int search[8][15];
            int t;//0:前半1:中2:後半
            clearTable(search);
            for(i=0;i<13;i++){
              for(j=0;j<7;j++){
                pattern[i][j]=0;
              }
            }
            int own_cards_copy[8][15];//探索用手札テーブル
            copyTable(own_cards_copy,own_cards);
            if(joker_flag==2){//フラグが2(念の為)の場合1
              joker_flag=1;
            }
            if(joker_flag==3){//フラグが3(提出手で使用)の場合0
              joker_flag=0;
            }
            //カードの組を作る
            i=0;
            j=1;
            while(j!=0){
              k=setmake(search,own_cards_copy,joker_flag);
              j=kaidanhand(select_cards,search,own_cards_copy);//階段を作る
              if(j!=0){//階段
                cardsDiff(own_cards_copy,select_cards);
                pattern[i][0]=j;
                pattern[i][3]=0;
                for(pattern[i][2]=0;select_cards[pattern[i][2]][j]==0;pattern[i][2]++){
                  //スートを調べる
                }
                for(;j>=0;j--){
                  if(select_cards[pattern[i][2]][j]==2){
                    pattern[i][3]=j;
                    pattern[12][2]=2;
                    joker_flag=0;
                  }
                  else if(select_cards[pattern[i][2]][j]==0){
                    pattern[i][1]=pattern[i][0]-j;
                    j=0;
                  }
                }
                if(pattern[i][2]==0){pattern[i][2]=1;}
                else if(pattern[i][2]==1){pattern[i][2]=2;}
                else if(pattern[i][2]==2){pattern[i][2]=4;}
                else if(pattern[i][2]==3){pattern[i][2]=8;}
                if(pattern[i][3]==0){
                  pattern[i][3]=15;
                }
                i++;
                j=-1;
              }
              else{//枚数組
                clearTable(select_cards);
                j=grouphand(select_cards,search,0);//弱い枚数組を作る

                if(j>=1){//8,強カード以外の枚数組
                  cardsDiff(own_cards_copy,select_cards);
                  pattern[i][0]=j;
                  pattern[i][1]=select_cards[0][j]+select_cards[1][j]+select_cards[2][j]+select_cards[3][j];
                  pattern[i][3]=0;
                  if(pattern[i][1]==4 && state.rev==0 && j>=max){//強カード4枚の場合,2枚ずつの組にする
                    pattern[i][1]=2;
                    pattern[i][2]=3;
                    i++;
                    pattern[i][0]=j;
                    pattern[i][1]=2;
                    pattern[i][2]=12;
                    pattern[i][3]=0;
                  }
                  else if(pattern[i][1]==4 && state.rev==1 && j<=max){//強カード4枚の場合(革命時)
                    pattern[i][1]=2;
                    pattern[i][2]=3;
                    i++;
                    pattern[i][0]=j;
                    pattern[i][1]=2;
                    pattern[i][2]=12;
                    pattern[i][3]=0;
                  }
                  else{
                    pattern[i][2]=0;
                    if(select_cards[0][j]==1){pattern[i][2]+=1;}
                    if(select_cards[1][j]==1){pattern[i][2]+=2;}
                    if(select_cards[2][j]==1){pattern[i][2]+=4;}
                    if(select_cards[3][j]==1){pattern[i][2]+=8;}
                  }
                  i++;
                  j=-1;
                }
                if(j==0 && joker_flag==1){//ジョーカー単体
                  pattern[i][1]=1;
                  pattern[i][3]=14;
                  pattern[12][2]=1;
                  joker_flag=0;
                  i++;
                }
              }
              clearTable(search);
              clearTable(select_cards);
            }
//組数格納
            for(i=0;i<11;i++){
              if(pattern[i][1]==0){
                pattern[11][0]=i;
                i=12;
              }
            }
            if(i==11){
              pattern[11][0]=11;
            }
//x,revx,z,revz,1min,2minの決定
            value_strong(pattern,own_cards,used_cards,state.rev,4,tes);
            value_strong(pattern,own_cards,used_cards,(state.rev+1)%2,5,tes);
//評価値yの決定
            for(i=0;i<pattern[11][0];i++){
              if(pattern[i][3]==14){
                pattern[i][6]=3;
              }
              else if(pattern[i][4]>=STRONG && pattern[i][4]<=100){//流れる可能性が高い場合
                pattern[i][6]=110-pattern[i][4];//10～15
              }
              else if(pattern[i][4]>100 && pattern[i][3]!=0){//階段
                pattern[i][6]=9;
              }
              else if(pattern[i][4]>=1 && pattern[i][4]<=200){
                if(state.rev==0){
                  pattern[i][6]=40+(2*(14-pattern[i][0]));//44～64
                }
                else{
                  pattern[i][6]=40+(2*pattern[i][0]);//44～64
                }
                if(pattern[i][4]>100){//8が含まれる場合
                  pattern[i][6]+=8;
                }
                else{//含まれない場合
                  for(j=0;j<11;j++){
                    if(pattern[i][1]==pattern[j][1] && pattern[i][3]==0 && pattern[j][3]==0){
                      pattern[i][6]+=4;
                    }
                  }
                }
              }
              else{
                pattern[i][6]=-1;
              }
            }
//最弱カードの評価値yの変化+followで出せない組の評価値yの変化
            if(pattern[11][0]>=5){
              if(state.rev==0){
                for(i=0;i<11;i++){
                  if(pattern[i][0]==1){
                    pattern[i][6]-=7;
                    if(pattern[11][pattern[11][0]-1]>=100 || pattern[11][pattern[11][0]-2]>=100){
                      pattern[i][6]-=8;
                    }
                  }
                  else if(pattern[i][5]>=97 && pattern[i][5]<=100){
                    pattern[i][6]+=13;
                  }
                }
              }
              else if(state.rev==1){
                for(i=0;i<11;i++){
                  if(pattern[i][0]==13){
                    pattern[i][6]-=7;
                    if(pattern[11][pattern[11][0]-1]>=100 || pattern[11][pattern[11][0]-2]>=100){
                      pattern[i][6]-=8;
                    }
                  }
                  else if(pattern[i][5]>=97 && pattern[i][5]<=100){
                    pattern[i][6]+=13;
                  }
                }
              }
            }
//リスク(revxが高い組のみの場合,勝てる手札と判断しない)
            if(pattern[11][3]>=STRONG && pattern[11][3]<=94){
              j=0;
              for(i=0;i<5;i++){
                if(own_cards[6][i]>0){
                  j++;
                }
              }
              if(j>=4){
                j=202;
                for(i=0;i<pattern[11][0];i++){
                  if(j>pattern[i][5]){
                    j=pattern[i][5];
                  }
                }
                if(j>50){
                  pattern[11][3]=STRONG-1;
                }
              }
            }
//2minによる評価値yの変化
            if(pattern[11][3]>=STRONG){
              for(i=0;i<11;i++){
                if(pattern[i][4]>=1 && pattern[i][4]<STRONG){
                  pattern[i][6]=2;
                }
                else if(pattern[i][3]==14 && pattern[i][2]==0){
                  if(pattern[i][4]==100){
                    pattern[i][6]=3;
                  }
                  else{
                    pattern[i][6]=1;
                  }
                }
                if(pattern[i][4]>=STRONG){
                  if(pattern[11][3]>=100){
                    pattern[i][6]=pattern[i][4]-60;
                    if(pattern[i][4]>100){
                      pattern[i][6]=41;
                    }
                  }
                  else{
                    pattern[i][6]=110-pattern[i][4];
                    if(pattern[i][4]>100){
                      pattern[i][6]=9;
                    }
                  }
                }
              }
            }
            else if(pattern[11][6]>=STRONG && pattern[11][1]==1){
              pattern[11][1]=0;//leadの判定変更
              if(pattern[i][1]>=5 || (pattern[i][1]==4 && (pattern[i][3]==0 || pattern[i][3]==14))){//革命手
                pattern[i][6]=9;
              }
              else if(pattern[i][4]>100){//8切り
                pattern[i][6]=7;
              }
              else if(pattern[i][4]>=STRONG && pattern[i][4]>pattern[i][5] ){//x>=STRONG かつ x>revx
                pattern[i][6]=110-pattern[i][4];
              }
              else{
                pattern[i][6]=8;
              }
            }
            if(pattern[11][0]>=3 && pattern[pattern[11][0]-1][3]==14 && pattern[pattern[11][0]-1][2]==0){
               pattern[11][0]-=1;//ジョーカー単体は組数に含めない
            }
}

void value_strong(int pattern[][7],int own_cards[8][15],int used_cards[8][15],int rev,int n,int tes[3][4][3]){//強さ評価値と全体評価値計算
  //x,revx,z,revz,1min,2min
  //nはpatternへの格納位置(4か5)
  if(n!=4 && n!=5){
    n=5;
  }
  int i,j,k,jk,suit,t;
  int p=0;//残り人数
  //ゲームの前後半を記録
  j=used_cards[5][1];
  if(j>=0 && j<13)t=0;
  else if(j>=13 && j<32)t=1;
  else if(j>=32)t=2;
  for(i=0;i<5;i++){
    if(own_cards[6][i]>0){
      p++;
    }
  }
  int own_num[15]={0};//自分のカードの枚数を記録
  for(i=1;i<=13;i++){
    own_num[i]=own_cards[0][i]+own_cards[1][i]+own_cards[2][i]+own_cards[3][i];
  }
  int max=0;
  if(rev==0){
    for(i=13;i>0 && max==0;i--){
      if(own_num[i]+used_cards[4][i]!=4){//同じ強さのカードが他の手札にあれば
        max=i;//その強さを記録
      }
    }
  }
  else{
    for(i=1;i<14 && max==0;i++){
      if(own_num[i]+used_cards[4][i]!=4){//同じ強さのカードが他の手札にあれば
        max=i;//その強さを記録
      }
    }
  }
  if(max==0){
    if(rev==0){
      max=1;
    }
    else{
      max=13;
    }
  }

  for(i=0;i<11;i++){
    if(pattern[i][3]!=14 && pattern[i][3]!=0){
    //階段の場合
      if(rev==0){//通常時
        pattern[i][n]=100;
        for(j=pattern[i][0]+pattern[i][1];j<=(max+1);j++){
          for(suit=0;suit<4;suit++){
            jk=1-used_cards[4][14]-state.joker;//ジョーカーが含まれる可能性
            for(k=0;k<pattern[i][1];k++){
              if(used_cards[suit][j-k]==1 || own_cards[suit][j-k]==1){
                if(jk==1){
                  jk=0;
                }
                else{
                  jk=2;
                }
              }
            }
            if(jk!=2){
              if(p==2){
                pattern[i][n]-=15;
              }
              else if(jk==0){
                pattern[i][n]-=1;
              }
              else{
                pattern[i][n]-=6-p;
              }
            }
          }
        }
      pattern[i][n]+=tes[t][3][2]/10;
      }
      else{//革命時
        pattern[i][n]=100;
        for(j=pattern[i][0]-(2*pattern[i][1])+1;j>=(max-1);j--){//j=カード強さ-(2*枚数)+1=上に出すときに必要な強さ(3に近い方)
          for(suit=0;suit<4;suit++){
            jk=1-used_cards[4][14]-state.joker;
            for(k=0;k<pattern[i][1];k++){
              if(used_cards[suit][j+k]==1 || own_cards[suit][j+k]==1){
                if(jk==1){
                  jk=0;
                }
                else{
                  jk=2;
                }
              }
            }
            if(jk!=2){
              if(p==2){
                pattern[i][n]-=15;
              }
              else if(jk==0){
                pattern[i][n]-=1;
              }
              else{
                pattern[i][n]-=6-p;
              }
            }
          }
        }
      pattern[i][n]+=tes[t][3][2]/10;
      }
      if(pattern[i][0]>=6 && (pattern[i][0]-pattern[i][1]<6)){//8を含む場合
        pattern[i][n]+=100;
      }
    }
    else if(pattern[i][3]==14 && pattern[i][2]==0){
    //ジョーカー単体
      if(used_cards[0][1]+own_cards[0][1]==1){//スペード3の可能性なし
        pattern[i][n]=100;
        pattern[i][n]+=tes[t][0][2]/10;
      }
      else{
        pattern[i][n]=1;
      }
    }
    else if(pattern[i][0]==0){
    //カード無し
      pattern[i][0]=0;
      pattern[i][1]=0;
      pattern[i][2]=0;
      pattern[i][3]=0;
      pattern[i][n]=-1;
    }
    else{
    //枚数組
      pattern[i][n]=100;//基準
      if(pattern[i][1]==1){
      //単体の場合、相手が出せる組があるとき-30する
        if(rev==0){
          for(j=max;j>pattern[i][0];j--){
            if((4-used_cards[4][j]-own_num[j])>=1){
              pattern[i][n]-=30;
            }
          }
        }
        else{
          for(j=max;j<pattern[i][0];j++){
            if((4-used_cards[4][j]-own_num[j])>=1){
              pattern[i][n]-=30;
            }
          }
        }
        if(used_cards[4][14]==0 && state.joker==0){//ジョーカーがある場合
          pattern[i][n]-=1;
        }
        pattern[i][n]+=tes[t][1][2]/10;
      }
      else{
      //複数枚の場合
        if(rev==0){
          for(j=max;j>pattern[i][0];j--){
            if((4-used_cards[4][j]-own_num[j])==(pattern[i][1]-1) && used_cards[4][14]==0 && state.joker==0){//ジョーカーを含めると出される場合
              pattern[i][n]-=1;
            }
            else if((4-used_cards[4][j]-own_num[j])<=(pattern[i][1]-1)){//出される可能性なし
              //減らさない
            }
            else{
              k=(4-used_cards[4][j]-own_num[j])-pattern[i][1]-p+5;
              if(p==2){
                pattern[i][n]-=30;
              }
              else if(k<=0){pattern[i][n]-=4;}
              else if(k==1){pattern[i][n]-=9;}
              else if(k==2){pattern[i][n]-=15;}
              else{pattern[i][n]-=24;}
            }
          }
          pattern[i][n]+=tes[t][2][2]/10;
        }
        else{
          for(j=max;j<pattern[i][0];j++){
            if((4-used_cards[4][j]-own_num[j])==(pattern[i][1]-1) && used_cards[4][14]==0 && state.joker==0){//ジョーカーを含めると出される場合
              pattern[i][n]-=1;
            }
            else if((4-used_cards[4][j]-own_num[j])<=(pattern[i][1]-1)){//出される可能性なし
              //減らさない
            }
            else{
              k=(4-used_cards[4][j]-own_num[j])-pattern[i][1]-p+5;
              if(p==2){
                pattern[i][n]-=30;
              }
              else if(k<=0){pattern[i][n]-=4;}
              else if(k==1){pattern[i][n]-=9;}
              else if(k==2){pattern[i][n]-=15;}
              else{pattern[i][n]-=24;}
            }
          }
          pattern[i][n]+=tes[t][2][2]/10;
        }
      }
      if(pattern[i][n]<=0){
        pattern[i][n]=1;
      }
      if(pattern[i][0]==6){//8切り
        pattern[i][n]+=100;
      }
    }
  }//x計算ここまで

  for(i=0;i<11;i++){
    if(n==4){//1min,2minの決定
      if(i==0){
        pattern[11][2]=pattern[i][4];
        pattern[11][3]=201;
      }
      else if(pattern[i][4]==-1 || (pattern[i][3]==14 && pattern[i][2]==0) ){//ジョーカー単体
      }
      else{
        if(pattern[i][4]<pattern[11][2]){
          pattern[11][3]=pattern[11][2];
          pattern[11][2]=pattern[i][4];
        }
        else if(pattern[i][4]<pattern[11][3]){
          pattern[11][3]=pattern[i][4];
        }
      }
    }
    if(n==5){//rev2mの決定
      if(i==0){
        j=pattern[i][5];
        pattern[11][6]=201;
      }
      else if(pattern[i][4]>=STRONG && pattern[i][4]>pattern[i][5] ){//x>=STRONG かつ x>revx
      }
      else if(pattern[i][1]>=5 || (pattern[i][1]==4 && (pattern[i][3]==0 || pattern[i][3]==14))){//革命手
      }
      else{
        if(pattern[i][5]<j){
          pattern[11][6]=j;
          j=pattern[i][5];
        }
        else if(pattern[i][5]<pattern[11][6]){
          pattern[11][6]=pattern[i][5];
        }
      }
    }
    //z,revzの決定
    if(i==0){
      pattern[11][n]=0;
    }
    if(i!=13){
      if(pattern[i][1]<=3 && pattern[i][1]!=0){
        if(pattern[i][n]>=1 && pattern[i][n]<=30){
          pattern[11][n]+=2;
        }
        else if(pattern[i][n]>=31 && pattern[i][n]<=60){
          pattern[11][n]+=1;
        }
        else if(pattern[i][n]>=61 && pattern[i][n]<=90){
          //pattern[11][n]+=0;
        }
        else if(pattern[i][n]>=91 && pattern[i][n]<=200){
          if(rev==0 && pattern[i][0]>=max){//一番強い数字
            pattern[11][n]-=pattern[i][1];
          }
          else if(rev==1 && pattern[i][0]<=max){
            pattern[11][n]-=pattern[i][1];
          }
          else{
            pattern[11][n]-=1;
          }
        }
      }
      if(pattern[i][3]==14 && pattern[i][2]==0){//ジョーカー
        pattern[11][n]-=2;
      }
    }
  }
}

void cardprint(int own_cards[8][15]){
	int i,j;
	for(j=0;j<15;j++){
		for(i=0;i<4;i++){
			if(own_cards[i][j]>=1){
				printf(" ");
				if(own_cards[i][j]==2){
					printf("[jk]");
				}
				switch(i){
				case 0:printf("s");break;
				case 1:printf("h");break;
				case 2:printf("d");break;
				case 3:printf("c");break;
				}
				if(j>=1 && j<=8)printf("%d",j+2);
				else if(j==9)printf("J");
				else if(j==10)printf("Q");
				else if(j==11)printf("K");
				else if(j==12)printf("A");
				else if(j==13)printf("2");
				else if(j==0)printf("3-");
				else if(j==14)printf("2+");
			}
		}
	}
	if(own_cards[4][1]==2){
		printf(" jk");
	}
	printf("\n");
}
void Tableprint(int own_cards[8][15],int print){
	//print 0:カードテーブルのみ 1:状態も表示
	int i,j,k;
	k=5;
	if(print==1){
		k=7;
	}
	  fprintf( stderr, "    [3-][ 3][ 4][ 5][ 6][ 7][ 8][ 9][10][ J][ Q][ K][ A][ 2][2+]\n");
      for(i=0;i<k;i++){
        if(i==0){
          fprintf( stderr, "[ S]");
        }
        if(i==1){
          fprintf( stderr, "[ H]");
        }
        if(i==2){
          fprintf( stderr, "[ D]");
        }
        if(i==3){
          fprintf( stderr, "[ C]");
        }
        if(i==4){
          fprintf( stderr, "[jk]");
        }
        if(i==5){
          fprintf( stderr, "[ba]");
        }
        if(i==6){
          fprintf( stderr, "[ps]");
        }
        for(j=0;j<=14;j++){
          if(own_cards[i][j]==0){fprintf( stderr, "    ");}
          else{
            fprintf( stderr, " %2d ",own_cards[i][j]);
          }
        }
        fprintf( stderr, "\n");
      }
}
void set(pattern *pat,int own_cards_ad[8][15]){
  /*
    持ち札から作成できる手役すべてをpatにセット
  */
  int i,j,m;
  int k,l=0;
  int c=0;
  int joker, suits;
  int own_cards[8][15];
  copyTable(own_cards,own_cards_ad);//関数を使用して持ち札の中身のみコピー

  /*
  //デバッグ用,パラメータ
  clearCards(own_cards);
  own_cards[0][11]=1;
  own_cards[0][12]=1;
  own_cards[0][13]=1;
  own_cards[1][11]=1;
  own_cards[1][12]=1;
  own_cards[1][13]=1;
  own_cards[4][1]=2;//ジョーカフラグ。ここは2でフラグをたてる
  own_cards[5][6]=1;//革命フラグ
		     */

  for(i=0;i<128;i++){//初期化
    pat[i].num=0;
    pat[i].suits=0;
    pat[i].count=0;
    pat[i].joker=0;
    pat[i].jnum=0;
    pat[i].jsuits=0;
    pat[i].x=0;
  }
  c++;
/*---------------------------------------------*/
/*------------------単体を走査------------------*/
/*---------------------------------------------*/
  for(i=0;i<4;i++){
    for(j=1;j<14;j++){
      if(own_cards[i][j]>0){
        pat[c].num=j;
        pat[c].suits=conv2to10(i);
        pat[c].count=1;
	pat[c].x=j;
	for(m=0;m<4;m++){
	  if(m==i)continue;
	  if(own_cards[m][j]>0)pat[c].x=-1;
	}
        c++;
      }
    }
  }
  if(own_cards[4][1]>0){//ジョーカー所持の場合
    pat[c].num=14;
    pat[c].suits=15;
    pat[c].count=1;
    pat[c].joker=1;
    pat[c].jnum=14;
    pat[c].jsuits=15;
    pat[c].x=14;
    c++;
  }
/*---------------------------------------------*/
/*------------------階段を走査------------------*/
/*---------------------------------------------*/
  if(own_cards[4][1]>0){//ジョーカー所持の場合
    for(i=0;i<4;i++){//ジョーカーを基点に回して走査する
      for(j=0;j<15;j++){
	joker=own_cards[i][j];//この後のループのため、ジョーカーの変化する位置に１をたてる。あとで元に戻す
	own_cards[i][j]=1;
	for(k=0;own_cards[i][j-k]>0 && j-k>=0;k++){//ジョーカーから後ろ方向の探索
	  for(l=0;own_cards[i][j+l]>0;l++){//ジョーカーから前方向の探索
	    if(k+l+1>2){//使用カード枚数が3枚以上の場合
	      if(own_cards[5][6]>0)pat[c].num=j+l;//革命発生時は逆
	      else pat[c].num=j-k;//階段の中で最弱カードがその階段の強さとなるためkを代入
	      pat[c].suits=conv2to10(i);
	      pat[c].count=k+l+1;
	      pat[c].joker=1;
	      pat[c].jnum=j;
	      pat[c].jsuits=pat[c].suits;
	      pat[c].x=pat[c].num;
	      c++;
	    }
	  }
	}
	own_cards[i][j]=joker;//ループ用に立てた1を元に戻す
      }
    }
  }
  else{
    for(i=0;i<4;i++){//ジョーカー未所持の場合
      for(j=1;j<12;j++){
        if(own_cards[i][j]>0 && own_cards[i][j+1]>0 && own_cards[i][j+2]>0){
          if(own_cards[5][6]>0)pat[c].num=j+2;
	  else pat[c].num=j;
          pat[c].suits=conv2to10(i);
          pat[c].count=3;
          pat[c].x=pat[c].num;
          c++;
          k=3;
          while(own_cards[i][j+k]>0){
	    if(own_cards[5][6]>0)pat[c].num=j+k;
            else pat[c].num=j;
            pat[c].suits=conv2to10(i);
            pat[c].count=k+1;
            pat[c].x=pat[c].num;
            c++;
            k++;
          }
        }
      }
    }
  }

/*---------------------------------------------*/
/*-------------------ペアを走査------------------*/
/*---------------------------------------------*/

  if(own_cards[4][1]>0){//ジョーカー所持の場合
    
    for(i=0;i<4;i++){//最初に単体+ジョーカーのペアを作成
      for(j=1;j<14;j++){
	if(own_cards[i][j]>0){
	  suits=conv2to10(i);
	  c=make2to1(suits,j,c,pat);
	}
      }
    }
    
    for(j=1;j<14;j++){
      if(own_cards[0][j]+own_cards[1][j]+own_cards[2][j]+own_cards[3][j]>3){//革命手を走査
        pat[c].num=j;
        pat[c].suits=15;
        pat[c].count=4;
        pat[c].x=j;
        c++;
        pat[c].num=j;//革命手にジョーカーを含めた場合
        pat[c].suits=15;
        pat[c].count=5;
        pat[c].joker=1;
        pat[c].jnum=j;
	pat[c].jsuits=15;
        pat[c].x=j;
        c++;
	for(i=0;i<4;i++){//革命手一枚をジョーカーに変更
	  pat[c].num=j;
	  pat[c].suits=15;
	  pat[c].count=4;
	  pat[c].x=j;
	  pat[c].joker=1;
	  pat[c].jnum=j;
	  pat[c].jsuits^=conv2to10(i);
	  c++;
	}
        for(i=0;i<4;i++){//革命手を崩してできる3ペアを作成
          pat[c].num=j;
          pat[c].suits=15;
          pat[c].count=3;
          pat[c].x=-1;
          pat[c].suits^=conv2to10(i);//排他的論理和で革命手から作成できる3ペアのスートを抽出
          c++;
          pat[c].num=j;
          pat[c].jsuits=conv2to10(i);
          pat[c].count=4;
          pat[c].joker=1;
          pat[c].jnum=j;
          pat[c].suits=15;
          pat[c].x=-1;
          c++;
        }
        for(i=0;i<3;i++){//革命手を崩してできる2ペアを作成
          for(k=i+1;k<4;k++){
            pat[c].num=j;
            pat[c].suits=15;
            pat[c].count=2;
            pat[c].x=-1;
            pat[c].suits^=(conv2to10(i) | conv2to10(k));//論理和と排他的論理和で革命手から作成できる2ペアのスートを抽出
            suits=pat[c].suits;
            c++;
            c=make3to2(suits,j,c,pat);
          }
        }
        for(i=0;i<4;i++){//革命手で使用したカードを除外
          own_cards[i][j]=0;
        }
      }
    }
    for(j=1;j<14;j++){
      if(own_cards[0][j]+own_cards[1][j]+own_cards[2][j]+own_cards[3][j]>2){//3ペアを走査
        pat[c].num=j;
        pat[c].suits=15;
        pat[c].count=3;
        pat[c].x=-1;

        for(i=0;i<4;i++){
          if(own_cards[i][j]==0){
            pat[c].suits^=conv2to10(i);//排他的論理和で3ペアのスートを抽出
            c++;
            break;
          }
        }
        for(k=0;k<4;k++){//3ペアを崩してできる2ペアを作成
          if(k==i)continue;
          pat[c].num=j;
          pat[c].suits=15;
          pat[c].count=2;
          pat[c].x=-1;

          pat[c].suits^=(conv2to10(i) | conv2to10(k));//論理和と排他的論理和で3ペアから作成できる2ペアのスートを抽出
          suits=pat[c].suits;
          c++;
          c=make3to2(suits,j,c,pat);
        }
        for(i=0;i<4;i++){//3ペアで使用したカードを除外
          own_cards[i][j]=0;
        }
      }
    }

    for(j=1;j<14;j++){
      if(own_cards[0][j]+own_cards[1][j]+own_cards[2][j]+own_cards[3][j]>1){//2ペアを走査
        pat[c].num=j;
        pat[c].suits=0;
        pat[c].count=2;
        pat[c].x=j;

        for(i=0;i<4;i++){
          if(own_cards[i][j]>0){
            pat[c].suits|=conv2to10(i);//論理和で2ペアのスートを抽出
          }
        }
        suits=pat[c].suits;
        c++;
        c=make3to2(suits,j,c,pat);
        for(i=0;i<4;i++){//2ペアで使用したカードを除外
          own_cards[i][j]=0;
        }
      }
    }
  }
  else{//ジョーカー未所持の場合
    for(j=1;j<14;j++){
      if(own_cards[0][j]+own_cards[1][j]+own_cards[2][j]+own_cards[3][j]>3){//革命手を走査
        pat[c].num=j;
        pat[c].suits=15;
        pat[c].count=4;
        pat[c].x=j;
        c++;
        for(i=0;i<4;i++){//革命手を崩してできる3ペアを作成
          pat[c].num=j;
          pat[c].suits=15;
          pat[c].count=3;
          pat[c].x=-1;

          pat[c].suits^=conv2to10(i);//排他的論理和で革命手から作成できる3ペアのスートを抽出
          c++;
        }
        for(i=0;i<3;i++){//革命手を崩してできる2ペアを作成
          for(k=i+1;k<4;k++){
            pat[c].num=j;
            pat[c].suits=15;
            pat[c].count=2;
            pat[c].x=-1;

            pat[c].suits^=(conv2to10(i) | conv2to10(k));//論理和と排他的論理和で革命手から作成できる2ペアのスートを抽出
            c++;
          }
        }
        for(i=0;i<4;i++){//革命手で使用したカードを除外
          own_cards[i][j]=0;
        }
      }
    }
    for(j=1;j<14;j++){
      if(own_cards[0][j]+own_cards[1][j]+own_cards[2][j]+own_cards[3][j]>2){//3aペアを走査
        pat[c].num=j;
        pat[c].suits=15;
        pat[c].count=3;
        pat[c].x=-1;

        for(i=0;i<4;i++){
          if(own_cards[i][j]==0){
            pat[c].suits^=conv2to10(i);//排他的論理和で3ペアのスートを抽出
            c++;
            break;
          }
        }
        for(k=0;k<4;k++){//3ペアを崩してできる2ペアを作成
          if(k==i)continue;
          pat[c].num=j;
          pat[c].suits=15;
          pat[c].count=2;
          pat[c].x=-1;

          pat[c].suits^=(conv2to10(i) | conv2to10(k));//論理和と排他的論理和で3ペアから作成できる2ペアのスートを抽出
          c++;
        }
        for(i=0;i<4;i++){//3ペアで使用したカードを除外
          own_cards[i][j]=0;
        }
      }
    }

    for(j=1;j<14;j++){
      if(own_cards[0][j]+own_cards[1][j]+own_cards[2][j]+own_cards[3][j]>1){//2ペアを走査
        pat[c].num=j;
        pat[c].suits=0;
        pat[c].count=2;
        pat[c].x=j;

        for(i=0;i<4;i++){
          if(own_cards[i][j]>0){
            pat[c].suits|=conv2to10(i);//論理和で2ペアのスートを抽出
          }
        }
        c++;
        for(i=0;i<4;i++){//2ペアで使用したカードを除外
          own_cards[i][j]=0;
        }
      }
    }
  }
  pat[0].num=16;//0番目に所持手役全体のデータを格納
  pat[0].count=c-1;
  for(i=1;i<128;i++){
    if(pat[i].count==0)break;
    pat[0].x+=pat[i].x;
  }
}

int conv2to10(int i){
  switch(i){
    case 0:
      return 1;
    case 1:
      return 2;
    case 2:
      return 4;
    case 3:
      return 8;
    default:
      return -1;
  }
}
int conv10to2(int i){
  switch(i){
    case 1:
      return 0;
    case 2:
      return 1;
    case 4:
      return 2;
    case 8:
      return 3;
    default:
      return -1;
  }
}

void printPattern(pattern *pat){
  int i;
  for(i=0;i<128;i++){
    if(pat[i].count==0)break;
    fprintf(stderr,"----\nnum[%d]\nsuits[%d]\ncount[%d]\nJoker[%d]\nJnum[%d]\nJsuits[%d]\nx[%d]\n",pat[i].num,pat[i].suits,pat[i].count,pat[i].joker,pat[i].jnum,pat[i].jsuits,pat[i].x);
  }
  fprintf(stderr,"-----------------\nCount:%d\n",pat[0].count);
}

int make3to2(int suits, int j, int c, pattern *pat){
  /*
    ジョーカー所持の場合で2ペアから3ペアを出来る限り作成
  */
  if(suits==3){
    pat[c].num=j;
    pat[c].jsuits=4;
    pat[c].count=3;
    pat[c].joker=1;
    pat[c].jnum=j;
    pat[c].suits=7;
    pat[c].x=j;
    c++;
    pat[c].num=j;
    pat[c].jsuits=8;
    pat[c].count=3;
    pat[c].joker=1;
    pat[c].jnum=j;
    pat[c].suits=11;
    pat[c].x=j;
    c++;
  }
  else if(suits==5){
    pat[c].num=j;
    pat[c].jsuits=2;
    pat[c].count=3;
    pat[c].joker=1;
    pat[c].jnum=j;
    pat[c].suits=7;
    pat[c].x=j;
    c++;
    pat[c].num=j;
    pat[c].jsuits=8;
    pat[c].count=3;
    pat[c].joker=1;
    pat[c].jnum=j;
    pat[c].suits=13;
    pat[c].x=j;
    c++;
  }
  else if(suits==9){
    pat[c].num=j;
    pat[c].jsuits=2;
    pat[c].count=3;
    pat[c].joker=1;
    pat[c].jnum=j;
    pat[c].suits=11;
    pat[c].x=j;
    c++;
    pat[c].num=j;
    pat[c].jsuits=4;
    pat[c].count=3;
    pat[c].joker=1;
    pat[c].jnum=j;
    pat[c].suits=13;
    pat[c].x=j;
    c++;
  }
  else if(suits==6){
    pat[c].num=j;
    pat[c].jsuits=1;
    pat[c].count=3;
    pat[c].joker=1;
    pat[c].jnum=j;
    pat[c].suits=7;
    pat[c].x=j;
    c++;
    pat[c].num=j;
    pat[c].jsuits=8;
    pat[c].count=3;
    pat[c].joker=1;
    pat[c].jnum=j;
    pat[c].suits=14;
    pat[c].x=j;
    c++;
  }
  else if(suits==10){
    pat[c].num=j;
    pat[c].jsuits=1;
    pat[c].count=3;
    pat[c].joker=1;
    pat[c].jnum=j;
    pat[c].suits=11;
    pat[c].x=j;
    c++;
    pat[c].num=j;
    pat[c].jsuits=4;
    pat[c].count=3;
    pat[c].joker=1;
    pat[c].jnum=j;
    pat[c].suits=14;
    pat[c].x=j;
    c++;
  }
  else if(suits==12){
    pat[c].num=j;
    pat[c].jsuits=1;
    pat[c].count=3;
    pat[c].joker=1;
    pat[c].jnum=j;
    pat[c].suits=13;
    pat[c].x=j;
    c++;
    pat[c].num=j;
    pat[c].jsuits=2;
    pat[c].count=3;
    pat[c].joker=1;
    pat[c].jnum=j;
    pat[c].suits=14;
    pat[c].x=j;
    c++;
  }
  return c;
}

int make2to1(int suits, int j, int c, pattern *pat){
  /*
    ジョーカー所持の場合で1ペアから2ペアを出来る限り作成
  */
  int i;

  if(suits==1){
    for(i=1;i<16;i*=2){//2進数で表現
      if(suits==i)continue;
      pat[c].num=j;
      pat[c].jsuits=i;
      pat[c].count=2;
      pat[c].joker=1;
      pat[c].jnum=j;
      pat[c].suits=suits|i;
      pat[c].x=j;
      c++;
    }
  }
  else if(suits==2){
    for(i=1;i<16;i*=2){
      if(suits==i)continue;
      pat[c].num=j;
      pat[c].jsuits=i;
      pat[c].count=2;
      pat[c].joker=1;
      pat[c].jnum=j;
      pat[c].suits=suits|i;
      pat[c].x=j;
      c++;
    }
  }
  else if(suits==4){
    for(i=1;i<16;i*=2){
      if(suits==i)continue;
      pat[c].num=j;
      pat[c].jsuits=i;
      pat[c].count=2;
      pat[c].joker=1;
      pat[c].jnum=j;
      pat[c].suits=suits|i;
      pat[c].x=j;
      c++;
    }
  }
  else if(suits==8){
    for(i=1;i<16;i*=2){
      if(suits==i)continue;
      pat[c].num=j;
      pat[c].jsuits=i;
      pat[c].count=2;
      pat[c].joker=1;
      pat[c].jnum=j;
      pat[c].suits=suits|i;
      pat[c].x=j;
      c++;
    }
  }
  return c;
}

void Cselect(pattern *pat, struct state_type state){
  /*
    関数void setで作成した全ての提出手を、場をみて現在提出可能な手になるよう厳選する
  */
  pattern select[128];
  int i;
  int c=1;

  for(i=0;i<128;i++){//初期化
    select[i].num=0;
    select[i].suits=0;
    select[i].count=0;
    select[i].joker=0;
    select[i].jnum=0;
    select[i].jsuits=0;
    select[i].x=0;
  }

  if(pat[0].count==2){
    if(pat[1].num>11||pat[1].num==6)pat[1].x=128;
    if(pat[2].num>11||pat[2].num==6)pat[2].x=128;
  }

  if(state.onset==0){//場にカードがある場合、厳選開始。カードが無い場合そのまま終了
    if(state.rev==0){//--------------革命でないとき-----------------------------
      if(state.qty==1){//------------単体のとき
	for(i=1;pat[i].count>0 && i<128;i++){
	  if(state.lock==1){//縛りのとき
	    if(pat[i].count==1 && conv2tosuits(state.suit)==pat[i].suits && pat[i].num>state.ord){//単体かつスート一致かつ強い
	      select[c].num=pat[i].num;
	      select[c].suits=pat[i].suits;
	      select[c].count=1;
	      select[c].x=pat[i].x;
	      c++;
	    }
	  }
	  else{//縛りでない
	    if(pat[i].count==1 && pat[i].num>state.ord && pat[i].joker==0){//単体かつ強い。ジョーカーは後で追加するので無視
	      select[c].num=pat[i].num;
	      select[c].suits=pat[i].suits;
	      select[c].count=1;
	      select[c].x=pat[i].x;
	      c++;
	    }
	  }
	}
	for(i=1;pat[i].count>0 && i<128;i++){
	  if(pat[i].joker==1 && pat[i].count==1){//ジョーカーを所持していたら追加。ここはループ抜けた後
	    select[c].num=14;
	    select[c].count=1;
	    select[c].joker=1;
	    select[c].jnum=14;
	    select[c].x=14;
	    c++;
	    break;
	  }
	}
      }
      else{//------------------------複数のとき
	if(state.sequence==1){//階段のとき
	  for(i=1;pat[i].count>0 && i<128;i++){
	    if(state.lock==1){//縛りのとき
	      if(pat[i].count>2 && pat[i].count==state.qty && (pat[i].suits==1 || pat[i].suits==2 || pat[i].suits==4 || pat[i].suits==8) && conv2tosuits(state.suit)==pat[i].suits && pat[i].num>state.ord){//階段かつスート一致かつ強い
		select[c].num=pat[i].num;
		select[c].suits=pat[i].suits;
		select[c].count=pat[i].count;
		select[c].joker=pat[i].joker;
		select[c].jnum=pat[i].jnum;
		select[c].jsuits=pat[i].jsuits;
		select[c].x=pat[i].x;
		c++;
	      }
	    }
	    else{//縛りでない
	      if(pat[i].count>2 && pat[i].count==state.qty && (pat[i].suits==1 || pat[i].suits==2 || pat[i].suits==4 || pat[i].suits==8) && pat[i].num>state.ord){//階段かつ強い
		select[c].num=pat[i].num;
		select[c].suits=pat[i].suits;
		select[c].count=pat[i].count;
		select[c].joker=pat[i].joker;
		select[c].jnum=pat[i].jnum;
		select[c].jsuits=pat[i].jsuits;
		select[c].x=pat[i].x;
		c++;
	      }
	    }
	  }
	}
	else{//ペアのとき
	  for(i=1;pat[i].count>0 && i<128;i++){
	    if(state.lock==1){//縛りのとき
	      if(pat[i].count>1 && pat[i].count==state.qty && conv2tosuits(state.suit)==pat[i].suits && pat[i].num>state.ord){//2枚以上のペアかつスート一致かつ強い
		select[c].num=pat[i].num;
		select[c].suits=pat[i].suits;
		select[c].count=pat[i].count;
		select[c].joker=pat[i].joker;
		select[c].jnum=pat[i].jnum;
		select[c].jsuits=pat[i].jsuits;
		select[c].x=pat[i].x;
		c++;
	      }
	    }
	    else{//縛りでない
	      if(pat[i].count>1 && pat[i].count==state.qty && (pat[i].suits!=1 && pat[i].suits!=2 && pat[i].suits!=4 && pat[i].suits!=8) && pat[i].num>state.ord){//2枚以上のペアかつ強い
		select[c].num=pat[i].num;
		select[c].suits=pat[i].suits;
		select[c].count=pat[i].count;
		select[c].joker=pat[i].joker;
		select[c].jnum=pat[i].jnum;
		select[c].jsuits=pat[i].jsuits;
		select[c].x=pat[i].x;
		c++;
	      }
	    }
	  }
	}
      }
    }
    else{//-----------------------------革命の時---------------------------
      if(state.qty==1){//---------------単体のとき
	for(i=1;pat[i].count>0 && i<128;i++){
	  if(state.lock==1){//縛りのとき
	    if(pat[i].count==1 && conv2tosuits(state.suit)==pat[i].suits && pat[i].num<state.ord){//単体かつスート一致かつ弱い
	      select[c].num=pat[i].num;
	      select[c].suits=pat[i].suits;
	      select[c].count=1;
	      select[c].x=pat[i].x;
	      c++;
	    }
	  }
	  else{//縛りでない
	    if(pat[i].count==1 && pat[i].num<state.ord && pat[i].joker==0){//単体かつ弱い。ジョーカーは後で追加するので無視
	      select[c].num=pat[i].num;
	      select[c].suits=pat[i].suits;
	      select[c].count=1;
	      select[c].x=pat[i].x;
	      c++;
	    }
	  }
	}
	for(i=1;pat[i].count>0 && i<128;i++){
	  if(pat[i].joker==1 && pat[i].count==1){//ジョーカーを所持していたら追加
	    select[c].num=14;
	    select[c].count=1;
	    select[c].joker=1;
	    select[c].jnum=14;
	    select[c].x=14;
	    c++;
	    break;
	  }
	}
      }
      else{//---------------------------複数のとき
	if(state.sequence==1){//階段のとき
	  for(i=1;pat[i].count>0 && i<128;i++){
	    if(state.lock==1){//縛りのとき
	      if(pat[i].count>2 && pat[i].count==state.qty && (pat[i].suits==1 || pat[i].suits==2 || pat[i].suits==4 || pat[i].suits==8) && conv2tosuits(state.suit)==pat[i].suits && pat[i].num<state.ord){//階段かつスート一致かつ弱い
		select[c].num=pat[i].num;
		select[c].suits=pat[i].suits;
		select[c].count=pat[i].count;
		select[c].joker=pat[i].joker;
		select[c].jnum=pat[i].jnum;
		select[c].jsuits=pat[i].jsuits;
		select[c].x=pat[i].x;
		c++;
	      }
	    }
	    else{//縛りでない
	      if(pat[i].count>2 && pat[i].count==state.qty && (pat[i].suits==1 || pat[i].suits==2 || pat[i].suits==4 || pat[i].suits==8) && pat[i].num<state.ord){//階段かつ弱い
		select[c].num=pat[i].num;
		select[c].suits=pat[i].suits;
		select[c].count=pat[i].count;
		select[c].joker=pat[i].joker;
		select[c].jnum=pat[i].jnum;
		select[c].jsuits=pat[i].jsuits;
		select[c].x=pat[i].x;
		c++;
	      }
	    }
	  }
	}
	else{//ペアのとき
	  for(i=1;pat[i].count>0 && i<128;i++){
	    if(state.lock==1){//縛りのとき
	      if(pat[i].count>1 && pat[i].count==state.qty && conv2tosuits(state.suit)==pat[i].suits && pat[i].num<state.ord){//2枚以上のペアかつスート一致かつ弱い
		select[c].num=pat[i].num;
		select[c].suits=pat[i].suits;
		select[c].count=pat[i].count;
		select[c].joker=pat[i].joker;
		select[c].jnum=pat[i].jnum;
		select[c].jsuits=pat[i].jsuits;
		select[c].x=pat[i].x;
		c++;
	      }
	    }
	    else{//縛りでない
	      if(pat[i].count>1 && pat[i].count==state.qty && (pat[i].suits!=1 && pat[i].suits!=2 && pat[i].suits!=4 && pat[i].suits!=8) && pat[i].num<state.ord){//2枚以上のペアかつ弱い
		select[c].num=pat[i].num;
		select[c].suits=pat[i].suits;
		select[c].count=pat[i].count;
		select[c].joker=pat[i].joker;
		select[c].jnum=pat[i].jnum;
		select[c].jsuits=pat[i].jsuits;
		select[c].x=pat[i].x;
		c++;
	      }
	    }
	  }
	}
      }
    }
    select[0].num=16;//0番目に所持手役全体のデータを格納
    select[0].count=c-1;
    for(i=1;i<128;i++){
      if(select[i].count==0)break;
      select[0].x+=select[i].x;
    }
    for(i=0;i<128;i++){
      pat[i].num=select[i].num;
      pat[i].suits=select[i].suits;
      pat[i].count=select[i].count;
      pat[i].joker=select[i].joker;
      pat[i].jnum=select[i].jnum;
      pat[i].jsuits=select[i].jsuits;
      pat[i].x=select[i].x;
      if(select[i].count==0)break;
    }
  }
}

int conv2tosuits(int *suits){
  /*
    スートを2進数に変換
   */
  int i;
  int s;
  for(i=0,s=0;i<4;i++){
    if(suits[i]==1)s|=conv2to10(i);
  }
  return s;
}

void convsuitsto2(int suit[5],int x){
  /*
    2進数をスートに変換
   */
  int i;
  for(i=0;i<4;i++){
    if(x%2==1)suit[i]=1;
    else suit[i]=0;
    x>>=1;//右シフト
  }
}

void cardstoPat(int kakumei,int out_cards[5][15],pattern pat){
  /*
    patの手役情報をカードテーブルに格納
  */
  
  int i,j;
  for(i=0;i<5;i++){
    for(j=0;j<15;j++)out_cards[i][j]=0;
  }
  
  if(pat.joker>0){//Joker有り
    out_cards[4][0]=2;
    if(pat.count>1){//複数のとき
      if(pat.suits==1 || pat.suits==2 || pat.suits==4 || pat.suits==8){//階段のとき
	if(kakumei>0){//革命のとき
	  for(i=0;i < pat.count;i++){
	    if(pat.num-i==pat.jnum)out_cards[conv10to2(pat.suits)][pat.jnum]=2;
	    else out_cards[conv10to2(pat.suits)][pat.num-i]=1;
	  }
	}
	else{//革命なし
	  for(i=0;i < pat.count;i++){
	    if(pat.num+i==pat.jnum)out_cards[conv10to2(pat.suits)][pat.jnum]=2;
	    else out_cards[conv10to2(pat.suits)][pat.num+i]=1;
	  }
	}
      }
      else{//ペアのとき
	if(pat.suits==3){
	  out_cards[0][pat.num]=1;
	  out_cards[1][pat.num]=1;
	  out_cards[conv10to2(pat.jsuits)][pat.num]=2;
	}
	else if(pat.suits==5){
	  out_cards[0][pat.num]=1;
	  out_cards[2][pat.num]=1;
	  out_cards[conv10to2(pat.jsuits)][pat.num]=2;
	}
	else if(pat.suits==9){
	  out_cards[0][pat.num]=1;
	  out_cards[3][pat.num]=1;
	  out_cards[conv10to2(pat.jsuits)][pat.num]=2;
	}
	else if(pat.suits==6){
	  out_cards[1][pat.num]=1;
	  out_cards[2][pat.num]=1;
	  out_cards[conv10to2(pat.jsuits)][pat.num]=2;
	}
	else if(pat.suits==10){
	  out_cards[1][pat.num]=1;
	  out_cards[3][pat.num]=1;
	  out_cards[conv10to2(pat.jsuits)][pat.num]=2;
	}
	else if(pat.suits==12){
	  out_cards[2][pat.num]=1;
	  out_cards[3][pat.num]=1;
	  out_cards[conv10to2(pat.jsuits)][pat.num]=2;
	}
	else if(pat.suits==7){
	  out_cards[0][pat.num]=1;
	  out_cards[1][pat.num]=1;
	  out_cards[2][pat.num]=1;
	  out_cards[conv10to2(pat.jsuits)][pat.num]=2;
	}
	else if(pat.suits==11){
	  out_cards[0][pat.num]=1;
	  out_cards[1][pat.num]=1;
	  out_cards[3][pat.num]=1;
	  out_cards[conv10to2(pat.jsuits)][pat.num]=2;
	}
	else if(pat.suits==13){
	  out_cards[0][pat.num]=1;
	  out_cards[2][pat.num]=1;
	  out_cards[3][pat.num]=1;
	  out_cards[conv10to2(pat.jsuits)][pat.num]=2;
	}
	else if(pat.suits==14){
	  out_cards[1][pat.num]=1;
	  out_cards[2][pat.num]=1;
	  out_cards[3][pat.num]=1;
	  out_cards[conv10to2(pat.jsuits)][pat.num]=2;
	}
	else if(pat.suits==15){
	  if(pat.count==4){
	    out_cards[0][pat.num]=1;
	    out_cards[1][pat.num]=1;
	    out_cards[2][pat.num]=1;
	    out_cards[3][pat.num]=1;
	    out_cards[conv10to2(pat.jsuits)][pat.num]=2;
	  }
	  else if(pat.count==5){
	    out_cards[0][pat.num]=1;
	    out_cards[1][pat.num]=1;
	    out_cards[2][pat.num]=1;
	    out_cards[3][pat.num]=1;
	    out_cards[4][pat.num]=2;
	  }
	}
      }
    }
  }
  else{//Joker無し
    if(pat.count==1){//単体のとき
      out_cards[conv10to2(pat.suits)][pat.num]=1;
    }
    else if(pat.count>1){//複数のとき
      if(pat.suits==1 || pat.suits==2 || pat.suits==4 || pat.suits==8){//階段のとき
	if(kakumei>0){//革命のとき
	  for(i=0;i < pat.count;i++) out_cards[conv10to2(pat.suits)][pat.num-i]=1;
	}
	else{//革命なし
	  for(i=0;i < pat.count;i++) out_cards[conv10to2(pat.suits)][pat.num+i]=1;
	}
      }
      else{//ペアのとき
	if(pat.suits==3){
	  out_cards[0][pat.num]=1;
	  out_cards[1][pat.num]=1;
	}
	else if(pat.suits==5){
	  out_cards[0][pat.num]=1;
	  out_cards[2][pat.num]=1;
	}
	else if(pat.suits==9){
	  out_cards[0][pat.num]=1;
	  out_cards[3][pat.num]=1;
	}
	else if(pat.suits==6){
	  out_cards[1][pat.num]=1;
	  out_cards[2][pat.num]=1;
	}
	else if(pat.suits==10){
	  out_cards[1][pat.num]=1;
	  out_cards[3][pat.num]=1;
	}
	else if(pat.suits==12){
	  out_cards[2][pat.num]=1;
	  out_cards[3][pat.num]=1;
	}
	else if(pat.suits==7){
	  out_cards[0][pat.num]=1;
	  out_cards[1][pat.num]=1;
	  out_cards[2][pat.num]=1;
	}
	else if(pat.suits==11){
	  out_cards[0][pat.num]=1;
	  out_cards[1][pat.num]=1;
	  out_cards[3][pat.num]=1;
	}
	else if(pat.suits==13){
	  out_cards[0][pat.num]=1;
	  out_cards[2][pat.num]=1;
	  out_cards[3][pat.num]=1;
	}
	else if(pat.suits==14){
	  out_cards[1][pat.num]=1;
	  out_cards[2][pat.num]=1;
	  out_cards[3][pat.num]=1;
	}
	else if(pat.suits==15){
	  out_cards[0][pat.num]=1;
	  out_cards[1][pat.num]=1;
	  out_cards[2][pat.num]=1;
	  out_cards[3][pat.num]=1;
	}
      }
    }
  }
}

int NNselect(pattern *pat, double OUT[53],int kakumei,int count){
  int cards[5][15]={{0}};
  double out_buf[53]={0};
  double sum;
  double rate[128]={0.0};
  int i,j,k,l;
  double Maxrate=0.0;

  rate[0]=0.006;

  for(i=1;i<pat[0].count+1;i++){
    cardstoPat(kakumei,cards,pat[i]);
    for(j=0,l=0;j<4;j++){
      for(k=1;k<14;k++){
	out_buf[l]=(double)cards[j][k];
	if(out_buf[l]>1)out_buf[l]=1.0;
	l++;
      }
    }
    out_buf[l]=(double)pat[i].joker;
    /*一致率を計算*/


    sum=0.0;

    
    for(j=0,k=0;j<53;j++){
      if(out_buf[j]>0.9){
	sum+=out_buf[j]-OUT[j];
	k++;
      }
    }
    rate[i]=((k-sum)/(double)k)*100;

    if(pat[i].count>1)rate[i]+=(0.0025*pat[i].count);


    if(kakumei==0){
      if(count==0)rate[i]+=(0.006*(14-pat[i].num));
      rate[i]+=(0.002*(14-pat[i].num));
    }
    else{
      if(count==0)rate[i]+=(0.006*pat[i].num);
      rate[i]+=(0.006*pat[i].num);
    }
    if(pat[i].x>100)rate[i]+=0.1;
    if(pat[i].x<0)rate[i]-=0.4;
    }

  /*一致率最大の手役を返す*/
  for(i=0;i<pat[0].count+1;i++){
    if(Maxrate<rate[i]){
      j=i;
      Maxrate=rate[i];
    }
  }
  return j;
}
