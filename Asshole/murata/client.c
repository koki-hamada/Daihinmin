#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "daihinmin.h"
#include "connection.h"

const int g_logging = 0;                     //ログ取りをするか否かを判定するための変数

int main(int argc, char* argv[]) {
	int my_playernum;              //プレイヤー番号を記憶する
	int whole_gameend_flag = 0;		 //全ゲームが終了したか否かを判別する変数
	int one_gameend_flag = 0;		 //1ゲームが終わったか否かを判別する変数
	int accept_flag = 0;             //提出したカードが受理されたかを判別する変数
	int game_count = 0;				 //ゲームの回数を記憶する
	int own_cards_buf[8][15];      //手札のカードテーブルをおさめる変数
	int own_cards[8][15];	         //操作用の手札のテーブル
	int ba_cards_buf[8][15];       //場に出たカードテーブルを納める
	int ba_cards[8][15];           //操作用の場の札のテーブル

	int last_playernum = 0;          //最後に提出したプレイヤー
	int used_cards[8][15] = { {0} };   //提出後のカード
	int i, j, k;
	int search[8][15] = { {0} };//検索用テーブル
	int max = 0;//一番強いカードを記録

	int pattern[13][7] = { {0} };
	//[0]～[10] 組の情報：[0]強さ[1]枚数[2]スート[3]jokerを使った数字[4]強さ評価値(以降x)[5]革命時の強さ評価値(revx)[6]優先評価値(y)
	//     [11] 手札の情報：[0]組数[1]革命可能性の有無[2]xの最小値(1min)[3]xの2番目に小さい値(2min)[4]全体評価値(z)[5]革命時のz(revz)[6]革命時の2min(rev2m)
	//     [12] 提出後の状況：[0]選択した組の強さ評価値(x')[1]選択した組提出時の全体評価値(z')[2]ジョーカーの使用(0:未所持,1:1枚所持,2:階段の一部(patmake),3:使用)
	//     leadでは[12]には提出する組の情報が入る

	int seat_num = 0;//ターンプレイヤの席番号
	int score[5] = { 0 };//合計スコア 順番はプレイヤ番号順
	int ranks[5] = { 0 };//1ゲームの各プレイヤの順位(大富豪0...大貧民4,あがれなかった場合4) 順番はプレイヤ番号順
	int jk1card = 0;//ジョーカーが1枚で出たとき1
	int win_flag = 0;//誰かがあがった直後1
	int ba_cut = 0;//場が流れるとき1

	int rank_buf = 0;//自分のランクを格納（席順戦略確認用）


	//席順チェックのフラグ-----------------------
	int seat_order_flag = 0;

	//座ってる順番を格納する配列[->自分→〇→〇→〇→〇-]
	int seat_order[5] = { 0 };
	//-------------------------------------------
	//席順戦略でパスした時ちゃんと流れているか
	int pass_check_lag[2][100000] = { 0 };
	//-------------------------------------------
	//場が空の時のクライアントごとの提出履歴
	int onset_flag = 0;
	//-------------------------------------------

	//引数のチェック 引数に従ってサーバアドレス、接続ポート、クライアント名を変更
	checkArg(argc, argv);


	//1000試合などのゲームのループ・ゲームに参加
	my_playernum = entryToGame();
	while (whole_gameend_flag == 0) {
		one_gameend_flag = 0;                 //1ゲームが終わった事を示すフラグを初期化
		game_count = startGame(own_cards_buf);//ラウンドを始める 最初のカードを受け取る。
		copyTable(own_cards, own_cards_buf); //もらったテーブルを操作するためのテーブルにコピー

		//100ゲーム毎にデータ操作等の処理
		if (game_count % 100 == 1) {//階級をリセットするときに，前回の階級を平民にする
			for (i = 0; i < 5; i++) {
				ranks[i] = 2;
			}
			if (game_count != 1) {
				if ((PRINT_SCORE == 1 && my_playernum == 0) || PRINT_SCORE == 2) {
					fprintf(stderr, "%5d game score ", game_count - 1);//スコア表示
					for (i = 0; i < 5; i++) {
						fprintf(stderr, "%5d", score[i]);
						if (i == 4) {
							fprintf(stderr, "\n");
						}
						else {
							fprintf(stderr, " / ");
						}
					}
				}
			}
		}

		//毎試合ごとの席順チェックのフラグの初期化
		seat_order_flag = 0;

		//ゲーム開始時にデータ格納と初期化
		for (i = 0; i < 5; i++) {//席順を調べる
			if (own_cards_buf[6][10 + i] == my_playernum) {
				for (j = 0; j < 5; j++) {
					if ((i + j) < 5) {
						ss.seat[j] = own_cards_buf[6][10 + i + j];
					}
					else {
						ss.seat[j] = own_cards_buf[6][5 + i + j];
					}
				}
			}
		}
		for (i = 0; i < 5; i++) {
			ss.score[i] = score[ss.seat[i]];//合計スコアセット
			ss.old_rank[i] = ranks[ss.seat[i]];//前ゲームの階級セット
			ss.hand_qty[i] = own_cards_buf[6][ss.seat[i]];//初期の手札枚数セット
			ss.pass[i] = 0;//パス/あがり状況リセット
		}
		for (i = 0; i < 5; i++) {
			ranks[i] = 4;//順位4にリセット
		}
		clearTable(used_cards);
		clearTable(search);
		jk1card = 0;
		win_flag = 0;
		ba_cut = 1;

		///カード交換
		if (own_cards[5][0] == 0) { //カード交換時フラグをチェック ==1で正常
			printf("not card-change turn?\n");
			exit(1);
		}
		else { //テーブルに問題が無ければ実際に交換へ
			if (own_cards[5][1] > 0 && own_cards[5][1] < 100) {
				int change_qty = own_cards[5][1];          //カードの交換枚数
				int select_cards[8][15] = { {0} };           //選んだカードを格納

				//自分が富豪、大富豪であれば不要なカードを選び出す
				/////////////////////////////////////////////////////////////
				//カード交換のアルゴリズムはここに書く
				/////////////////////////////////////////////////////////////
				kou_change(select_cards, own_cards, change_qty);
				/////////////////////////////////////////////////////////////
				//カード交換のアルゴリズム ここまで
				/////////////////////////////////////////////////////////////

				//選んだカードを送信
				sendChangingCards(select_cards);
			}
			else {
				//自分が平民以下なら、何かする必要はない
			}
		} //カード交換ここまで


		//席順戦略有効性の確認用---------------------
		if (game_count >= 1) {
			for (i = 0; i < 5; i++)
			{
				if (i == my_playernum) {
					rank_buf = state.player_rank[i];
					break;
				}
			}

			//自分がなった階級を格納
			seat_tactics_check[(game_count - 1)][1] = rank_buf;
		}
		//-------------------------------------------

		//1ゲームでのループ----------------------------------------------------------//
		while (one_gameend_flag == 0) {     //1ゲームが終わるまでの繰り返し

		    //場のカード表示
			if ((PRINT_STATE == 1 && my_playernum == 0) || PRINT_STATE == 2) {
				fprintf(stderr, "bacard:");
				if (ba_cut == 1 || state.onset == 1) {
					fprintf(stderr, " no card\n");
				}
				else {
					fprintf(stderr, "P%d:", last_playernum);
					if (jk1card == 1) {
						fprintf(stderr, " jk\n");
					}
					else {
						cardprint(ba_cards);
					}
				}
			}
			int select_cards[8][15] = { {0} };      //提出用のテーブル
			if (receiveCards(own_cards_buf) == 1) {  //カードをown_cards_bufに受け取り
												  //場を状態の読み出し
											  //自分のターンであるかを確認する
			//自分のターンであればこのブロックが実行される。
				clearCards(select_cards);             //選んだカードのクリア
				copyTable(own_cards, own_cards_buf);   //カードテーブルをコピー
			  /////////////////////////////////////////////////////////////
			  //アルゴリズムここから
			  //どのカードを出すかはここにかく
			  /////////////////////////////////////////////////////////////

				//---------------------------------------------------------------
				//自分の手札がパスをせざるを得ない状況か確認
				/*if (pass_check(own_cards, ba_cards) == 1) {
					printf("\n[must_pass]\n");
				}else if (pass_check(own_cards, ba_cards) == 0) {
					printf("\n[must_not_pass]\n");
				}*/
				//---------------------------------------------------------------

				//場の強さ評価値
				/*
				if (kou_e_value(own_cards, seat_order, used_cards) >= 86) {
					printf("E value=%d\n", kou_e_value(own_cards, seat_order, used_cards));
				}
				*/



				//自分と相手の席の関係を格納する--------------------
				if (seat_order_flag == 0) {
					for (i = 0; i < 5; i++) {
						if (own_cards[6][10 + i] == my_playernum) {
							break;
						}
					}

					for (j = 0; j < 5; j++) {
						seat_order[j] = own_cards[6][10 + (i + j) % 5];
					}

					seat_order_flag = 1;

					//前席クライアントの階級格納
					if (game_count >= 1) {
						seat_tactics_check[game_count][2] = state.player_rank[seat_order[4]];
					}
				}
				//--------------------------------------------------

				max = 0;//自分以外のカードの強さの最大
				if (state.rev == 0) {
					for (i = 13; i > 0 && max == 0; i--) {
						if (own_cards[0][i] + own_cards[1][i] + own_cards[2][i] + own_cards[3][i] + used_cards[4][i] != 4) {//同じ強さのカードが他の手札にあれば
							max = i;//その強さを記録
						}
					}
				}
				else {
					for (i = 1; i < 14 && max == 0; i++) {
						if (own_cards[0][i] + own_cards[1][i] + own_cards[2][i] + own_cards[3][i] + used_cards[4][i] != 4) {//同じ強さのカードが他の手札にあれば
							max = i;//その強さを記録
						}
					}
				}
				if (max == 0) {
					if (state.rev == 0) {
						max = 1;
					}
					else {
						max = 13;
					}
				}

				for (i = 0; i < 13; i++) {
					for (j = 0; j < 6; j++) {
						pattern[i][j] = 0;
					}
				}

				//手札の組作りと評価値の決定
				pat_make(pattern, own_cards, max, used_cards, state.joker);

				//状況表示
				if ((PRINT_STATE == 1 && my_playernum == 0) || PRINT_STATE == 2) {
					for (i = 0; i < 5; i++) {
						fprintf(stderr, "P%d:score%4d rank%d hand%2d ", ss.seat[i], ss.score[i], ss.old_rank[i], ss.hand_qty[i]);
						if (ss.pass[i] == 1) {
							fprintf(stderr, "Pass");
						}
						if (ss.pass[i] == 3) {
							fprintf(stderr, "Win");
						}
						fprintf(stderr, "\n");
					}
					fprintf(stderr, "my_cards:");
					cardprint(own_cards);
					fprintf(stderr, "used_cards_table\n");
					fprintf(stderr, "\n");
					Tableprint(used_cards, 0);
				}
				//提出前の組と評価値表示
				if ((PRINT_PAT == 1 && my_playernum == 0) || PRINT_PAT == 2) {
					for (i = 0; i < 11; i++) {
						if (pattern[i][1] == 0) {
							i = 11;
						}
						else {
							fprintf(stderr, "[ord%2d][qty%d][sui%2d][Jok%2d][x%3d][rx%3d][y%2d]\n", pattern[i][0], pattern[i][1], pattern[i][2], pattern[i][3], pattern[i][4], pattern[i][5], pattern[i][6]);
						}
					}
					fprintf(stderr, "[set%2d][rev%d][1min%3d][2min%3d][z%3d][rz%3d][r2m%3d]\n", pattern[11][0], pattern[11][1], pattern[11][2], pattern[11][3], pattern[11][4], pattern[11][5], pattern[11][6]);
					fprintf(stderr, "\n");
				}

				//ここからどのカードを出すか判定-----------------------------------------------------------------------

				if (state.onset == 1) { //場にカードが無いとき
					copyTable(own_cards, own_cards_buf);
					kou_lead(select_cards, own_cards, max, used_cards);//新しくカードを出すときの判定
				}
				else {//すでに場にカードがあるとき
				  //  jkcard:ジョーカーが1枚出たとき１  own_cards:操作用テーブル、[0][1]はスぺ3
					if (jk1card == 1 && own_cards[0][1] > 0) {//ジョーカーがでた　かつ　スペード3持ってる//////////////////////////////
						k = pattern[11][0];//pattern[11][0]:手札の組数を記録
						own_cards[0][1] = 0;//[0][1]=0:スぺ3持ってない
						pat_make(pattern, own_cards, max, used_cards, state.joker);//pat_make手札の組作りと評価値の決定

						//組数が増えなければ出す,serch[][]:検索用テーブル
						if ((last_playernum != own_cards[5][3] && k >= pattern[11][0]) || (last_playernum == own_cards[5][3] && search[4][1] == 1)) {
							select_cards[0][1] = 1;//select_cards:提出用のテーブル
						}
						copyTable(own_cards, own_cards_buf);//判定後own_cardsを元に戻す
					}
					else if (state.qty == own_cards[6][my_playernum])//手札枚数と場の枚数が同じときは、defaultから選択(出せるときは出す)
					{
						if (state.rev == 0) {
							follow(select_cards, own_cards);    //通常時の提出用 
						}
						else {
							followRev(select_cards, own_cards); //革命時の提出用
						}
					}
					else {////////////////////////////////////////////////////////////////////////////////////////////////////////
						i = 0;
						//誰かがあがった直後　かつ　階段じゃない時,場の組が場を流せる単体・ペアの場合は提出しない
						if (win_flag == 1 && state.sequence == 0) {
							i = 1;
							if (state.rev == 0) {
								for (j = state.ord + 1; j <= max; j++) {
									if (4 - (own_cards[0][j] + own_cards[1][j] + own_cards[2][j] + own_cards[3][j] + used_cards[4][j]) >= state.qty) {
										//i=0で現在の単品or複数枚の場のカードより強いの持ってる
										i = 0;
										j = 14;
									}
								}
							}
							else if (state.rev == 1) {
								for (j = state.ord - 1; j >= max; j--) {
									if (4 - (own_cards[0][j] + own_cards[1][j] + own_cards[2][j] + own_cards[3][j] + used_cards[4][j]) >= state.qty) {
										i = 0;
										j = 0;
									}
								}
							}
						}
						if (state.sequence == 1) {//階段組の選択
							clearTable(select_cards);
							kou_followsequence(select_cards, own_cards, max, used_cards);//階段で出すときの思考
						}
						else if (i == 0 && state.qty != 5) {//枚数組の選択(5枚の場合は出せないので入らない)
							clearTable(select_cards);
							kou_followgroup(select_cards, own_cards, max, used_cards, my_playernum, seat_order, last_playernum, ba_cards, game_count);//出すカードの判定
						}
					}
					//最後にカードを出したプレイヤーが自分の場合/////////////////////////////////////////////////////////////////
					if (beEmptyCards(select_cards) == 0 && last_playernum == own_cards[5][3]) {//自分以外パス
						j = pattern[11][3];//2minを記録
						k = pattern[11][0];//組数を記録
						cardsDiff(own_cards, select_cards);//提出予定の組をown_cardsから抜く
						pat_make(pattern, own_cards, max, used_cards, state.joker);//提出後のパターンを作る
						//ジョーカー判定があいまいの可能性あり
						if (pattern[11][3] < 100) {//提出後2min<100の場合(2min>=100の場合は提出)
							if (j > 70 && j > pattern[11][3]) {//提出後<提出前かつ提出前>70
								clearTable(select_cards);
							}
							//ジョーカー単体は出さない
							if (select_cards[0][0] == 2)
								select_cards[0][0] = 0;
							//強いカードの枚数組は出さない
							if (state.sequence == 0 && state.rev == 0) {
								for (i = max; i <= 13; i++) {
									select_cards[0][i] = 0;
									select_cards[1][i] = 0;
									select_cards[2][i] = 0;
									select_cards[3][i] = 0;
								}
							}
							if (state.sequence == 0 && state.rev == 1) {
								for (i = max; i >= 1; i--) {
									select_cards[0][i] = 0;
									select_cards[1][i] = 0;
									select_cards[2][i] = 0;
									select_cards[3][i] = 0;
								}
							}
							//組の数が減らない場合は出さない
							if (k <= pattern[11][0]) {
								clearTable(select_cards);
							}
						}
					}
				}//すでに場にカードがあるときの判定ここまで

			/////////////////////////////////////////////////////////////
			//アルゴリズムはここまで
			/////////////////////////////////////////////////////////////
				accept_flag = sendCards(select_cards);//cardsを提出
			}
			else {
				//自分のターンではない時
				//必要ならここに処理を記述する
				if (state.onset == 1) {
					onset_flag = 1;
				}
			}
			//そのターンに提出された結果のテーブル受け取り,場に出たカードの情報を解析する
			lookField(ba_cards_buf);
			copyTable(ba_cards, ba_cards_buf);

			///////////////////////////////////////////////////////////////
			//カードが出されたあと 誰かがカードを出す前の処理はここに書く
			///////////////////////////////////////////////////////////////
			last_playernum = getLastPlayerNum(ba_cards);

			//-------------------------------------------------------------
			//client_log[2][4][14][4]に提出カード格納
			if (onset_flag == 1) {
				if (state.qty == 1) {
					for (i = 0; i <= 4; i++) {
						for (j = 0; j <= 14; j++) {
							if (state.ord==j && state.suit[i]==1) {
								if (state.rev == 0) {
									client_log[0][i][j][last_playernum] = 1;
								}else if (state.rev == 1) {
									client_log[0][i][j][last_playernum] = 2;
								}
							}
						}
					}
				}
				else if (state.qty == 2) {
					for (i = 0; i <= 4; i++) {
						for (j = 0; j <= 14; j++) {
							if (state.ord == j && state.suit[i] == 1) {
								if (state.rev == 0) {
									client_log[1][i][j][last_playernum] = 1;
								}
								else if (state.rev == 1) {
									client_log[1][i][j][last_playernum] = 2;
								}
							}
						}
					}
				}

				onset_flag = 0;
			}
			//-------------------------------------------------------------

			//提出後のカードテーブルに場のカードを記録
			ba_cut = 0;
			for (i = 0; i <= 4; i++) {
				for (j = 0; j <= 14; j++) {
					if (ba_cards[i][j] == 1) {
						used_cards[i][j] = 1;
						if (j == 6) {//8切り
							ba_cut = 1;
						}
						else if (i == 0 && j == 1 && jk1card == 1) {//ジョーカーへのスペード3
							ba_cut = 1;
						}
					}
					if (ba_cards[i][j] == 2) {
						used_cards[4][14] = 1;
						if (j == 6) {//8切り
							ba_cut = 1;
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

			used_cards[5][1] = 0;//場に出たカードの合計枚数を格納
			for (j = 1; j <= 13; j++) {
				used_cards[4][j] = used_cards[0][j] + used_cards[1][j] + used_cards[2][j] + used_cards[3][j];
				used_cards[5][1] += used_cards[4][j];
			}
			used_cards[5][1] += used_cards[4][14];

			//場がジョーカー1枚の場合のみフラグを1に保つ
			if ((state.ord == 0 || state.ord == 14) && state.qty == 1) {
				jk1card = 1;
			}
			for (i = 0; i < 5; i++) {//ターンプレイヤの席番号を格納
				if (ss.seat[i] == own_cards_buf[5][3]) {
					seat_num = i;
				}
			}

			//used_cards[5][0]=直前のターンまでに場に出たカードの合計枚数
			if (used_cards[5][0] == used_cards[5][1]) {//ターンの前後でカードが出た枚数が同じとき(パス)
				ss.pass[seat_num] = 1;
				if ((PRINT_STATE == 1 && my_playernum == 0) || PRINT_STATE == 2) {
					fprintf(stderr, " turn :P%d:Pass\n", ba_cards[5][3]);
				}
			}
			else {//そうでない場合提出したプレイヤの手札枚数を減らす
				ss.hand_qty[seat_num] -= (used_cards[5][1] - used_cards[5][0]);
				if ((PRINT_STATE == 1 && my_playernum == 0) || PRINT_STATE == 2) {
					fprintf(stderr, " turn :P%d:use:", ba_cards[5][3]);
					if (jk1card == 1) {
						fprintf(stderr, " jk\n");
					}
					else {
						cardprint(ba_cards);
					}
				}
			}
			//枚数が0になった場合(あがり)
			if (ss.hand_qty[seat_num] == 0 && ss.pass[seat_num] != 3) {
				ranks[ss.seat[seat_num]] = 0;
				for (i = 0; i < 5; i++) {
					if (ss.pass[i] == 3) {//あがっている数カウント
						ranks[ss.seat[seat_num]]++;//順位記録
					}
				}
				ss.pass[seat_num] = 3;//あがり状態とする
				win_flag = 1;
			}
			else {
				win_flag = 0;
			}
			used_cards[5][0] = used_cards[5][1];//場に出たカードの合計枚数更新

			for (i = 0; i < 5; i++) {
				if (ss.pass[i] == 0) {
					break;
				}
			}
			if (i == 5) {//全員パス
				ba_cut = 1;

				//----------------------------------------------------------------
				if (ba_nagare_last_playernum != 10) {
					//席順戦略でパスした時の最後に出したクライアント（前のクライアント）が最終的に場を流しているか
					if (last_playernum == ba_nagare_last_playernum) {
						//予定通り流れた
						pass_check_lag[0][game_count]++;
						ba_nagare_last_playernum = 10;
					}
					else {
						//予定通り流れなかった
						pass_check_lag[1][game_count]++;
						ba_nagare_last_playernum = 10;
					}
				}
				//----------------------------------------------------------------
			}

			//全員パスまたは8切り、ジョーカーへのスペード3の場合パス状態とフラグリセット
			if (ba_cut == 1) {
				for (i = 0; i < 5; i++) {
					if (ss.pass[i] <= 2) {
						ss.pass[i] = 0;
					}
				}
				jk1card = 0;
				win_flag = 0;
			}

			//プレイヤの状態表示
			if ((PRINT_STATE == 1 && my_playernum == 0) || PRINT_STATE == 2) {
				fprintf(stderr, "state :");
				if (ba_cards[5][6] == 1) {
					fprintf(stderr, "rev ");
				}
				if (ba_cut == 1) {
					printf("ba_cut");
				}
				else if (ba_cards[5][7] == 1) {
					fprintf(stderr, "bind ");
				}
				printf("\n\n");

				for (i = 0; i < 5; i++) {
					fprintf(stderr, "P%d:score%4d rank%d hand%2d ", ss.seat[i], ss.score[i], ss.old_rank[i], ss.hand_qty[i]);
					if (ss.pass[i] == 1) {
						printf("Pass");
					}
					if (ss.pass[i] == 3) {
						printf("Win");
					}
					printf("\n");
				}
				printf("\n");
			}

			///////////////////////////////////////////////////////////////
			//カードが出されたあと 誰かがカードを出す前の処理ここまで
			///////////////////////////////////////////////////////////////

			//一回のゲームが終わったか否かの通知をサーバからうける。
			switch (beGameEnd()) {
			case 0: //0のときゲームを続ける
				one_gameend_flag = 0;
				whole_gameend_flag = 0;
				break;
			case 1: //1のとき 1ゲームの終了
				one_gameend_flag = 1;
				whole_gameend_flag = 0;
				if (g_logging == 1) {
					printf("game #%d was finished.\n", game_count);
				}
				break;
			default: //その他の場合 全ゲームの終了
				one_gameend_flag = 1;
				whole_gameend_flag = 1;
				if (g_logging == 1) {
					printf("All game was finished(Total %d games.)\n", game_count);
				}
				break;
			}
		}
		//1ゲームが終わるまでの繰り返しここまで---------------------------------------------------------//

		//1ゲームごとにリセット
		for (i = 0; i <= 4; i++) {
			for (j = 0; j <= 14; j++) {
				for (k = 0; k <= 4; k++) {
					client_log[0][i][j][k] = 0;
					client_log[1][i][j][k] = 0;
				}
			}
		}

		for (i = 0; i < 5; i++) {
			score[i] += (5 - ranks[i]);
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

	}//全ゲームが終わるまでの繰り返しここまで-----------------------------------------------------


	//自分用ゲーム終わりの状態確認---------------------------

	//printf("\nseat_tactics_on:%d\n", seat_tactics_check_on);
	//printf("seat_tactics_off:%d\n", seat_tactics_check_off);

	//printf("\nba_nagare_single:%d\n", ba_nagare_single);
	//printf("ba_nagare_double:%d\n", ba_nagare_double);
	//printf("ba_nagare_three_cards:%d\n", ba_nagare_three_cards);
	//printf("ba_nagare_max:%d\n", ba_nagare_max_count);
	//printf("	ba_nagare_max_single:%d\n", ba_nagare_max_count_single);
	//printf("		ba_nagare_max_single_nomal:%d\n", ba_nagare_max_count_single_nomal);
	//printf("		ba_nagare_max_single_nomal_rev:%d\n", ba_nagare_max_count_single_nomal_rev);
	//printf("		ba_nagare_max_single_sibari:%d\n", ba_nagare_max_count_single_sibari);
	//printf("		ba_nagare_max_single_sibari_rev:%d\n", ba_nagare_max_count_single_sibari_rev);
	//printf("	ba_nagare_max_double:%d\n", ba_nagare_max_count_double);
	//printf("		ba_nagare_max_double_nomal:%d\n", ba_nagare_max_count_double_nomal);
	//printf("		ba_nagare_max_double_nomal_rev:%d\n", ba_nagare_max_count_double_nomal_rev);
	//printf("		ba_nagare_max_double_sibari:%d\n", ba_nagare_max_count_double_sibari);
	//printf("		ba_nagare_max_double_sibari_rev:%d\n", ba_nagare_max_count_double_sibari_rev);
	//printf("\nflag_count_normal:%d\n", flag_count_normal);
	//printf("pass_check_count:%d\n", pass_check_count);
	//printf("client_log_pass_off_count:%d\n", client_log_st_pass_off_count);

	//printf("\n\n");
	////---------------------------
	//int rank_total[5] = { 0 };//席順戦略が実行された際の階級を格納、0:大富豪～4:大貧民
	//float rank_sum = 0, zero_sum = 0, one_sum = 0, two_sum = 0, three_sum = 0, four_sum = 0;
	//float zero_rate, one_rate, two_rate, three_rate, four_rate;

	//for (i = 0; i < 1000; i++)
	//{
	//	if (seat_tactics_check[i][0] >= 1) {
	//		//階級を格納
	//		if (seat_tactics_check[i][1] == 0) {
	//			rank_total[0]++;
	//		}
	//		else if (seat_tactics_check[i][1] == 1) {
	//			rank_total[1]++;
	//		}
	//		else if (seat_tactics_check[i][1] == 2) {
	//			rank_total[2]++;
	//		}
	//		else if (seat_tactics_check[i][1] == 3) {
	//			rank_total[3]++;
	//		}
	//		else if (seat_tactics_check[i][1] == 4) {
	//			rank_total[4]++;
	//		}
	//	}
	//}

	//printf("daihugou:%d\n", rank_total[0]);
	//printf("hugou:%d\n", rank_total[1]);
	//printf("heimin:%d\n", rank_total[2]);
	//printf("hinmin:%d\n", rank_total[3]);
	//printf("daihinmin:%d\n", rank_total[4]);


	////--------------------------------------------------------------------------
	//printf("\n\n<seat_tactics>\n");
	////各試合の状況
	//for (i = 0; i < 1000; i++)
	//{
	//	if (seat_tactics_check[i][0] >= 1) {
	//		printf("%3d:%d , rank%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1]);
	//		rank_sum++;
	//		if (seat_tactics_check[i][1] == 0) {
	//			zero_sum++;
	//		}
	//		else if (seat_tactics_check[i][1] == 1) {
	//			one_sum++;
	//		}
	//		else if (seat_tactics_check[i][1] == 2) {
	//			two_sum++;
	//		}
	//		else if (seat_tactics_check[i][1] == 3) {
	//			three_sum++;
	//		}
	//		else if (seat_tactics_check[i][1] == 4) {
	//			four_sum++;
	//		}
	//	}
	//}
	//zero_rate = zero_sum / rank_sum;
	//one_rate = one_sum / rank_sum;
	//two_rate = two_sum / rank_sum;
	//three_rate = three_sum / rank_sum;
	//four_rate = four_sum / rank_sum;

	//printf("\ndaihugou_rate :%.1f\n", (zero_sum / rank_sum) * 100);
	//printf("hugou_rate    :%.1f\n", (one_sum / rank_sum) * 100);
	//printf("heimin_rate   :%.1f\n", (two_sum / rank_sum) * 100);
	//printf("hinmin_rate   :%.1f\n", (three_sum / rank_sum) * 100);
	//printf("daihinmin_rate:%.1f\n", (four_sum / rank_sum) * 100);

	//rank_sum = 0;
	//zero_sum = 0;
	//one_sum = 0;
	//two_sum = 0;
	//three_sum = 0;
	//four_sum = 0;
	//four_sum = 0;

	////--------------------------------------------------------------------------
	//printf("\n\n<seat_tactics_lot>\n");
	////各試合の状況（席順戦略が多い時）
	//for (i = 0; i < 1000; i++)
	//{
	//	if (seat_tactics_check[i][0] >= 2) {
	//		printf("%3d:%d , rank%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1]);
	//		rank_sum++;
	//		if (seat_tactics_check[i][1] == 0) {
	//			zero_sum++;
	//		}
	//		else if (seat_tactics_check[i][1] == 1) {
	//			one_sum++;
	//		}
	//		else if (seat_tactics_check[i][1] == 2) {
	//			two_sum++;
	//		}
	//		else if (seat_tactics_check[i][1] == 3) {
	//			three_sum++;
	//		}
	//		else if (seat_tactics_check[i][1] == 4) {
	//			four_sum++;
	//		}
	//	}
	//}

	//zero_rate = zero_sum / rank_sum;
	//one_rate = one_sum / rank_sum;
	//two_rate = two_sum / rank_sum;
	//three_rate = three_sum / rank_sum;
	//four_rate = four_sum / rank_sum;

	//printf("\ndaihugou_rate :%.1f\n", (zero_sum / rank_sum) * 100);
	//printf("hugou_rate    :%.1f\n", (one_sum / rank_sum) * 100);
	//printf("heimin_rate   :%.1f\n", (two_sum / rank_sum) * 100);
	//printf("hinmin_rate   :%.1f\n", (three_sum / rank_sum) * 100);
	//printf("daihinmin_rate:%.1f\n", (four_sum / rank_sum) * 100);

	//zero_sum = 0;
	//one_sum = 0;
	//two_sum = 0;
	//three_sum = 0;
	//four_sum = 0;

	////--------------------------------------------------------------------------
	//printf("\n\n<seat_tactics_few>\n");
	////各試合の状況（席順戦略が少ない時）
	//for (i = 0; i < 1000; i++)
	//{
	//	if (seat_tactics_check[i][0] == 1) {
	//		printf("%3d:%d , rank%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1]);
	//	}
	//	rank_sum++;
	//	if (seat_tactics_check[i][1] == 0) {
	//		zero_sum++;
	//	}
	//	else if (seat_tactics_check[i][1] == 1) {
	//		one_sum++;
	//	}
	//	else if (seat_tactics_check[i][1] == 2) {
	//		two_sum++;
	//	}
	//	else if (seat_tactics_check[i][1] == 3) {
	//		three_sum++;
	//	}
	//	else if (seat_tactics_check[i][1] == 4) {
	//		four_sum++;
	//	}
	//}

	//zero_rate = zero_sum / rank_sum;
	//one_rate = one_sum / rank_sum;
	//two_rate = two_sum / rank_sum;
	//three_rate = three_sum / rank_sum;
	//four_rate = four_sum / rank_sum;

	//printf("\ndaihugou_rate :%.1f\n", (zero_sum / rank_sum) * 100);
	//printf("hugou_rate    :%.1f\n", (one_sum / rank_sum) * 100);
	//printf("heimin_rate   :%.1f\n", (two_sum / rank_sum) * 100);
	//printf("hinmin_rate   :%.1f\n", (three_sum / rank_sum) * 100);
	//printf("daihinmin_rate:%.1f\n", (four_sum / rank_sum) * 100);

	////-------------------------------------------------------
	////1000試合での発動回数と順位の関係を出力する
	//int my_score;
	//int end_rank;	// 5は最終スコア１位～1は最終スコア5位

	//my_score = score[my_playernum];
	//end_rank = 5;	//1位と仮定

	//for (i = 0; i < 5; i++) {
	//	if (i == my_playernum) {
	//		continue;
	//	}

	//	if (my_score < score[i]) {
	//		end_rank--;
	//	}
	//}

	//FILE *fp4;

	//fp4 = fopen("st_all_log.csv", "a+");

	//if (fp4 == NULL) {
	//	printf("[st_all_log.csv]file not open\n");
	//}
	//else {
	//	fprintf(fp4, "%d,%d\n", seat_tactics_check_on, end_rank);

	//	printf("[st_all_log.csv]file write OK\n");
	//}

	//fclose(fp4);
	////-------------------------------------------------------
	////ファイルにログ書き出し
	////通常書き込みモード"w"、追加書き込みモード"a+"（追加のみ）
	//FILE *fp;

	//fp = fopen("st_log.csv", "a+");

	//if (fp == NULL) {
	//	printf("[st_log.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++)
	//	{
	//		fprintf(fp, "%d,%d,%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1]);
	//	}

	//	printf("[st_log.csv]file write OK\n");
	//}

	//fclose(fp);
	////----------
	//FILE *fp1;

	//fp1 = fopen("st_log_1.csv", "a+");

	//if (fp1 == NULL) {
	//	printf("[st_log_1.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++)
	//	{
	//		if (seat_tactics_check[i][0] == 1) {
	//			fprintf(fp1, "%d,%d,%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1]);
	//		}

	//	}

	//	printf("[st_log_1.csv]file write OK\n");
	//}

	//fclose(fp1);
	////----------
	//FILE *fp2;

	//fp2 = fopen("st_log_2.csv", "a+");

	//if (fp2 == NULL) {
	//	printf("[st_log_2.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++)
	//	{
	//		if (seat_tactics_check[i][0] == 2) {
	//			fprintf(fp2, "%d,%d,%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1]);
	//		}

	//	}

	//	printf("[st_log_2.csv]file write OK\n");
	//}

	//fclose(fp2);
	////----------
	//FILE *fp3;

	//fp3 = fopen("st_log_3_over.csv", "a+");

	//if (fp3 == NULL) {
	//	printf("[st_log_3.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++)
	//	{
	//		if (seat_tactics_check[i][0] >= 3) {
	//			fprintf(fp3, "%d,%d,%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1]);
	//		}

	//	}

	//	printf("[st_log_3.csv]file write OK\n");
	//}

	//fclose(fp3);

	////-------------------------------------------------------
	////発動回数と自分の階級、前のクライアントの階級の関係を出力(rank=0、大富豪)
	//FILE *fp5;

	//fp5 = fopen("st_log_rank0.csv", "a+");

	//if (fp5 == NULL) {
	//	printf("[st_log_rank0.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++)
	//	{
	//		if (seat_tactics_check[i][0] >= 1 && seat_tactics_check[i][1] == 0) {
	//			fprintf(fp5, "%d,%d,%d,%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1], seat_tactics_check[i][2]);
	//		}

	//	}

	//	printf("[st_log_ramk0.csv]file write OK\n");
	//}

	//fclose(fp5);

	////-------------------------------------------------------
	////-------------------------------------------------------
	////発動回数と自分の階級、前のクライアントの階級の関係を出力(rank=1、富豪)
	//FILE *fp6;

	//fp6 = fopen("st_log_rank1.csv", "a+");

	//if (fp6 == NULL) {
	//	printf("[st_log_rank1.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++)
	//	{
	//		if (seat_tactics_check[i][0] >= 1 && seat_tactics_check[i][1] == 1) {
	//			fprintf(fp6, "%d,%d,%d,%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1], seat_tactics_check[i][2]);
	//		}

	//	}

	//	printf("[st_log_ramk1.csv]file write OK\n");
	//}

	//fclose(fp6);

	////-------------------------------------------------------
	////-------------------------------------------------------
	////発動回数と自分の階級、前のクライアントの階級の関係を出力(rank=2、平民)
	//FILE *fp7;

	//fp7 = fopen("st_log_rank2.csv", "a+");

	//if (fp7 == NULL) {
	//	printf("[st_log_rank2.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++)
	//	{
	//		if (seat_tactics_check[i][0] >= 1 && seat_tactics_check[i][1] == 2) {
	//			fprintf(fp7, "%d,%d,%d,%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1], seat_tactics_check[i][2]);
	//		}

	//	}

	//	printf("[st_log_ramk2.csv]file write OK\n");
	//}

	//fclose(fp7);

	////-------------------------------------------------------
	////-------------------------------------------------------
	////発動回数と自分の階級、前のクライアントの階級の関係を出力(rank=3、貧民)
	//FILE *fp8;

	//fp8 = fopen("st_log_rank3.csv", "a+");

	//if (fp8 == NULL) {
	//	printf("[st_log_rank3.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++)
	//	{
	//		if (seat_tactics_check[i][0] >= 1 && seat_tactics_check[i][1] == 3) {
	//			fprintf(fp8, "%d,%d,%d,%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1], seat_tactics_check[i][2]);
	//		}

	//	}

	//	printf("[st_log_ramk3.csv]file write OK\n");
	//}

	//fclose(fp8);

	////-------------------------------------------------------
	////-------------------------------------------------------
	////発動回数と自分の階級、前のクライアントの階級の関係を出力(rank=4、大貧民)
	//FILE *fp9;

	//fp9 = fopen("st_log_rank4.csv", "a+");

	//if (fp9 == NULL) {
	//	printf("[st_log_rank4.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++)
	//	{
	//		if (seat_tactics_check[i][0] >= 1 && seat_tactics_check[i][1] == 4) {
	//			fprintf(fp9, "%d,%d,%d,%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1], seat_tactics_check[i][2]);
	//		}

	//	}

	//	printf("[st_log_ramk4.csv]file write OK\n");
	//}

	//fclose(fp9);

	////-------------------------------------------------------
	////-------------------------------------------------------
	////席順戦略で予定していた通り前のクライアントで流れたかどうかの評価
	//FILE *fp10;

	//fp10 = fopen("nagare_kakuritu.csv", "a+");

	//if (fp10 == NULL) {
	//	printf("[nagare_kakuritu.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++) {
	//		fprintf(fp10, "%d,%d,%d,%d\n", i, pass_check_lag[0][i], pass_check_lag[1][i], (pass_check_lag[0][i] + pass_check_lag[1][i]));
	//	}

	//	printf("[nagare_kakuritu.csv]file write OK\n");
	//}

	//fclose(fp10);

	//-------------------------------------------------------

	//自分用ゲーム終わりの状態確認終わり---------------------------

	if ((PRINT_SCORE == 1 && my_playernum == 0) || PRINT_SCORE == 2) {
		fprintf(stderr, "     Final score ");//スコア表示
		for (i = 0; i < 5; i++) {
			fprintf(stderr, "%5d", score[i]);
			if (i == 4) {
				fprintf(stderr, "\n");
			}
			else {
				fprintf(stderr, " / ");
			}
		}
	}

	//ソケットを閉じて終了
	if (closeSocket() != 0) {
		printf("failed to close socket\n");
		exit(1);
	}
	exit(0);
}
