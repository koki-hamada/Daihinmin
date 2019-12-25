#include "Player.h"
#include "def.h"
#include "logger.h"
#include "snowl/bitCard.h"
#include "snowl/cardChange.h"
#include "snowl/cardSelect.h"
#include "snowl/checkInfo.h"
#include "snowl/connection.h"
#include "snowl/handGenerator.h"
#include "snowl/mt19937ar.h"
#include "snowl/mydef.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

extern const int g_logging = 0; //ログ取りをするか否かを判定するための変数

int main(int argc, char* argv[]) {
	// 乱数の初期化
	init_genrand(2583741);


	Player* player = new Player();
	player->Init("weight.txt");

	int whole_gameend_flag = 0; //全ゲームが終了したか否かを判別する変数
	int one_gameend_flag = 0;   // 1ゲームが終わったか否かを判別する変数
	int accept_flag = 0;		//提出したカードが受理されたかを判別する変数
	int game_count = 0;			//ゲームの回数を記憶する
	int game_start_flag[5] = {0};
	int is_my_turn = 0;
	int change_qty = 0;

	int own_cards[8][15] = {0}; //手札のカードテーブルをおさめる変数
	int baCards[8][15] = {0};   //場に出たカードテーブルを納める

	int64 befCards[5] = {0};
	int64 aftCards[5] = {0}; // getCards;
	State prestate[5];
	State state[5];
	const int64 allCards = (1LL << 53) - 1;

	//引数のチェック
	//引数に従ってサーバアドレス、接続ポート、クライアント名を変更
	checkArg(argc, argv);

	//ゲームに参加
	for(int c = 0; c < player->ClientNum(); c++) {
		prestate[c].playerNum = entryToGame(c);
	}

	while(whole_gameend_flag == 0) {
		one_gameend_flag = 0; // 1ゲームが終わった事を示すフラグを初期化

		for(int c = 0; c < player->ClientNum(); c++)
			game_start_flag[c] = 1;

		for(int c = 0; c < player->ClientNum(); c++) {
			game_count = startGame(own_cards, c); //ラウンドを始める 最初のカードを受け取る。
			if(c == 0) {
				// printf("game %d\n", game_count);
			}

			///カード交換
			if(own_cards[5][0] == 0) { //カード交換時フラグをチェック ==1で正常
				printf("not card-change turn?\n");
				exit(1);
			} else { //テーブルに問題が無ければ実際に交換へ
					 // 交換前の手札をビットで保存しておく
				befCards[c] = setBit(own_cards);

				if(own_cards[5][1] > 0 && own_cards[5][1] < 100) {
					change_qty = own_cards[5][1];	//カードの交換枚数
					int select_cards[8][15] = {{0}}; //選んだカードを格納

					//自分が富豪、大富豪であれば不要なカードを選び出す
					checkCards(select_cards, own_cards, change_qty);

					//選んだカードを送信
					sendChangingCards(select_cards, c);
				} else {
					change_qty = 0;
					//自分が平民以下なら、何かする必要はない
				}
			} //カード交換ここまで
		}

		while(one_gameend_flag == 0) { // 1ゲームが終わるまでの繰り返し
			int submitPlayerNum = -1;
			for(int c = 0; c < player->ClientNum(); c++) {
				int select_cards[8][15] = {{0}}; //提出用のテーブル
				is_my_turn = receiveCards(own_cards, c);

				// 提出するプレイヤー番号を登録
				submitPlayerNum = own_cards[5][3];

				// getStates();
				checkState(&prestate[c].finfo, own_cards);

				// ゲーム開始時の処理
				if(game_start_flag[c] == 1) {
					// 自分の手札
					prestate[c].myCards = setBit(own_cards);
					// 自分以外が持つカード
					prestate[c].oppCards = allCards ^ prestate[c].myCards;
					aftCards[c] = prestate[c].myCards;

					// 場のカードを初期化
					memset(baCards, 0, sizeof baCards); // !!!!
					memset(prestate[c].baCards, 0, sizeof prestate[c].baCards);

					// 提出カードの初期化
					memset(prestate[c].submit, 0, sizeof prestate[c].submit);

					// 履歴の初期化
					player->history[prestate[c].playerNum].size = 0;
					game_start_flag[c] = 0;

					// fieldInfo初期化
					prestate[c].finfo.onset = 1;
					prestate[c].finfo.pass = 0;
					prestate[c].finfo.goal = 0;
					prestate[c].finfo.rev = 0;
					prestate[c].finfo.seq = 0;
					prestate[c].finfo.lock = 0;
					prestate[c].finfo.suit = 0;

					// 席順・階級・カードの初期枚数を取得
					for(int i = 0; i < 5; i++)
						prestate[c].finfo.seat[own_cards[6][i + 10]] = i;
					for(int i = 0; i < 5; i++) {
						prestate[c].finfo.lest[prestate[c].finfo.seat[i]] = own_cards[6][i];
						prestate[c].finfo.rank[prestate[c].finfo.seat[i]] = own_cards[6][i + 5];
						if(own_cards[6][i + 10] == prestate[c].playerNum) {
							prestate[c].finfo.mypos = i;
						}
					}

					// 手札交換が行われた時
					if(befCards[c] != aftCards[c]) {
						int myRank = prestate[c].finfo.rank[prestate[c].finfo.seat[prestate[c].playerNum]];
						// 渡した相手を探す
						int sendRank = 4 - myRank;
						for(int i = 0; i < 5; i++) {
							if(prestate[c].finfo.rank[prestate[c].finfo.seat[i]] == sendRank) {
								prestate[c].changePlayerNum = i;
								break;
							}
						}
						// 渡したカード
						prestate[c].havingChangeCards = befCards[c] ^ (befCards[c] & aftCards[c]);
					} else { // 手札交換なし
						prestate[c].havingChangeCards = 0;
						prestate[c].changePlayerNum = -1;
					}
				}

				// 状態を生成
				prestate[c].havingChangeCards &= prestate[c].oppCards;
				memcpy(prestate[c].baCards, baCards, sizeof(baCards));

				// 自分のターンのとき
				if(is_my_turn == 1) {
					// 自分の手札をビットに変換
					prestate[c].myCards = setBit(own_cards);

					//PrintState(prestate[c]);

					// 出すカードを決定する
					player->Select(select_cards, &prestate[c]);

					// 選んだカードを提出
					accept_flag = sendCards(select_cards, c);

					if(setBit(select_cards) != 0ULL && accept_flag == 8) {
						printf("accept error!\n");
						for(int i = 0; i < 8; i++) {
							for(int j = 0; j < 15; j++) {
								printf("%d", select_cards[i][j]);
							}
							printf("\n");
						}
						PrintBitCard(prestate[c].myCards);
						printf("\n");
						PrintBitCard(prestate[c].oppCards);
						printf("\n");
						PrintFieldInfo(&prestate[c].finfo);
						//exit(1);
					}

					// 自分の手札から除く
					assert((prestate[c].myCards & setBit(select_cards)) == setBit(select_cards));
					int64 submitCard = setBit(select_cards);
					prestate[c].myCards ^= submitCard;

					//最後に提出したプレイヤを更新
					if(submitCard != 0LL) {
						prestate[c].lastPlayer = prestate[c].playerNum;
					}
				} else {
					// 自分のターンではない時
					// 意味もなく乱数を進めてみる
					genrand_int32();
				}
			}

			for(int c = 0; c < player->ClientNum(); c++) {
				//そのターンに提出された結果のテーブル受け取り,場に出たカードの情報を解析する
				lookField(baCards, c);

				// ターンの結果から場の情報を更新
				checkField(&prestate[c].finfo, NULL, baCards, 0LL);

				int64 prebaCards = setBit(prestate[c].baCards);
				int64 curbaCards = setBit(baCards);
				// 相手が持ちうる手札の情報を更新
				prestate[c].oppCards ^= (prestate[c].oppCards & curbaCards);
				// パスでないなら最後に提出したプレイヤを更新
				if(prebaCards != curbaCards) {
					prestate[c].lastPlayer = submitPlayerNum;
					// 提出したカードを記録
					int64 submitCards = setBit(baCards);
					prestate[c].submit[submitPlayerNum] |= submitCards;
				}

#ifndef NO_HISTORY
				// 事後状態の生成(評価では使わない)
				state[c].myCards = prestate[c].myCards;
				state[c].oppCards = prestate[c].oppCards;
				state[c].finfo = prestate[c].finfo;
				//state[c].finfo.mypos = mypos[c];
				state[c].playerNum = prestate[c].playerNum; //!!!!
				state[c].lastPlayer = prestate[c].lastPlayer;
				state[c].changePlayerNum = prestate[c].changePlayerNum;
				state[c].havingChangeCards = prestate[c].havingChangeCards & prestate[c].oppCards;
				memcpy(state[c].baCards, baCards, sizeof(baCards));
				memcpy(state[c].submit, prestate[c].submit, sizeof(prestate[c].submit));

				// 履歴を追加
				player->history[prestate[c].playerNum].Add(prestate[c], baCards, state[c]);
#endif
				//一回のゲームが終わったか否かの通知をサーバからうける。
				switch(beGameEnd(c)) {
				case 0: // 0のときゲームを続ける
					one_gameend_flag = 0;
					whole_gameend_flag = 0;
					break;
				case 1: // 1のとき 1ゲームの終了
					one_gameend_flag = 1;
					whole_gameend_flag = 0;
					if(g_logging == 1) {
						printf("game #%d was finished.\n", game_count);
					}
					break;
				default: //その他の場合 全ゲームの終了
					one_gameend_flag = 1;
					whole_gameend_flag = 1;
					if(g_logging == 1) {
						printf("All game was finished(Total %d games.)\n", game_count);
					}
					break;
				}
			}
		} // 1ゲームが終わるまでの繰り返しここまで
	}	 //全ゲームが終わるまでの繰り返しここまで
		  //ソケットを閉じて終了
	for(int c = 0; c < player->ClientNum(); c++) {
		if(closeSocket(c) != 0) {
			printf("failed to close socket\n");
			exit(1);
		}
	}
	return 0;
}
