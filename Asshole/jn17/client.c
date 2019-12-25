#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "connection.h"
#include "cardChange.h"
#include "bitCard.h"
#include "cardSelect.h"
#include "mt19937ar.h"
#include "checkInfo.h"
#include "mydef.h"

const int g_logging=0;                     //ログ取りをするか否かを判定するための変数

int main(int argc,char* argv[]){

  int i, j;

  int my_playernum;            //プレイヤー番号を記憶する
  int whole_gameend_flag=0;	 //全ゲームが終了したか否かを判別する変数
  int one_gameend_flag=0;	     //1ゲームが終わったか否かを判別する変数
  int accept_flag=0;           //提出したカードが受理されたかを判別する変数
  int game_count=0;		     //ゲームの回数を記憶する
  int game_start_flag = 0;
  int is_my_turn = 0;
  int change_qty = 0;

  int own_cards[8][15];    //手札のカードテーブルをおさめる変数
  int ba_cards[8][15];     //場に出たカードテーブルを納める

  int64 myCards, oppCards;
  int64 befCards, aftCards; // , getCards;
  changeInfo cinfo;
  fieldInfo  finfo;
  const int64 allCards = (1LL << 53) - 1;

  // 乱数の初期化
  //init_genrand(25837341);
  init_genrand(141592653);
  
  //引数のチェック 引数に従ってサーバアドレス、接続ポート、クライアント名を変更
  checkArg(argc,argv);

  //ゲームに参加
  my_playernum=entryToGame();
  
  while(whole_gameend_flag==0){
    one_gameend_flag=0;                 //1ゲームが終わった事を示すフラグを初期化
    game_start_flag = 1;

    game_count=startGame(own_cards);//ラウンドを始める 最初のカードを受け取る。

    ///カード交換
    if(own_cards[5][0]== 0){ //カード交換時フラグをチェック ==1で正常
      printf("not card-change turn?\n");
      exit (1);
    }
    else{ //テーブルに問題が無ければ実際に交換へ
      // 交換前の手札をビットで保存しておく
      befCards = setBit(own_cards);

      if(own_cards[5][1] > 0 && own_cards[5][1]<100){
	change_qty = own_cards[5][1];          //カードの交換枚数
	int select_cards[8][15] = {{0}};       //選んだカードを格納
	
        if(change_qty == 2){
	  //自分が大富豪であれば不要なカードを選び出す	
	  checkCards(select_cards, own_cards, change_qty);
        }else{
          //自分が富豪のときは最弱を出す
	  checkCards2(select_cards, own_cards, change_qty);
	  //printf("Im fugo\n");
        }
    
	//選んだカードを送信
	sendChangingCards(select_cards);
      }
      else{
	change_qty = 0;
	//自分が平民以下なら、何かする必要はない
      }
    } //カード交換ここまで

    while(one_gameend_flag == 0){     //1ゲームが終わるまでの繰り返し
      int select_cards[8][15]={{0}};      //提出用のテーブル

      is_my_turn = receiveCards(own_cards);

      //getStates();
      checkState(&finfo, own_cards);

      // ゲーム開始時の処理
      if(game_start_flag == 1){
	// 自分の手札
	myCards = setBit(own_cards);
	// 自分以外が持つカード
	oppCards = allCards^myCards;
	aftCards = myCards;
	// 交換に出したカード
	cinfo.chgCards = befCards^(befCards&aftCards);

	cinfo.firstPlayer = -1;

	// あがりフラグ初期化
	finfo.goal = 0;
	// 席順・階級・カードの初期枚数を取得
	for(i=0;i<5;i++) finfo.seat[own_cards[6][i+10]] = i;
	for(i=0;i<5;i++){
	  finfo.lest[finfo.seat[i]] = own_cards[6][i  ];
	  finfo.rank[finfo.seat[i]] = own_cards[6][i+5];
	  if(own_cards[6][i+10] == my_playernum) finfo.mypos = i;
	}


	// 自分が富豪以上の場合，カード交換相手が持ち得ないカードの集合を求める
	cinfo.notCards = 0LL;
	int cnt = 0;
	if(finfo.rank[my_playernum] < 2){
	  int ni=13, nj=0;
	  for(i=13;i>=0;i--){
	    for(j=0;j<4;j++){
	      if(((befCards>>(13*j+i))&1)==1&&cnt<change_qty){
		ni = i, nj = j;
		cnt++;
	      }
	    }
	  }
	  for(i=0;i<13;i++){
	    for(j=3;j>=0;j--){
	      if(ni<i||(ni==i&&nj>j)){
		cinfo.notCards |= (1LL << (13*j+i));
	      }
	    }
	  }
	}

#ifdef USE_ESTIMATE_HAND
	initEnemysProb();
#endif

	game_start_flag = 0;
      }

      // 自分のターンのとき
      if(is_my_turn== 1){  
	// 自分の手札をビットに変換
	myCards = setBit(own_cards);
	// 出すカードを決定する
	monteCarloSearch(select_cards, myCards, oppCards, &cinfo, &finfo);

	// 選んだカードを提出
	accept_flag=sendCards(select_cards);
      }
      else{
	// 自分のターンではない時
	// 意味もなく乱数を進めてみる
	//genrand_int32();
      }

      //そのターンに提出された結果のテーブル受け取り,場に出たカードの情報を解析する
      lookField(ba_cards);

      // 最初に行動したプレイヤを覚えておく
      if(cinfo.firstPlayer == -1){
	cinfo.firstPlayer = finfo.seat[ba_cards[5][3]];
      }

      // ターンの結果から場の情報を更新
      checkField(&finfo, &cinfo, ba_cards, oppCards);

      // 相手が持ちうる手札の情報を更新
      oppCards ^= (oppCards&setBit(ba_cards));
      
      //一回のゲームが終わったか否かの通知をサーバからうける。
      switch (beGameEnd()){
      case 0: //0のときゲームを続ける
	one_gameend_flag=0;
	whole_gameend_flag=0;
	break;
      case 1: //1のとき 1ゲームの終了
	one_gameend_flag=1;
	whole_gameend_flag=0;
	if(g_logging == 1){
	  printf("game #%d was finished.\n",game_count);
	}
	break;
      default: //その他の場合 全ゲームの終了
	one_gameend_flag=1;
	whole_gameend_flag=1;
	if(g_logging == 1){
	  printf("All game was finished(Total %d games.)\n",game_count);
	}
	break;
      }
    }//1ゲームが終わるまでの繰り返しここまで
  }//全ゲームが終わるまでの繰り返しここまで
  //ソケットを閉じて終了
  if(closeSocket()!=0){
    printf("failed to close socket\n");
    exit(1);
  }
  exit(0);
}
