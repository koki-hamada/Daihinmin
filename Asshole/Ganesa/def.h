#ifndef DEF_H_
#define DEF_H_

#include "snowl/mydef.h"
#include <cassert>
#include <cstdio>
#include <cstring>

#define MAX_PLAYER_NUM 5
#define MAX_STATE_NUM 100
#define MAX_EVALUATE_NUM 2000
#define MAX_HISTORY_NUM 512

//#define DEBUG_SEARCH
//#define DEBUG_LAST_SEARCH
//#define DEBUG_UPDATE
//#define DEBUG_OBSERVE

//#define MULTI_NETWORK
#define MAX_NET_NUM 4
//#define EXTRA_NETWORK
//#define NO_HISTORY
//#define FILTER_DATA

//#define LOGISTIC_REGRESSION

struct State {
	int64 myCards;			 // 自分のカード
	int64 oppCards;			 // 相手4人のカード
	fieldInfo finfo;		 // 場
	int playerNum;			 // プレイヤー番号
	int lastPlayer;			 // 最後に提出したプレイヤ
	int changePlayerNum;	 // 交換相手
	int64 havingChangeCards; // 交換相手がまだ持っているカード
	int baCards[8][15];		 // 場に出ているカード
	int64 submit[5];		 // 今まで提出したカード
	int skip;
	int pass;
	State()
		: playerNum(-1), lastPlayer(-1), changePlayerNum(-1) {
	}
};

struct SearchData {
	float maxv;											// 評価値の最大値
	bitValidHandsArray maxSubmit;						// 評価値最大のカード
	int random;											// ランダム選択したかどうかのフラグ
	int evalnum;										// 評価候補の数
	bitValidHandsArray submit;							// 提出予定のカード
	bitValidHandsArray submitHistory;					// カード提出の履歴(探索時に使用)
	State states[MAX_EVALUATE_NUM + 1];					// 評価候補の事後状態
	bitValidHandsArray histories[MAX_EVALUATE_NUM + 1]; // 評価候補のカード提出履歴
};

struct History {
	int size;
	State prestate[MAX_HISTORY_NUM];
	State state[MAX_HISTORY_NUM];
	int action[MAX_HISTORY_NUM][8][15];
	bool invalid[MAX_HISTORY_NUM];

	History()
		: size(0){};
	void Add(const State& pres, const int a[8][15], const State& s) {
		assert(size < MAX_HISTORY_NUM);
		prestate[size] = pres;
		for(int i = 0; i < 8; i++) {
			for(int j = 0; j < 15; j++) {
				action[size][i][j] = a[i][j];
			}
		}
		state[size] = s;
		invalid[size] = 0;
		size++;
	}
};


#endif /* DEF_H_ */
