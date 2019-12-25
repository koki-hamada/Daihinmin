#include "Player.h"
#include "Common.h"
#include "logger.h"
#include "snowl/bitCard.h"
#include "snowl/cardSelect.h"
#include "snowl/handGenerator.h"
#include "snowl/mt19937ar.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

//#define CHECK_INPUT

#ifdef CHECK_INPUT
bool check[1000000];
#endif

void CopyBitValidHandsArray(bitValidHandsArray* dst, const bitValidHandsArray* src) {
	for(int i = 0; i < src->size; i++) {
		dst->hands[i] = src->hands[i];
	}
	dst->size = src->size;
}

int64 RevCard(int64 card) {
	int64 ret = 0;
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 13; j++) {
			if(card >> (13 * i + j) & 1) {
				int nj = 12 - j;
				ret |= 1LL << (13 * i + nj);
			}
		}
	}
	if(card >> 52 & 1) ret |= 1LL << 52;
	return ret;
}

void PrintCard(int64 card) {
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < 13; j++) {
			std::cout << (card >> (13 * i + j) & 1) << " ";
		}
		std::cout << std::endl;
	}
	for(int j = 0; j < 13; j++) {
		std::cout << (card >> 52 & 1) << " ";
	}
	std::cout << std::endl
			  << std::endl;
}

int RevOrder(int ord) {
	return 14 - ord;
}

Player::Player()
	: clientNum(1), randomEps(0) {
}

Player::~Player() {
}

// 初期化(configはネットの重みのファイル名)
void Player::Init(std::string config) {
	std::ifstream ifs(config);
	if(!ifs) {
		std::cerr << "config file(" << config << ") not found" << std::endl;
		exit(1);
	}

	// ネットを作成して読み込む
	net = new Net();
	net->Read(config);
	// 入力画像の情報を取得
	channel = net->layers_[0]->num_input_;
	height = net->layers_[0]->input_height_;
	width = net->layers_[0]->input_width_;
	inputSize = channel * height * width;
}

// 状態を評価する
float Player::Evaluate(const State* state, const History* history) {
	// 状態を入力画像として与える
	if(height == 1) {
		AddInput(net->Input(), state);
	}
	else {
		AddInput(net->Input(), state, history, history->size);
	}
	// フォワードプロパゲーション
	net->Forward();

	if(net->layers_[net->layer_num_ - 1]->num_output_ == 1) {
		return net->Output()[0];
	}

	float out[5] = {0};
	for(int i = 0; i < 5; i++) {
		out[i] = net->Output()[i];
	}
	// 獲得点の期待値を計算
	// 出力をソフトマックス関数に通す
	float max_out = *std::max_element(out, out + 5);
	float exp_sum = 0;
	for(int i = 0; i < 5; i++) {
		out[i] -= max_out;
		out[i] = std::exp(out[i]);
		exp_sum += out[i];
	}
	// 得られるポイントの期待値を計算
	// 1位:4pt, 2位:3pt, ... , 5位:0ptとする
	float sum = 0.0;
	float outsum = 0.0;
	for(int i = 0; i < 5; i++) {
		out[i] /= exp_sum;
		sum += i * out[i];
		outsum += out[i];
	}
	assert(std::abs(outsum - 1.0) < 1e-4);
	return sum;
}

void FillImage(int image[5 * 15], int val, int rows) {
	for(int i = 0; i < rows; i++) {
		for(int j = 0; j < 15; j++) {
			if(val == 1) {
				assert(image[i * 15 + j] == 0);
			}
			image[i * 15 + j] = val;
		}
	}
}

void AddImage(float* input, int image[5 * 15], int rows, int height, int& idx) {
	for(int i = 0; i < rows; i++) {
		for(int j = 0; j < 15; j++) {
			input[idx] = image[i * 15 + j];
#ifdef CHECK_INPUT
			if(check[idx]) {
				std::cout << std::endl;
			}
			assert(check[idx] == 0);
			check[idx] = 1;
			std::cout << image[i * 15 + j] << " ";
			if(j == 14) {
				std::cout << std::endl;
			}
#endif
			idx++;
		}
		idx += (height - 1) * 15;
	}
}

void Player::AddCardImage(float* input, int64 card, int& idx) {
	int image[5 * 15] = {0};
	// 全体
	for(int c = 0; c < 52; c++) {
		if((card >> c) & 1) {
			image[(c / 13) * 15 + (c % 13 + 1)] = 1;
		}
	}
	// 自分のジョーカー
	if((card >> 52) & 1) {
		for(int i = 0; i < 15; i++) {
			image[4 * 15 + i] = 1;
		}
	}

	AddImage(input, image, 5, height, idx);
	// FillImage(image, 0, 5);
}

void Player::AddStateImage(float* input, const State* state, int& idx) {
	const int plNum = state->playerNum;
	const int64 myCard = state->myCards, oppCard = state->oppCards;
	const fieldInfo* finfo = &state->finfo;
	int preIdx = idx;
	// assert(inputSize < MaxInputSize);

	int image[5 * 15] = {0};

	// 自分の手札と相手の手札と提出手札を合わせたら53枚になるかチェック
	int64 all = 0;
	all |= state->myCards;
	assert((all & state->oppCards) == 0);
	all |= state->oppCards;
	for(int i = 0; i < 5; i++) {
		assert((all & state->submit[i]) == 0);
		all |= state->submit[i];
		//std::cout << bitCount(state->submit[i]) << std::endl;
	}
	assert(all == (1LL << 53) - 1);

#ifdef CHECK_INPUT
	PrintState(*state);
	std::cout << "Cards" << std::endl;
#endif
	// カード
	for(int i = 1; i <= 13; i++) {
		image[i] = 1;
		AddImage(input, image, 1, height, idx);
		FillImage(image, 0, 1);
	}

#ifdef CHECK_INPUT
	std::cout << "MyCards" << std::endl;
#endif
	// 自分の手札
	if(!finfo->rev) {
		AddCardImage(input, myCard, idx);
	}
	else {
		AddCardImage(input, RevCard(myCard), idx);
	}

#ifdef CHECK_INPUT
	std::cout << "OppCards" << std::endl;
#endif
	// 相手の手札
	if(!finfo->rev) {
		AddCardImage(input, oppCard, idx);
	}
	else {
		AddCardImage(input, RevCard(oppCard), idx);
	}

	// 場の状態
	AddFieldImage(input, &state->finfo, state->baCards, idx);

	// プレイヤー情報
	AddPlayerInfoImage(input, state, idx);

#ifdef CHECK_INPUT
	for(int i = 0; i < inputSize; i++) {
		assert(check[i]);
		check[i] = 0;
	}

#endif
	//-------------------------------------------------------------
	if(height == 1) {
		if(idx - preIdx != inputSize) {
			std::cerr << idx << " " << inputSize << std::endl;
			std::cerr << "channel is not " << channel << std::endl;
			assert(0);
			exit(1);
		}
	}
}

void Player::AddFieldImage(float* input, const fieldInfo* finfo, const int baCards[8][15], int& idx) {
	int image[5 * 15] = {0};
#ifdef CHECK_INPUT
	std::cout << "Order" << std::endl;
#endif
	// 強さ
	for(int i = 0; i < 1; i++) {
		for(int j = 0; j < 15; j++) {
			if(!finfo->rev) {
				image[i * 15 + j] = (!finfo->onset && j == finfo->ord);
			}
			else {
				int ord = RevOrder(finfo->ord);
				if(finfo->seq) {
					// 階段の時は逆転させた後の一番弱い強さにする
					ord -= finfo->qty - 1;
				}
				image[i * 15 + j] = (!finfo->onset && j == ord);
			}
		}
	}
	AddImage(input, image, 1, height, idx);
	FillImage(image, 0, 1);

#ifdef CHECK_INPUT
	std::cout << "Sequence" << std::endl;
#endif
	// 階段
	// 3枚,4枚,5枚,6枚,7枚
	for(int i = 3; i <= 7; i++) {
		if(!finfo->onset && finfo->seq && finfo->qty == i) {
			FillImage(image, 1, 1);
		}
		AddImage(input, image, 1, height, idx);
		FillImage(image, 0, 1);
	}

#ifdef CHECK_INPUT
	std::cout << "Lock" << std::endl;
#endif
	// 縛り
	if(!finfo->onset && finfo->lock) {
		FillImage(image, 1, 1);
	}
	AddImage(input, image, 1, height, idx);
	FillImage(image, 0, 1);

#ifdef CHECK_INPUT
	std::cout << "Reverse" << std::endl;
#endif
	// 革命
	if(finfo->rev) {
		FillImage(image, 1, 1);
	}
	AddImage(input, image, 1, height, idx);
	FillImage(image, 0, 1);

#ifdef CHECK_INPUT
	std::cout << "Pair" << std::endl;
#endif
	// ペア
	// 1枚,2枚,3枚,4枚,5枚
	for(int i = 1; i <= 5; i++) {
		if(!finfo->onset && !finfo->seq && finfo->qty == i) {
			FillImage(image, 1, 1);
		}
		AddImage(input, image, 1, height, idx);
		FillImage(image, 0, 1);
	}

#ifdef CHECK_INPUT
	std::cout << "BaCards" << std::endl;
#endif
	// 場のカード
	for(int i = 0; i < 5; i++) {
		for(int j = 0; j < 15; j++) {
			if(baCards[i][j] > 0) {
				if(!finfo->rev) {
					image[i * 15 + j] = 1;
				}
				else {
					image[i * 15 + RevOrder(j)] = 1;
				}
			}
		}
	}
	AddImage(input, image, 5, height, idx);
	// FillImage(image, 0, 5);
}

void Player::AddPlayerInfoImage(float* input, const State* state, int& idx) {
	int image[5 * 15] = {0};
	int plNum = state->playerNum;
	int mypos = state->finfo.mypos;
	const fieldInfo* finfo = &state->finfo;

	assert(0 <= mypos && mypos < 5);
	// 各プレイヤの状況
	// 順番([0]自分, [1]:次のプレイヤ, [2]:次の次,…)
	int order[5] = {-1, -1, -1, -1, -1};
	for(int i = 0; i < 5; i++) {
		for(int j = 0; j < 5; j++) {
			if((mypos + i) % 5 == finfo->seat[j]) {
				order[i] = j;
				break;
			}
		}
	}
#ifdef CHECK_INPUT
	std::cout << "Submit" << std::endl;
#endif
	// 提出したカード(全員)
	for(int i = 0; i < 5; i++) {
		int64 submit = 0;
		if(!finfo->rev) {
			submit = state->submit[order[i]];
		}
		else {
			submit = RevCard(state->submit[order[i]]);
		}
		AddCardImage(input, submit, idx);
	}

#ifdef CHECK_INPUT
	std::cout << "Rest" << std::endl;
#endif
	// プレイヤの残り枚数(全員)
	for(int i = 0; i < 5; i++) {
		if(i == 0) {
			int64 myCard = state->myCards;
			//std::cout << bitCount(myCard) << " " << finfo->lest[mypos] << std::endl;
			//PrintState(*state);
			assert(bitCount(myCard) == finfo->lest[mypos]);
		}
		// for(int j = 1; j <= 11; j++) {
		// 	if(finfo->lest[finfo->seat[order[i]]] >= j) {
		// 		FillImage(image, 1, 1);
		// 	}
		// 	AddImage(input, image, 1, height, idx);
		// 	FillImage(image, 0, 1);
		// }
		for(int j = 0; j <= 11; j++) {
			if(finfo->lest[finfo->seat[order[i]]] == j) {
				FillImage(image, 1, 1);
			}
			AddImage(input, image, 1, height, idx);
			FillImage(image, 0, 1);
		}
	}

#ifdef CHECK_INPUT
	std::cout << "Pass or Goal" << std::endl;
#endif
	// パスまたはあがりかどうか(全員)
	for(int i = 0; i < 5; i++) {
		if(((finfo->pass >> finfo->seat[order[i]]) & 1) || ((finfo->goal >> finfo->seat[order[i]]) & 1)) {
			FillImage(image, 1, 1);
		}
		AddImage(input, image, 1, height, idx);
		FillImage(image, 0, 1);
	}

#ifdef CHECK_INPUT
	std::cout << "Having" << std::endl;
#endif
	// 確定手札(自分以外)
	for(int i = 1; i < 5; i++) {
		if(state->changePlayerNum == order[i]) {
			int64 cards = 0;
			if(!finfo->rev) {
				cards = state->havingChangeCards;
			}
			else {
				cards = RevCard(state->havingChangeCards);
			}
			AddCardImage(input, cards, idx);
		}
		else {
			// 交換対象でないプレイヤは0で埋める
			// AddCardImage(input, 0LL, idx);
			AddImage(input, image, 5, height, idx);
		}
	}

	// 階級
	/*
     for (int i = 0; i < 5; i++) {
     int rnk = finfo->rank[finfo->seat[order[i]]];
     for (int j = 0; j < 15; j++) {
     //image[rnk][j] = 1;
     }
     AddImage(input, image, 5, w);
     FillImage(image, 0);
     }*/
}

void Player::AddPlayerSeatImage(float* input, const State* state, int& idx) {
	int plNum = state->playerNum;
	assert(0 <= plNum && plNum < 5);
	int mypos = state->finfo.mypos;
	int dist = -1;
	// myPlayerから見て何人先の人か調べる
	for(int i = 0; i < 5; i++) {
		if((mypos + i) % 5 == state->finfo.seat[plNum]) {
			dist = i;
			break;
		}
	}
	assert(0 <= dist && dist < 5);

	int image[5 * 15] = {0};
	for(int i = 0; i < 5; i++) {
		for(int j = 0; j < 15; j++) {
			image[i * 15 + j] = (i == dist);
		}
	}
	AddImage(input, image, 5, height, idx);
}

// 状態を入力画像として与える
void Player::AddInput(float* input, const State* state) {
	int idx = 0;
	AddStateImage(input, state, idx);
}

void Player::AddInput(float* input, const State* state, const History* history, int size) {
	int myPlayerNum = state->playerNum;
	int idx = 0;
	// プレイした人の席と行動分を0で埋める
	int image[5 * 15] = {0};
	AddImage(input, image, 5, height, idx);
	AddCardImage(input, 0LL, idx);

	// 事後状態追加
	AddStateImage(input, state, idx);

	// height - 1個の状態と行動を追加
	for(int i = 0; i < height - 1; i++) {
		int id = size - 1 - i; // iターン前
#ifdef NO_HISTORY
		id = -1; // 常に0埋め
#endif
		idx = (i + 1) * width;
		if(id >= 0) {
			// プレイした人の席
			AddPlayerSeatImage(input, &history->state[id], idx);
			// 行動
			for(int i = 0; i < 5; i++) {
				for(int j = 0; j < 15; j++) {
					image[i * 15 + j] = history->action[id][i][j];
				}
			}
			AddImage(input, image, 5, height, idx);
			// 事前状態
			AddStateImage(input, &history->prestate[id], idx);
		}
		else {
			// 空きは0で埋める
			memset(image, 0, sizeof image);
			for(int i = 0; i < channel; i++) {
				AddImage(input, image, 1, height, idx);
			}
		}
	}

#ifdef CHECK_INPUT
	for(int i = 0; i < inputSize; i++) {
		assert(check[i]);
		check[i] = 0;
	}
#endif
}

// 深さ優先探索で手と事後状態を列挙
float Player::SearchHand(const int depth, const State* state, const float visitProb, const bool continuous) {
	const int plNum = state->playerNum;
	const int64 myCards = state->myCards, oppCards = state->oppCards;
	const fieldInfo* finfo = &state->finfo;

	// 訪れたかチェック
	float hashval = 0;
	int64 hash = hashtable.GetHash(myCards, 0LL, finfo);
	if(hashtable.Find(hash, hashval)) {
		return hashval;
	}

	bitValidHandsArray vha, vhdG, vhdS;
	generateAllHands(&vhdG, &vhdS, myCards);
	getAllValidHands(&vha, &vhdG, &vhdS, finfo, myCards);

	// 上がれる手が存在する場合それを提出
	for(int i = 0; i < vha.size; i++) {
		if(vha.hands[i].qty == finfo->lest[finfo->seat[plNum]]) {
			// 提出履歴にpush
			pushValidHands(&sd[plNum].submitHistory, &vha.hands[i]);

			// 連続提出中なら上がり手として保存
			if(continuous && sd[plNum].maxv < INF) {
				sd[plNum].maxv = INF;
				CopyBitValidHandsArray(&sd[plNum].maxSubmit, &sd[plNum].submitHistory);
#ifdef DEBUG_SEARCH
				// 提出したカードを出力
				printf("(goal) ");
				PrintBitValidHandsArray(&sd[plNum].submitHistory);
#endif
			}

			// 提出履歴からpop
			sd[plNum].submitHistory.size--;

			float value = 4 - CountGoal(finfo);
			// 獲得点を返す
			return value;
		}
	}

	// 空場でないならパス追加
	if(finfo->onset == 0) {
		vha.hands[vha.size].hands = 0ULL;
		vha.hands[vha.size].qty = 0;
		vha.hands[vha.size].ord = 0; // 初期化されずに偶然6が入ると8切り扱いされる
		vha.hands[vha.size++].seq = 0;
	}

	// 手をシャッフル
	// ShuffleHands(&vha);

	float currentMaxv = -INF;
	for(int i = 0; i < vha.size; i++) {
		fieldInfo nextfinfo = *finfo;
		int64 nextMyCards = myCards;
		// 提出履歴にpush
		pushValidHands(&sd[plNum].submitHistory, &vha.hands[i]);

		int lastPlayer = state->lastPlayer;
		//パスでないなら最後に提出した人を自分に更新
		if(vha.hands[i].hands != 0) {
			lastPlayer = plNum;
		}

		// カードを提出し，もう一度自分の番が来るかチェック
		int result = simulateSubmit2(&vha.hands[i], &nextfinfo, &nextMyCards, oppCards, plNum, lastPlayer);

		// 場と自分の手札を更新して次の状態を生成
		State nextState = *state;
		nextState.finfo = nextfinfo;
		setSubmitCard(nextState.baCards, &vha.hands[i]);
		assert((nextState.submit[plNum] & myCards) == 0);
		nextState.submit[plNum] |= myCards - nextMyCards;
		nextState.myCards = nextMyCards;
		nextState.lastPlayer = lastPlayer;

		float value = 0;
		int64 nexthash = hashtable.GetHash(nextMyCards, state->oppCards, &nextfinfo);
		// ハッシュテーブルに登録されていないなら評価
		if(!hashtable.Find(nexthash, value)) {

			if(result) {
				// 必ず流せるなら潜る
				value = SearchHand(depth + 1, &nextState, visitProb, continuous);
			}
			else {
				float resetProb = 0.0;
				value = Evaluate(&nextState, nullptr);

/*
                // 流せる確率を出す
				resetProb = predicter.Predict(&nextState);
				// 閾値を下回ったらデフォルトのネットワーク(ネットワーク0)で評価
				if(visitProb * resetProb < 0.8) {
					//SwitchNet(0);
					value = Evaluate(&nextState, nullptr);
				}
				else {
					exit(1);
					// v1：流せた時の得点, v２：流せなかった時の得点
					float v1 = 0, v2 = 0;
					if(resetProb > 0.01) {
						// 連続提出フラグをfalseにして潜る
						v1 = SearchHand(depth + 1, &nextState, visitProb * resetProb, false);
					}
					if(resetProb < 0.99) {
						// 連続提出出来なかった場合をネットワーク1で評価
						SwitchNet(1);
						v2 = Evaluate(&nextState, nullptr);
					}
					value = resetProb * v1 + (1.0 - resetProb) * v2;
                }
*/
#ifdef DEBUG_SEARCH
				// 評価値を出力
				printf("(%.2f) ", value);
				PrintBitValidHandsArray(&sd[plNum].submitHistory);
#endif
				// 評価値最大が更新されたらその手を登録
				// 連続提出中だったとき追加
				if(continuous && sd[plNum].maxv < value) {
					sd[plNum].maxv = value;
					CopyBitValidHandsArray(&sd[plNum].maxSubmit, &sd[plNum].submitHistory);
				}
			}
			// 登録
			hashtable.Add(nexthash, value);
		}

		currentMaxv = std::max(currentMaxv, value);

		// 提出履歴からpop
		sd[plNum].submitHistory.size--;
		// 上がり手が見つかったら切り上げ
		if(sd[plNum].maxv == INF) break;
	}

	// ハッシュテーブルに登録
	hashtable.Add(hash, currentMaxv);
	return currentMaxv;
}

// 提出するカードを選択する
void Player::Select(int sendCard[8][15], const State* state) {
	const int plNum = state->playerNum;
	const int64 myCards = state->myCards, oppCards = state->oppCards;
	const fieldInfo* finfo = &state->finfo;

#ifdef DEBUG_SEARCH
	printf("player%d\n", plNum);
	PrintBitCard(myCards);
	printf("\n");
	PrintBitCard(oppCards);
	printf("\n");
	PrintFieldInfo(finfo);
#endif

	bitValidHand submit_vh;
	bitValidHandsArray vha, vhdG, vhdS;
	generateAllHands(&vhdG, &vhdS, myCards);
	getAllValidHands(&vha, &vhdG, &vhdS, finfo, myCards);

	if(finfo->onset == 0) {
		vha.hands[vha.size].hands = 0ULL;
		vha.hands[vha.size].qty = 0;
		vha.hands[vha.size].ord = 0;
		vha.hands[vha.size++].seq = 0;
	}

	// 提出予定の手札があるとき、合法手に含まれているかチェック
	if(sd[plNum].submit.size > 0) {
		bool ok = false;
		for(int i = 0; i < vha.size; i++) {
			if(SameHand(&sd[plNum].submit.hands[0], &vha.hands[i])) {
				ok = true;
				break;
			}
		}
		// 含まれていなかったら探索し直す
		if(!ok) {
			sd[plNum].submit.size = 0;
		}
	}

	// ランダム行動の印をリセット
	sendCard[7][0] = 0;
	if(sd[plNum].submit.size == 0) {
		sd[plNum].random = 0;
		// PrintBitValidHandsArray(&vha);
		//探索
		// 残り2人だったら詰み探索
		if(bitCount(finfo->goal) == 3) {
#ifdef DEBUG_LAST_SEARCH
			printf("last search\n");
			PrintBitCard(myCards);
			std::cout << myCards << std::endl;
			PrintBitCard(oppCards);
			std::cout << oppCards << std::endl;
			PrintFieldInfo(finfo);
			clock_t start = clock();
#endif
			int v = lastSearchTop(myCards, oppCards, finfo);
			// 勝ち手札が見つかったらそれを選択
			if(v != -1) {
				sd[plNum].submit.hands[0] = vha.hands[v];
				sd[plNum].submit.size++;
			}
#ifdef DEBUG_LAST_SEARCH
			clock_t end = clock();
			printf("time %f\n", double(end - start) / CLOCKS_PER_SEC);
			printf("v %d\n", v);
			if(v != -1) {
				printf("win hand ");
				PrintBitValidHandsArray(&sd[plNum].submit);
			}
			printf("\n");
#endif
		}

		// 3人以上残っている、または詰みが見つからなかったとき探索
		if(sd[plNum].submit.size == 0) {
			// 初期化
			hashtable.Init();
			sd[plNum].maxv = -INF;
			sd[plNum].evalnum = 0;
			sd[plNum].submitHistory.size = 0;
#ifdef DEBUG_SEARCH
			clock_t start = clock();
#endif
			// 評価値最大の手を見つける
			float result = SearchHand(0, state, 1.0, true);
			// 評価値最大の手を提出予定の手とする
			CopyBitValidHandsArray(&sd[plNum].submit, &sd[plNum].maxSubmit);
#ifdef DEBUG_SEARCH
			// 評価値と手を出力
			printf("maxv %.6f\n", sd[plNum].maxv);
			PrintBitValidHandsArray(&sd[plNum].maxSubmit);
			clock_t end = clock();
			printf("time %f\n", double(end - start) / CLOCKS_PER_SEC);
#endif
		}
	}

#ifdef DEBUG_SEARCH
	printf("hand queue\n");
	for(int i = 0; i < 5; i++) {
		PrintBitValidHandsArray(&sd[i].submit);
	}
#endif

	// submitの先頭の手を提出
	submit_vh = sd[plNum].submit.hands[0];
	for(int i = 1; i < sd[plNum].submit.size; i++) {
		sd[plNum].submit.hands[i - 1] = sd[plNum].submit.hands[i];
	}
	sd[plNum].submit.size--;
	// ランダム行動したフラグがたっていたら配列の左下に印を付けておく
	if(sd[plNum].random) {
		sendCard[7][0] = 1;
	}

#ifdef DEBUG_SEARCH
	printf("submit\n");
	PrintBitCard(submit_vh.hands);
	printf("\n\n");
#endif
	if(submit_vh.hands == 0ULL) {
		return;
	}
	// カードを配列にセット
	setSubmitCard(sendCard, &submit_vh);
	return;
}

// 手をシャッフル
void ShuffleHands(bitValidHandsArray* vha) {
	int t;
	bitValidHand temp;
	for(int i = 0; i < vha->size; i++) {
		t = i + genrand_int31() % (vha->size - i);
		temp = vha->hands[i];
		vha->hands[i] = vha->hands[t];
		vha->hands[t] = temp;
	}
}

void Player::SwitchNet(int id) {
}
