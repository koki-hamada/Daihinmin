/*
testes(2017)
動的に評価値を求め，柔軟性に富んだプログラムを目指した．
自身の失敗や成功を記憶し，将来に役立てる．
*/
/*
testestes(2018)
機械学習による提出手の評価を追加した.
現在の環境を入力データとして
提出したいおおよそのカードデータ情報を出力データとした.
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "daihinmin.h"
#include "connection.h"
#include "NN.h"
#include "init_weight.h"

const int g_logging=0;                     //ログ取りをするか否かを判定するための変数

int main(int argc,char* argv[]){
  int my_playernum;              //プレイヤー番号を記憶する
  int whole_gameend_flag=0;	 //全ゲームが終了したか否かを判別する変数
  int one_gameend_flag=0;	 //1ゲームが終わったか否かを判別する変数
  int accept_flag=0;             //提出したカードが受理されたかを判別する変数
  int game_count=0;		 //ゲームの回数を記憶する
  int own_cards_buf[8][15];      //手札のカードテーブルをおさめる変数
  int own_cards[8][15];	         //操作用の手札のテーブル
  int ba_cards_buf[8][15];       //場に出たカードテーブルを納める
  int ba_cards[8][15];           //操作用の場の札のテーブル
  int ba_cards_bef[8][15]={{0}};      //比較用テーブル

  int last_playernum=0;          //最後に提出したプレイヤー
  int used_cards[8][15]={{0}};         //提出後のカード
  int i,j,k;//汎用変数
  int search[8][15]={{0}};//検索用テーブル
  int max=0;//一番強いカードを記録
  int pass_num=0;//自身が提出してからの他プレイヤパス回数
  int t,y;//前中後半，手役を格納
  int f;//汎用フラグ

  int pattern[13][7]={{0}};
  //[0]～[10] 組の情報：[0]強さ[1]枚数[2]スート[3]jokerを使った数字[4]強さ評価値(以降x)[5]革命時の強さ評価値(revx)[6]優先評価値(y)
  //     [11] 手札の情報：[0]組数[1]革命可能性の有無[2]xの最小値(1min)[3]xの2番目に小さい値(2min)[4]全体評価値(z)[5]革命時のz(revz)[6]革命時の2min(rev2m)
  //     [12] 提出後の状況：[0]選択した組の強さ評価値(x')[1]選択した組提出時の全体評価値(z')[2]ジョーカーの使用(0:未所持,1:1枚所持,2:階段の一部(patmake),3:使用)
  //     leadでは[12]には提出する組の情報が入る

  int tes[3][4][3]={{{0}}};//比重調整パラメータ．t=序盤中盤終盤，y=手役(0:Joker,1:単体,2:ペア,3:階段)，[t][y][0]=手役の強さ，[t][y][1]=役提出時の他プレイヤパス数，[t][y][2]=役の評価値
  int tes_buf[3][4][3]={{{0}}};

  int seat_num=0;//ターンプレイヤの席番号
  int score[5]={0};//合計スコア 順番はプレイヤ番号順
  int ranks[5]={0};//1ゲームの各プレイヤの順位(大富豪0...大貧民4,あがれなかった場合4) 順番はプレイヤ番号順
  int jk1card=0;//ジョーカーが1枚で出たとき1
  int win_flag=0;//誰かがあがった直後1
  int ba_cut=0;//場が流れるとき1

  int NNc;
  double IN[INPUTNO]={0};        //NN入力データ
  double OUT[OUTPUTNO]={0};      //NN出力データ
  double wh[HIDDENNO][INPUTNO+1]={{0}};//中間層重み
  double wo[OUTPUTNO][HIDDENNO+1]={{0}};//出力層重み
  
  struct Pattern pat[128];

  /*重みをセット*/
  for(i=0,k=0;i<HIDDENNO;i++){
    for(j=0;j<INPUTNO+1;j++){
      wh[i][j]=w[k];
      k++;
    }
  }
  for(i=0;i<OUTPUTNO;i++){
    for(j=0;j<HIDDENNO+1;j++){
      wo[i][j]=w[k];
      k++;
    }
  }

  //引数のチェック 引数に従ってサーバアドレス、接続ポート、クライアント名を変更
  checkArg(argc,argv);
  //ゲームに参加
  my_playernum=entryToGame();
  while(whole_gameend_flag==0){
    one_gameend_flag=0;                 //1ゲームが終わった事を示すフラグを初期化
    game_count=startGame(own_cards_buf);//ラウンドを始める 最初のカードを受け取る。
    copyTable(own_cards,own_cards_buf); //もらったテーブルを操作するためのテーブルにコピー

    //100ゲーム毎にデータ操作等の処理
    if(game_count%100==1){//階級をリセットするときに，前回の階級を平民にする
      for(i=0;i<5;i++){
        ranks[i]=2;
      }
      if(game_count%500==1){//500ゲーム目で負け越していた場合，動的評価値を一旦リセット
        for(i=0;i<5;i++){
          if(i!=my_playernum){
            if(score[my_playernum]<score[i]){
              for(j=0;j<3;j++){
                for(k=0;k<4;k++){
                  tes[j][k][2]=0;
                }
              }
              break;
            }
          }
        }
      }
      if(game_count!=1){
        if((PRINT_SCORE==1 && my_playernum==0) || PRINT_SCORE==2){
          fprintf( stderr, "%5d game score ",game_count-1);//スコア表示
          for(i=0;i<5;i++){
            fprintf( stderr, "%5d",score[i]);
            if(i==4){
              fprintf( stderr, "\n");
            }
            else{
              fprintf( stderr, " / ");
            }
          }
        }
      }
    }

    //ゲーム開始時にデータ格納と初期化
    for(i=0;i<5;i++){//席順を調べる
      if(own_cards_buf[6][10+i]==my_playernum){
        for(j=0;j<5;j++){
          if((i+j)<5){
            ss.seat[j]=own_cards_buf[6][10+i+j];
          }
          else{
            ss.seat[j]=own_cards_buf[6][5+i+j];
          }
        }
      }
    }

    for(i=0;i<5;i++){
      ss.score[i]=score[ss.seat[i]];//合計スコアセット
      ss.old_rank[i]=ranks[ss.seat[i]];//前ゲームの階級セット
      ss.hand_qty[i]=own_cards_buf[6][ss.seat[i]];//初期の手札枚数セット
      ss.pass[i]=0;//パス/あがり状況リセット
    }
    for(i=0;i<5;i++){
      ranks[i]=4;//順位4にリセット
    }
    clearTable(used_cards);
    clearTable(search);
    jk1card=0;
    win_flag=0;
    ba_cut=1;

    ///カード交換
    if(own_cards[5][0]== 0){ //カード交換時フラグをチェック ==1で正常
      printf("not card-change turn?\n");
      exit (1);
    }
    else{ //テーブルに問題が無ければ実際に交換へ
      if(own_cards[5][1] > 0 && own_cards[5][1]<100){
	int change_qty=own_cards[5][1];          //カードの交換枚数
      	int select_cards[8][15]={{0}};           //選んだカードを格納
	
	//自分が富豪、大富豪であれば不要なカードを選び出す
	/////////////////////////////////////////////////////////////
	//カード交換のアルゴリズムはここに書く
	/////////////////////////////////////////////////////////////
        kou_change(select_cards,own_cards,change_qty,tes);
	/////////////////////////////////////////////////////////////
	//カード交換のアルゴリズム ここまで
	/////////////////////////////////////////////////////////////

	//選んだカードを送信
	sendChangingCards(select_cards);
      }
      else{
	//自分が平民以下なら、何かする必要はない
      }
    } //カード交換ここまで
    while(one_gameend_flag == 0){     //1ゲームが終わるまでの繰り返し

      //場のカード表示
      if((PRINT_STATE==1 && my_playernum==0) || PRINT_STATE==2){
        fprintf( stderr, "bacard:");
        if(ba_cut==1 || state.onset==1){
          fprintf( stderr, " no card\n");
        }
        else{
          fprintf( stderr, "P%d:",last_playernum);
          if(jk1card==1){
            fprintf( stderr, " jk\n");
          }
          else{
            cardprint(ba_cards);
          }
        }
      }
      int select_cards[8][15]={{0}};      //提出用のテーブル
      if(receiveCards(own_cards_buf)== 1){  //カードをown_cards_bufに受け取り
                                            //場を状態の読み出し
	                                    //自分のターンであるかを確認する
	//自分のターンであればこのブロックが実行される。
	clearCards(select_cards);             //選んだカードのクリア
        copyTable(own_cards,own_cards_buf);   //カードテーブルをコピー
	/////////////////////////////////////////////////////////////
	//アルゴリズムここから
	//どのカードを出すかはここにかく
	/////////////////////////////////////////////////////////////


	//場のスートを2進数で格納
	for(i=0;i<5;i++) if(state.suit[i]==1)own_cards[5][8]|=conv2to10(i);

	set(pat, own_cards);
	Cselect(pat, state);

        for(i=0,k=0;i<4;i++){
          for(j=1;j<14;j++){
            IN[k]=(double)own_cards[i][j];
            k++;
          }
        }
        IN[k]=(double)state.joker;k++;
        for(i=0;i<5;i++,k++)IN[k]=((double)state.player_rank[i])/4;
        for(i=0;i<5;i++,k++)IN[k]=((double)state.seat[i])/4;
        for(i=0;i<5;i++,k++)IN[k]=((double)state.player_qty[i])/11;
        for(i=0;i<4;i++,k++){
          if(state.lock==1)IN[k]=((double)state.suit[i]);
	  else IN[k]=0.0;
        }
        IN[k]=(double)state.rev;k++;
	for(i=0;i<6;i++,k++){
	  if(state.qty==i)IN[k]=1.0;
	  else IN[k]=0.0;
	}
        if(state.qty>5)IN[78]=1.0;
        if(state.onset==1)IN[k]=1.0;
	else IN[k]=0.0;
        k++;
        if(state.qty==1)IN[k]=1.0;
	else IN[k]=0.0;
	k++;
	if(state.qty>1&&state.qty==0)IN[k]=1.0;
	else IN[k]=0.0;
	k++;
	if(state.sequence==1)IN[k]=1.0;
	else IN[k]=0.0;
	k++;
        for(i=0;i<13;i++,k++){
	  if(state.ord==i)IN[k]=1.0;
	  else IN[k]=0.0;
	}
	if(state.ord==14)IN[83]=1.0;
	for(i=0;i<4;i++,k++)IN[k]=(double)state.suit[i];

	NN(IN,OUT,wh,wo);


	if(pat[0].count>0){
	  NNc=NNselect(pat,OUT,state.rev,state.qty);
	  //fprintf(stderr,"NNchoice![%d]\n",i);
	}
	/*
	cardstoPat(state.rev,select_cards,pat[i]);
	if(pat[i].joker==1){
	  select_cards[4][0]=0;
	  if(pat[i].count==1)select_cards[0][0]=2;
	  }*/

        max=0;//自分以外のカードの強さの最大
	if(state.rev==0){
          for(i=13;i>0 && max==0;i--){
            if(own_cards[0][i]+own_cards[1][i]+own_cards[2][i]+own_cards[3][i]+used_cards[4][i]!=4){//同じ強さのカードが他の手札にあれば
              max=i;//その強さを記録
            }
          }
        }
	else{
          for(i=1;i<14 && max==0;i++){
            if(own_cards[0][i]+own_cards[1][i]+own_cards[2][i]+own_cards[3][i]+used_cards[4][i]!=4){//同じ強さのカードが他の手札にあれば
              max=i;//その強さを記録
            }
          }
        }
        if(max==0){
          if(state.rev==0){
            max=1;
          }
          else{
            max=13;
          }
        }

        for(i=0;i<13;i++){
          for(j=0;j<6;j++){
            pattern[i][j]=0;
          }
        }
        pat_make(pattern,own_cards,max,used_cards,state.joker,tes);

        //状況表示
        if((PRINT_STATE==1 && my_playernum==0) || PRINT_STATE==2){
          for(i=0;i<5;i++){
            fprintf( stderr, "P%d:score%4d rank%d hand%2d ",ss.seat[i],ss.score[i],ss.old_rank[i],ss.hand_qty[i]);
            if(ss.pass[i]==1){
              fprintf( stderr, "Pass");
            }
            if(ss.pass[i]==3){
              fprintf( stderr, "Win");
            }
            fprintf( stderr, "\n");
          }
          fprintf( stderr, "my_cards:");
          cardprint(own_cards);
          fprintf( stderr, "used_cards_table\n");
          fprintf( stderr, "\n");
          Tableprint(used_cards,0);
        }
        //提出前の組と評価値表示
        if((PRINT_PAT==1 && my_playernum==0) || PRINT_PAT==2){
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

//ここからどのカードを出すか判定-----------------------------------------------------------------------

        if(state.onset==1){ //場にカードが無いとき
          copyTable(own_cards,own_cards_buf);
          kou_lead(select_cards,own_cards,max,used_cards,tes);//新しくカードを出すときの判定
	}
        else{//すでに場にカードがあるとき
          if(jk1card==1 && own_cards[0][1]>0){//スペード3
            k=pattern[11][0];//組数を記録
            own_cards[0][1]=0;
            pat_make(pattern,own_cards,max,used_cards,state.joker,tes);
            if((last_playernum!=own_cards[5][3] && k>=pattern[11][0]) || (last_playernum==own_cards[5][3] && search[4][1]==1)){//組数が増えなければ出す
              select_cards[0][1]=1;
            }
            copyTable(own_cards,own_cards_buf);//判定後own_cardsを元に戻す
          }
          else if(state.qty==own_cards[6][my_playernum])//手札枚数と場の枚数が同じときは、defaultから選択(出せるときは出す)
          {
            if(state.rev==0){
              follow(select_cards,own_cards);    //通常時の提出用 
            }else{
              followRev(select_cards,own_cards); //革命時の提出用
            }
          }
          else{
            i=0;
            if(win_flag==1 && state.sequence==0){//誰かがあがった直後,場の組が場を流せる単体・ペアの場合は提出しない
              i=1;
              if(state.rev==0){
                for(j=state.ord+1;j<=max;j++){
                  if(4-(own_cards[0][j]+own_cards[1][j]+own_cards[2][j]+own_cards[3][j]+used_cards[4][j])>=state.qty){
                    i=0;
                    j=14;
                  }
                }
              }
              else if(state.rev==1){
                for(j=state.ord-1;j>=max;j--){
                  if(4-(own_cards[0][j]+own_cards[1][j]+own_cards[2][j]+own_cards[3][j]+used_cards[4][j])>=state.qty){
                    i=0;
                    j=0;
                  }
                }
              }
            }
            if(state.sequence==1){//階段組の選択
              clearTable(select_cards);
              kou_followsequence(select_cards,own_cards,max,used_cards,tes);
            }
            else if(i==0 && state.qty!=5){//枚数組の選択(5枚の場合は出せないので入らない)
              clearTable(select_cards);
              kou_followgroup(select_cards,own_cards,max,used_cards,tes);//出すカードの判定
            }
          }
          //最後にカードを出したプレイヤーが自分の場合
          if(beEmptyCards(select_cards)==0 && last_playernum==own_cards[5][3]){//自分以外パス
            j=pattern[11][3];//2minを記録
            k=pattern[11][0];//組数を記録
            cardsDiff(own_cards,select_cards);//提出予定の組をown_cardsから抜く
            pat_make(pattern,own_cards,max,used_cards,state.joker,tes);//提出後のパターンを作る
            //ジョーカー判定があいまいの可能性あり
            if(pattern[11][3]<100){//提出後2min<100の場合(2min>=100の場合は提出)
              if(j>70 && j>pattern[11][3]){//提出後<提出前かつ提出前>70
                clearTable(select_cards);
              }
              //ジョーカー単体は出さない
              if(select_cards[0][0]==2)
                select_cards[0][0]=0;
              //強いカードの枚数組は出さない
              if(state.sequence==0 && state.rev==0){
                for(i=max;i<=13;i++){
                  select_cards[0][i]=0;
                  select_cards[1][i]=0;
                  select_cards[2][i]=0;
                  select_cards[3][i]=0;
                }
              }
              if(state.sequence==0 && state.rev==1){
                for(i=max;i>=1;i--){
                  select_cards[0][i]=0;
                  select_cards[1][i]=0;
                  select_cards[2][i]=0;
                  select_cards[3][i]=0;
                }
              }
              //組の数が減らない場合は出さない
              if(k<=pattern[11][0]){
                clearTable(select_cards);
              }
            }
          }
        }//すでに場にカードがあるときの判定ここまで
if(beEmptyCards(select_cards)==0)f=0;
	/////////////////////////////////////////////////////////////
	//アルゴリズムはここまで
	/////////////////////////////////////////////////////////////
	accept_flag=sendCards(select_cards);//cardsを提出
      }
      else{
	//自分のターンではない時
	//必要ならここに処理を記述する
      }
      //そのターンに提出された結果のテーブル受け取り,場に出たカードの情報を解析する
      lookField(ba_cards_buf);
      copyTable(ba_cards,ba_cards_buf);

      ///////////////////////////////////////////////////////////////
      //カードが出されたあと 誰かがカードを出す前の処理はここに書く
      ///////////////////////////////////////////////////////////////
      last_playernum=getLastPlayerNum(ba_cards);

                             /*-----ここから新規追加-----*/
      /*------------------------------------------------------*/
      /*    動的評価付けに必要な情報をtes[3][4][3]に格納していく  　 */
      /*------------------------------------------------------*/
       if(f==0 || f==1){//フラグで管理する
         if(f==0){//自身の手札提出ターン
         //ゲームの前後半を記録
         j=used_cards[5][1];
         if(j>=0 && j<13)t=0;//序盤
         else if(j>=13 && j<32)t=1;//中盤
         else if(j>=32)t=2;//終盤

         copyTable(ba_cards_bef,ba_cards);//一ターン目から差異がでるのはまずいのでここで  
          if(state.rev==0){
            if(state.joker==3 && state.qty==1){//Joker単品の場合
              tes[t][0][0]=14;
              y=0;
              break;
            }
            for(i=13;i>0;i--){
              if(ba_cards[0][i]+ba_cards[1][i]+ba_cards[2][i]+ba_cards[3][i]>0){//強さ記憶
              if(state.qty==1){//単体の場合
                tes[t][1][0]=i;
                y=1;
                break;
              }
              else if(state.qty>1){//複数枚の場合
                if(state.sequence==1){//階段の場合
                  tes[t][3][0]=i;
                  y=3;
                  break;
                }
                else{//ペアの場合
                  tes[t][2][0]=i;
                  y=2;
                  break;
                }
              }
            }
          }
        }
        else if(state.rev==1){//革命時
            for(i=1;i<=13;i++){
            if(state.joker==3 && state.qty==1){//Joker単体の場合
              tes[t][0][0]=14;
              y=0;
              break;
            }
              if(ba_cards[0][i]+ba_cards[1][i]+ba_cards[2][i]+ba_cards[3][i]>0){//強さ記憶
              if(state.qty==1){//単体の場合
                tes[t][1][0]=i;
                y=1;
                break;
              }
              else if(state.qty>1){//複数枚の場合
                if(state.sequence==1){//階段の場合
                  tes[t][3][0]=i;
                  y=3;
                  break;
                }
                else{//ペアの場合
                  tes[t][2][0]=i;
                  y=2;
                  break;
                }
              }
            }
          }
        }
        f=1; //フラグを立ててこのループに入れないようにする
     }//f==0，役提出ターン専用条件分岐終了
      pass_num++;
      f=2;
      for(i=0;i<5;i++){
       if(i == last_playernum)continue;//自分を除く
            if(ss.pass[i]==0)f=1;//他プレイヤがパスをしていない場合，途中で提出役が負けたことがわかる
          }
       if(cmpCards(ba_cards_bef,ba_cards)==1 || f==2){//役が負けたor全員パスで場が流れた
          pass_num-=2;
         //fprintf(stderr,"pass_num=%d\n",pass_num);
          tes[t][y][1]=pass_num;//自身が提出した役で他プレイヤがパスをした回数を格納
      /*------------------------------------------------------*/
      /*                      評価値計算                        */
      /*------------------------------------------------------*/
          if(state.rev==0){
            if(t==0)tes[t][y][2]+=0;//ゲーム進行状況加点
            else if(t==1)tes[t][y][2]-=2;
            else if(t==2)tes[t][y][2]-=2;
            if(tes[t][y][0]==1)tes[t][y][2]-=3;//数値加点
            else if(tes[t][y][0]==2)tes[t][y][2]+=1;
            else if(tes[t][y][0]>=3 && tes[t][y][0]<9)tes[t][y][2]+=8-tes[t][y][0];
            else if(tes[t][y][0]>=9 && tes[t][y][0]<13)tes[t][y][2]+=9-tes[t][y][0];
            else tes[t][y][2]+=10-tes[t][y][0];
            if(tes[t][y][1]==0)tes[t][y][2]-=3;//パス数加点
            else if(tes[t][y][1]==1)tes[t][y][2]+=1;
            else if(tes[t][y][1]==2)tes[t][y][2]+=2;
            else tes[t][y][2]+=3;
            if(y==0)tes[t][y][2]-=0;//役加点
            else if(y==1)tes[t][y][2]-=1;
            else if(y==2)tes[t][y][2]-=2;
            else if(y==3)tes[t][y][2]-=3;
            if(state.lock==1)tes[t][y][2]-= 2;//しばり加点
          }
          else if(state.rev==1){
            if(t==0)tes[t][y][2]+=0;//ゲーム進行状況加点
            else if(t==1)tes[t][y][2]-=2;
            else if(t==2)tes[t][y][2]-=2;
            if(tes[t][y][0]==13)tes[t][y][2]-=3;//数値加点
            if(tes[t][y][0]==12)tes[t][y][2]+=1;
            else if(tes[t][y][0]<=11 && tes[t][y][0]>5)tes[t][y][2]+=tes[t][y][0]-6;
            else if(tes[t][y][0]<=7 && tes[t][y][0]>1)tes[t][y][2]+=tes[t][y][0]-5;
            else tes[t][y][2]+=tes[t][y][0]-4;
            if(tes[t][y][1]==0)tes[t][y][2]-=3;//パス数加点
            else if(tes[t][y][1]==1)tes[t][y][2]+=1;
            else if(tes[t][y][1]==2)tes[t][y][2]+=2;
            else tes[t][y][2]+=3;
            if(y==0)tes[t][y][2]-=0;//役加点
            else if(y==1)tes[t][y][2]-=1;
            else if(y==2)tes[t][y][2]-=2;
            else if(y==3)tes[t][y][2]-=3;
            if(state.lock==1)tes[t][y][2]-=2;//しばり加点
          }

          //パラメータの閾値設定
          for(i=0;i<3;i++){//評価値が±各閾値を超えないようにする
            for(j=0;j<4;j++){
              if(i==0){
                if(tes[i][j][2]>9)tes[i][j][2]=9;
                else if(tes[i][j][2]<-9)tes[i][j][2]=-9;
              }
              else if(i==1){
                if(j==1){
                  if(tes[i][j][2]>45)tes[i][j][2]=45;
                  else if(tes[i][j][2]<-45)tes[i][j][2]=-45;
                }
                else{
                  if(tes[i][j][2]>15)tes[i][j][2]=15;
                  else if(tes[i][j][2]<-15)tes[i][j][2]=-15;
                }
              }
              else{
                if(j==1){
                  if(tes[i][j][2]>65)tes[i][j][2]=65;
                  else if(tes[i][j][2]<-65)tes[i][j][2]=-65;
                }
                else if(j==2){
                  if(tes[i][j][2]>45)tes[i][j][2]=45;
                  else if(tes[i][j][2]<-45)tes[i][j][2]=-45;
                }
                else if(j==3){
                  if(tes[i][j][2]>35)tes[i][j][2]=35;
                  else if(tes[i][j][2]<-35)tes[i][j][2]=-35;
                }
              }
            }
          }
          f=3;
          pass_num=0;
          i=5;
        }
      }
                            /*-----ここまで新規追加-----*/
      //提出後のカードテーブルに場のカードを記録
      ba_cut=0;
      for(i=0;i<=4;i++){
        for(j=0;j<=14;j++){
          if(ba_cards[i][j]==1){
            used_cards[i][j]=1;
            if(j==6){//8切り
              ba_cut=1;
            }
            else if(i==0 && j==1 && jk1card==1){//ジョーカーへのスペード3
              ba_cut=1;
            }
          }
          if(ba_cards[i][j]==2){
            used_cards[4][14]=1;
            if(j==6){//8切り
              ba_cut=1;
            }
          }
        }
      }


      /*プレイヤごとにどのカードを提出したか格納したい場合
      for(i=0;i<4;i++){
        for(j=1;j<=13;j++){
          if(p_used_cards[i][j]==0 && used_cards[i][j]==1){
            p_used_cards[i][j]=ba_cards[5][3]+1;
          }
        }
        if(p_used_cards[4][14]==0 && used_cards[4][14]==1){
          p_used_cards[4][14]=ba_cards[5][3]+1;
        }
      }*/

      used_cards[5][1]=0;//場に出たカードの合計枚数を格納
      for(j=1;j<=13;j++){
        used_cards[4][j]=used_cards[0][j]+used_cards[1][j]+used_cards[2][j]+used_cards[3][j];
        used_cards[5][1]+=used_cards[4][j];
      }
      used_cards[5][1]+=used_cards[4][14];

      //場がジョーカー1枚の場合のみフラグを1に保つ
      if((state.ord==0 || state.ord==14) && state.qty==1){
        jk1card=1;
      }
      for(i=0;i<5;i++){//ターンプレイヤの席番号を格納
        if(ss.seat[i]==own_cards_buf[5][3]){
          seat_num=i;
        }
      }

      //used_cards[5][0]=直前のターンまでに場に出たカードの合計枚数
      if(used_cards[5][0]==used_cards[5][1]){//ターンの前後でカードが出た枚数が同じとき(パス)
        ss.pass[seat_num]=1;
        if((PRINT_STATE==1 && my_playernum==0) || PRINT_STATE==2){
          fprintf( stderr, " turn :P%d:Pass\n",ba_cards[5][3]);
        }
      }
      else{//そうでない場合提出したプレイヤの手札枚数を減らす
        ss.hand_qty[seat_num]-=(used_cards[5][1]-used_cards[5][0]);
        if((PRINT_STATE==1 && my_playernum==0) || PRINT_STATE==2){
          fprintf( stderr, " turn :P%d:use:",ba_cards[5][3]);
          if(jk1card==1){
            fprintf( stderr, " jk\n");
          }
          else{
            cardprint(ba_cards);
          }
        }
      }
      //枚数が0になった場合(あがり)
      if(ss.hand_qty[seat_num]==0 && ss.pass[seat_num]!=3){
        ranks[ss.seat[seat_num]]=0;
        for(i=0;i<5;i++){
          if(ss.pass[i]==3){//あがっている数カウント
            ranks[ss.seat[seat_num]]++;//順位記録
          }
        }
        ss.pass[seat_num]=3;//あがり状態とする
        win_flag=1;
      }
      else{
        win_flag=0;
      }
      used_cards[5][0]=used_cards[5][1];//場に出たカードの合計枚数更新

      for(i=0;i<5;i++){
        if(ss.pass[i]==0){
          break;
        }
      }
      if(i==5){//全員パス
        ba_cut=1;
      }

      //全員パスまたは8切り、ジョーカーへのスペード3の場合パス状態とフラグリセット
      if(ba_cut==1){
        for(i=0;i<5;i++){
          if(ss.pass[i]<=2){
            ss.pass[i]=0;
          }
        }
        jk1card=0;
        win_flag=0;
      }

      //プレイヤの状態表示
      if((PRINT_STATE==1 && my_playernum==0) || PRINT_STATE==2){
        fprintf( stderr, "state :");
        if(ba_cards[5][6]==1){
          fprintf( stderr, "rev ");
        }
        if(ba_cut==1){
          printf("ba_cut");
        }
        else if(ba_cards[5][7]==1){
          fprintf( stderr, "bind ");
        }
        printf("\n\n");

        for(i=0;i<5;i++){
          fprintf( stderr, "P%d:score%4d rank%d hand%2d ",ss.seat[i],ss.score[i],ss.old_rank[i],ss.hand_qty[i]);
          if(ss.pass[i]==1){
            printf("Pass");
          }
          if(ss.pass[i]==3){
            printf("Win");
          }
          printf("\n");
        }
        printf("\n");
      }

      ///////////////////////////////////////////////////////////////
      //ここまで
      ///////////////////////////////////////////////////////////////

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
    for(i=0;i<5;i++){
      score[i]+=(5-ranks[i]);
    }
      /*------------------------------------------------------*/
      /*             ランクによる評価値の条件分岐                  */
      /*------------------------------------------------------*/
    if(ranks[my_playernum]<ss.old_rank[my_playernum]){//前ランクよりも自身のランクが高くなったら評価値の変化を記憶
        for(i=0;i<3;i++){
          for(j=0;j<4;j++){
            for(k=0;k<3;k++){
            tes_buf[i][j][k]=tes[i][j][k];
          }
        }
      }
    }
    else{//ランク維持か以前より低くなったら評価値の変化をbufで上書き
        for(i=0;i<3;i++){
          for(j=0;j<4;j++){
            for(k=0;k<3;k++){
            tes[i][j][k]=tes_buf[i][j][k];
          }
        }
      }
    }

    //場に出たカード等をリセット

    /*各自の手札の記録
    //最後に残ったプレイヤが持っていたカードを記録
    for(k=0;k<5;k++){
      if(ranks[k]==4){
        for(i=0;i<4;i++){
          for(j=1;j<=13;j++){
            if(p_used_cards[i][j]==0){
              p_used_cards[i][j]=k+1;
            }
          }
          if(p_used_cards[4][14]==0){
            p_used_cards[4][14]=k+1;
          }
        }
      }
    }
    state.rev=0;
    for(k=0;k<5;k++){
      clearTable(own_cards);
      state.joker=0;
      for(i=0;i<4;i++){
        for(j=1;j<=13;j++){
          if(p_used_cards[i][j]==k+1){
            own_cards[i][j]=1;
          }
        }
      }
      if(p_used_cards[4][14]==k+1){
        own_cards[4][14]=1;
        state.joker=1;
      }
    }
    //プレイヤごとのカードリセット
    //clearTable(p_used_cards);
    */
  }//全ゲームが終わるまでの繰り返しここまで

  if((PRINT_SCORE==1 && my_playernum==0) || PRINT_SCORE==2){
    fprintf( stderr, "     Final score ");//スコア表示
    for(i=0;i<5;i++){
      fprintf( stderr, "%5d",score[i]);
      if(i==4){
        fprintf( stderr, "\n");
      }
      else{
        fprintf( stderr, " / ");
      }
    }
  }

  //ソケットを閉じて終了
  if(closeSocket()!=0){
    printf("failed to close socket\n");
    exit(1);
  }
  exit(0);
}
