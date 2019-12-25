/* cardSelect.c : モンテカルロで提出するカードを決定する */
/* Author       : Fumiya Suto                            */
#include <arpa/inet.h>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../logger.h"
#include "bitCard.h"
#include "estimateHand.h"
#include "handGenerator.h"
#include "mt19937ar.h"

// 評価関数で考慮する手役
const int seq5Size = 9;   // 5枚階段  3〜Jの9通り
const int seq4Size = 10;  // 4枚階段  3〜Qの10通り
const int seq3Size = 11;  // 3枚階段  3〜Kの11通り
const int groupSize = 52; // 1〜4枚組 3〜2の13通り × (1〜4)枚 = 52通り
const int jokerSize = 1;  // jokerは1枚

// モンテカルロ中のシミュレーションに使うパラメータ
const double param[166] = {
	// for Normal
	// 5-Sequence
	-0.165103445, -1.367353957, -1.084887944, -0.103446973, 0.005614329, // 3-7
	-0.177613166, 0.020646977, -0.204253239, 0.018329075,				 // 8-J
	// 4-Sequence
	-1.250221599, -1.305045504, -1.456065252, -0.889654668, -2.987781810, // 3-7
	-1.388516263, -1.176554458, -0.794348341, -2.055029278, -1.890634784, // 8-Q
	// 3-Sequence
	-3.704179332, -4.281589257, -4.719447787, -4.495022219, // 3-6
	-5.089943875, -6.087235981, -2.976696921, -4.422503509, // 7-T
	-4.221070745, -4.222361704, -4.002477132,				// J-K
	// Solo, Group(2-4pair)
	-3.902118723, -5.356407798, -4.459366682, -1.673269738,				 // 3
	-3.813512676, -5.646004771, -4.959193566, -0.598674962,				 // 4
	-3.707419103, -5.827590172, -5.076551563, -0.455296345,				 // 5
	-2.914792373, -5.043195734, -4.436616845, -0.790830702,				 // 6
	-2.659857260, -4.498763022, -3.548758188, -0.164144807,				 // 7
	-6.162883027, -7.620724057, -6.552047173, -0.409127296,				 // 8
	-3.377120658, -4.432217918, -3.604395246, -0.447012953,				 // 9
	-2.931679825, -3.827014698, -2.405417588, -0.186615449,				 // T
	-2.977701799, -3.088013233, -1.833889336, -0.034788011,				 // J
	-3.180260758, -2.933643112, -1.445919210, 0.025592376,				 // Q
	-3.039855900, -2.900483696, -1.851938754, 0.005744616,				 // K
	-2.396018281, -2.156369375, -1.014889482, 0.035431113,				 // A
	-2.386842708, -2.704110364, -2.010499011, -0.169877329,				 // 2
	0.557952043,														 // JOKER
																		 // for Revolution
																		 // 5-Sequence
	0.015941512, -0.076186286, -1.869357120, -0.280747683, -0.003119470, // 3-7
	-0.892415555, 0.001491174, -0.634278907, -1.064733776,				 // 8-J
	// 4-Sequence
	-1.496744051, -0.825525999, -2.733283880, -0.894995405, -1.898804633, // 3-7
	-1.538144313, -1.025547067, -0.837688942, -1.185448678, -3.504837011, // 8-Q
	// 3-Sequence
	-4.213282878, -3.897294649, -4.012261980, -4.517719657, // 3-6
	-5.015807231, -5.507484021, -4.051525440, -4.608462404, // 7-T
	-4.057425508, -4.326998459, -5.129364509,				// J-K
	// Solo, Group(2-4pair)
	-2.415693628, -2.782669751, -1.843946193, -0.004313174, // 3
	-2.704387616, -2.350158654, -1.209685749, 0.019062053,  // 4
	-3.187466989, -2.919736669, -1.705203490, -0.060453580, // 5
	-3.442788490, -3.467241349, -2.027801109, -0.012216187, // 6
	-3.321312748, -3.495115522, -2.244767429, -0.129789376, // 7
	-5.744447193, -5.757793499, -5.355164182, -2.659961621, // 8
	-3.147674227, -3.078045473, -2.287018461, -0.368540465, // 9
	-2.965633813, -4.850912354, -3.522098268, 0.016110389,  // T
	-3.047409384, -4.952092305, -4.031908245, -0.387839210, // J
	-3.378235820, -5.678286406, -4.574724270, -0.436443491, // Q
	-3.720091654, -6.173770806, -5.468976055, -0.706931919, // K
	-4.073372779, -6.424423870, -5.079275011, -0.898643720, // A
	-4.051587944, -5.937719397, -5.274388701, -2.298538090, // 2
	0.848130302,											// JOKER
};

// 選択した手で8切りが発生するかをチェックする
int checkEight(const bitValidHand* vh) {
	if(vh->seq == 0) {
		return vh->ord == 6;
	} else {
		return vh->ord <= 6 && 6 <= vh->ord + vh->qty - 1;
	}
}

int checkSpade3(const bitValidHand* vh) {
	// 未検証
	return vh->hands == 1LL && (vh->ord == 0 || vh->ord == 14);
}

// 選択した手で革命が発生するかをチェックする
int checkRev(const bitValidHand* vh) {
	// ペアなら4枚以上，階段なら5枚以上で革命
	return (4 + vh->seq) <= vh->qty;
}

// ランダムにカードを分配する
void randomCardDevide(int64 playersCard[5], int64 myCards, int64 oppCards, const changeInfo* cinfo, const fieldInfo* finfo) {
	int i, j;
	int pos, sum, count = 0;
	int card[5];
	for(i = 0; i < 5; i++) {
		card[i] = finfo->lest[i];
		playersCard[i] = 0LL;
		if(i != finfo->mypos) count += finfo->lest[i];
	}
	playersCard[finfo->mypos] = myCards;

	int64 opp = oppCards;
	int64 chg = ((cinfo->chgCards) & oppCards);
	// 交換に出したカード持ち主を確定させる
	if(chg != 0LL) {
		sum = 0;
		for(i = 0; i < 53; i++)
			sum += ((chg >> i) & 1);
		for(i = 0; i < 5; i++) {
			if(finfo->rank[i] + finfo->rank[finfo->mypos] == 4) break;
		}
		playersCard[i] |= chg;
		opp ^= chg;
		card[i] -= sum;
		count -= sum;
	}
	// 最初に行動したプレイヤにダイヤの3を分配する
	if((opp & (1LL << 26)) && cinfo->firstPlayer != -1) {
		playersCard[cinfo->firstPlayer] |= (1LL << 26);
		opp ^= (1LL << 26);
		card[cinfo->firstPlayer]--;
		count--;
	}
	if(cinfo->chgCards != 0LL) {
		// JOKERを平民以上の人に分配する
		if(((opp >> 52) & 1) == 1LL) {
			int cnt = 0;
			for(i = 0; i < 5; i++)
				if(finfo->rank[i] < 3 && i != finfo->mypos) cnt += card[i];
			sum = 0;
			pos = genrand_int32() % cnt;
			for(i = 0; i < 5; i++) {
				if(finfo->rank[i] > 2 || i == finfo->mypos) continue;
				sum += card[i];
				if(pos < sum) break;
			}
			playersCard[i] |= (1LL << 52);
			card[i]--;
			count--;
			opp ^= (1LL << 52);
		}
		// スペードの2を貧民以上の人に分配する
		if(((opp >> 12) & 1) == 1LL) {
			int cnt = 0;
			for(i = 0; i < 5; i++)
				if(finfo->rank[i] < 4 && i != finfo->mypos) cnt += card[i];
			sum = 0;
			pos = genrand_int32() % cnt;
			for(i = 0; i < 5; i++) {
				if(finfo->rank[i] == 4 || i == finfo->mypos) continue;
				sum += card[i];
				if(pos < sum) break;
			}
			playersCard[i] |= (1LL << 12);
			card[i]--;
			count--;
			opp ^= (1LL << 12);
		}
	}
	// 自分が富豪以上の場合，交換相手が持っていないことが分かるカードを分配する
	if((opp & (cinfo->notCards)) != 0LL) {
		for(i = 0; i < 52; i++) {
			if((((opp & (cinfo->notCards)) >> i) & 1) == 0LL) continue;
			int cnt = 0;
			for(j = 0; j < 5; j++)
				if(finfo->rank[j] + finfo->rank[finfo->mypos] != 4 && j != finfo->mypos) cnt += card[j];
			sum = 0;
			pos = genrand_int32() % cnt;
			for(j = 0; j < 5; j++) {
				if(finfo->rank[j] + finfo->rank[finfo->mypos] == 4 || j == finfo->mypos) continue;
				sum += card[j];
				if(pos < sum) break;
			}
			playersCard[j] |= (1LL << i);
			card[j]--;
			count--;
			opp ^= (1LL << i);
		}
	}
	// 残りのカードをランダムに分配する
	for(i = 0; i < 53; i++) {
		if(((opp >> i) & 1) == 1 && count != 0) {
			pos = genrand_int32() % count;
			sum = 0;
			for(j = 0; j < 5; j++) {
				if(j == finfo->mypos) continue;
				sum += card[j];
				if(pos < sum) break;
			}
			playersCard[j] |= (1LL << i);
			card[j]--;
			count--;
		}
	}
}

// パスの処理を行う
void simulatePass(fieldInfo* simField, int* current) {
	simField->pass |= (1 << (*current));
	// 全員がパスした状態
	if(simField->pass == (1 << 5) - 1) {
		// プレイヤーの状態をリセット．上がったプレイヤーはパス扱い
		simField->pass = simField->goal;
		simField->onset = 1;
		// ロック解除
		simField->lock = 0;
		// マークをリセット
		simField->suit = 0;
	} else {
		*current = (*current + 1) % 5;
	}
}

// カードの提出処理を行う
void simulateSubmit(const bitValidHand* vh, fieldInfo* simField, int64 playersCard[5], int* current) {
	// 出したカードを手札から除く．
	playersCard[*current] ^= vh->hands;
	// カードの残り枚数を更新する
	simField->lest[*current] -= vh->qty;

	if(checkEight(vh) == 1) {
		// プレイヤーの状態をリセット．上がったプレイヤーはパス扱い
		simField->pass = simField->goal;
		simField->onset = 1;
		// ロック解除
		simField->lock = 0;
		// マークをリセット
		simField->suit = 0;
		// 革命判定
		if(checkRev(vh)) simField->rev = !(simField->rev);
		// カード枚数・強さ・階段フラグはonset=1のとき使わない
		// currentはそのまま
	} else {
		// 場の状況を更新
		// カードを出したので onset = 0
		simField->onset = 0;
		// 場に出ているカード枚数
		simField->qty = vh->qty;
		// ロック判定
		if(simField->suit == vh->suit) simField->lock = 1;
		// 場に出ているマーク
		simField->suit = vh->suit;
		// 場に出ているカードの強さ
		simField->ord = vh->ord;
		// 場に出ているカードが階段かどうか
		simField->seq = vh->seq;
		// 革命判定
		if(checkRev(vh)) simField->rev = !(simField->rev);
		// 順番を一つ進める
		*current = ((*current) + 1) % 5;
	}
}

double calcValueUnrollVH(int64 myCards, int rev, bitValidHandsArray* vhS) {

	int handsSize = seq5Size + seq4Size + seq3Size + groupSize + jokerSize; // 考慮する手役の数(9+10+11+52+1=83通り)

	// 評価関数上の手役のインデックス
	int seq5Index = 0;
	int seq4Index = seq5Index + seq5Size;	// = 0+9  = 9;
	int seq3Index = seq4Index + seq4Size;	// = 9+10 = 19;
	int groupIndex = seq3Index + seq3Size;   // = 19+11 = 30;
	int jokerIndex = groupIndex + groupSize; // = 30+52 = 82;

	// 評価関数で考慮する項目
	int revSize = 2; // 革命時かどうか。通常・革命の2通り

	// 合計パラメータ数
	int paramNum = revSize * handsSize; // = 2*83 = 166;

	double res = 0.0;
	int64 solo = myCards;
	int vhSize = 0;
	int i;

	for(i = 0; i < vhS->size; i++) {
		if((vhS->hands[i].hands & myCards) != vhS->hands[i].hands) continue;
		vhSize++;
		unsigned char suit = vhS->hands[i].suit;
		unsigned char ord = vhS->hands[i].ord;
		int j = i + 1;
		while(1) {
			if(j < vhS->size && vhS->hands[j].suit == suit && vhS->hands[j].ord == ord && ((vhS->hands[j].hands & myCards) == vhS->hands[j].hands))
				j++;
			else {
				j--;
				break;
			}
		}
		solo ^= vhS->hands[j].hands;
		unsigned char qty = vhS->hands[j].qty;
		if(vhS->hands[j].qty == 3) {
			res += param[handsSize * rev + ord - 1 + seq3Index];
		} else if(vhS->hands[j].qty == 4) {
			res += param[handsSize * rev + ord - 1 + seq4Index];
		} else {
			res += param[handsSize * rev + ord - 1 + seq5Index];
		}
		i = j + 1;
		while(i < vhS->size && vhS->hands[i].suit == suit && vhS->hands[i].ord < ord + qty)
			i++;
		i--;
	}
	if(vhSize != 0) {
#ifdef USE_BIT64
		int64 card = myCards;
		int64 group = 0x0000008004002001ULL;
		for(i = 0; i < 13; i++) {
			int64 pr = (card & group);
			if(pr) {
				int count = (int)(((pr * group) >> 39) & 7);
				if(count > 1)
					res += param[handsSize * rev + (4 * i + count - 1) + groupIndex];
				else if(solo & group)
					res += param[handsSize * rev + 4 * i + groupIndex];
			}
			card = card >> 1;
			solo = solo >> 1;
		}

#else
		// 手札のGroupを調べる．強さ3〜2の13通り
		// 強さiのカードの枚数を数える(bitCount)
		unsigned int soloLow = (unsigned int)solo, soloHi = (unsigned int)(solo >> 32);
		unsigned int cardLow = (unsigned int)myCards, cardHi = (unsigned int)(myCards >> 32);
		unsigned int groupLow = 0x04002001, groupHi = 0x00000080;

		unsigned int pr = (cardLow & groupLow);
		int count = ((cardHi & groupHi) != 0) + (((pr >> 26) + (pr >> 13) + pr) & 3);

		// groupをチェック
		if(count > 1) {
			res += param[handsSize * rev + (4 * 0 + count - 1) + groupIndex];
		} else if((soloHi & groupHi) || (soloLow & groupLow)) {
			res += param[handsSize * rev + (4 * 0) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		soloLow = (soloLow >> 1) | (soloHi << (32 - 1));
		soloHi = soloHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0) + ((pr * groupLow) >> 26);
		// groupをチェック
		if(count > 1) {
			res += param[handsSize * rev + (4 * 1 + count - 1) + groupIndex];
		} else if((soloHi & groupHi) || (soloLow & groupLow)) {
			res += param[handsSize * rev + (4 * 1) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		soloLow = (soloLow >> 1) | (soloHi << (32 - 1));
		soloHi = soloHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0) + ((pr * groupLow) >> 26);
		// groupをチェック
		if(count > 1) {
			res += param[handsSize * rev + (4 * 2 + count - 1) + groupIndex];
		} else if((soloHi & groupHi) || (soloLow & groupLow)) {
			res += param[handsSize * rev + (4 * 2) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		soloLow = (soloLow >> 1) | (soloHi << (32 - 1));
		soloHi = soloHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0) + ((pr * groupLow) >> 26);
		// groupをチェック
		if(count > 1) {
			res += param[handsSize * rev + (4 * 3 + count - 1) + groupIndex];
		} else if((soloHi & groupHi) || (soloLow & groupLow)) {
			res += param[handsSize * rev + (4 * 3) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		soloLow = (soloLow >> 1) | (soloHi << (32 - 1));
		soloHi = soloHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0) + ((pr * groupLow) >> 26);
		// groupをチェック
		if(count > 1) {
			res += param[handsSize * rev + (4 * 4 + count - 1) + groupIndex];
		} else if((soloHi & groupHi) || (soloLow & groupLow)) {
			res += param[handsSize * rev + (4 * 4) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		soloLow = (soloLow >> 1) | (soloHi << (32 - 1));
		soloHi = soloHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0) + ((pr * groupLow) >> 26);
		// groupをチェック
		if(count > 1) {
			res += param[handsSize * rev + (4 * 5 + count - 1) + groupIndex];
		} else if((soloHi & groupHi) || (soloLow & groupLow)) {
			res += param[handsSize * rev + (4 * 5) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		soloLow = (soloLow >> 1) | (soloHi << (32 - 1));
		soloHi = soloHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0) + ((pr * groupLow) >> 26);
		// groupをチェック
		if(count > 1) {
			res += param[handsSize * rev + (4 * 6 + count - 1) + groupIndex];
		} else if((soloHi & groupHi) || (soloLow & groupLow)) {
			res += param[handsSize * rev + (4 * 6) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		soloLow = (soloLow >> 1) | (soloHi << (32 - 1));
		soloHi = soloHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0) + ((pr * groupLow) >> 26);
		// groupをチェック
		if(count > 1) {
			res += param[handsSize * rev + (4 * 7 + count - 1) + groupIndex];
		} else if((soloHi & groupHi) || (soloLow & groupLow)) {
			res += param[handsSize * rev + (4 * 7) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		soloLow = (soloLow >> 1) | (soloHi << (32 - 1));
		soloHi = soloHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0) + ((pr * groupLow) >> 26);
		// groupをチェック
		if(count > 1) {
			res += param[handsSize * rev + (4 * 8 + count - 1) + groupIndex];
		} else if((soloHi & groupHi) || (soloLow & groupLow)) {
			res += param[handsSize * rev + (4 * 8) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		soloLow = (soloLow >> 1) | (soloHi << (32 - 1));
		soloHi = soloHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0) + ((pr * groupLow) >> 26);
		// groupをチェック
		if(count > 1) {
			res += param[handsSize * rev + (4 * 9 + count - 1) + groupIndex];
		} else if((soloHi & groupHi) || (soloLow & groupLow)) {
			res += param[handsSize * rev + (4 * 9) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		soloLow = (soloLow >> 1) | (soloHi << (32 - 1));
		soloHi = soloHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0) + ((pr * groupLow) >> 26);
		// groupをチェック
		if(count > 1) {
			res += param[handsSize * rev + (4 * 10 + count - 1) + groupIndex];
		} else if((soloHi & groupHi) || (soloLow & groupLow)) {
			res += param[handsSize * rev + (4 * 10) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		soloLow = (soloLow >> 1) | (soloHi << (32 - 1));
		soloHi = soloHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0) + ((pr * groupLow) >> 26);
		// groupをチェック
		if(count > 1) {
			res += param[handsSize * rev + (4 * 11 + count - 1) + groupIndex];
		} else if((soloHi & groupHi) || (soloLow & groupLow)) {
			res += param[handsSize * rev + (4 * 11) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		soloLow = (soloLow >> 1) | (soloHi << (32 - 1));
		soloHi = soloHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0) + ((pr * groupLow) >> 26);
		// groupをチェック
		if(count > 1) {
			res += param[handsSize * rev + (4 * 12 + count - 1) + groupIndex];
		} else if((soloHi & groupHi) || (soloLow & groupLow)) {
			res += param[handsSize * rev + (4 * 12) + groupIndex];
		}
#endif
	} else {
#ifdef USE_BIT64
		int64 card = myCards;
		int64 group = 0x0000008004002001ULL;

		for(i = 0; i < 13; i++) {
			int64 pr = (card & group);
			if(pr) {
				int count = (int)(((pr * group) >> 39) & 7);
				res += param[handsSize * rev + (4 * i + count - 1) + groupIndex];
			}
			card = card >> 1;
		}
#else
		// 手札のGroupを調べる．強さ3〜2の13通り
		// 強さiのカードの枚数を数える(bitCount)
		unsigned int cardLow = (unsigned int)myCards, cardHi = (unsigned int)(myCards >> 32);
		unsigned int groupLow = 0x04002001, groupHi = 0x00000080;

		unsigned int pr = (cardLow & groupLow);
		int count = ((cardHi & groupHi) != 0);

		if(count | pr) {
			count += ((pr * groupLow) >> 26);
			res += param[handsSize * rev + (4 * 0 + count - 1) + groupIndex];
		}
		// groupをチェック
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0);
		// groupをチェック
		if(count | pr) {
			count += ((pr * groupLow) >> 26);
			res += param[handsSize * rev + (4 * 1 + count - 1) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0);
		// groupをチェック
		if(count | pr) {
			count += ((pr * groupLow) >> 26);
			res += param[handsSize * rev + (4 * 2 + count - 1) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0);
		// groupをチェック
		if(count | pr) {
			count += ((pr * groupLow) >> 26);
			res += param[handsSize * rev + (4 * 3 + count - 1) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0);
		// groupをチェック
		if(count | pr) {
			count += ((pr * groupLow) >> 26);
			res += param[handsSize * rev + (4 * 4 + count - 1) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0);
		// groupをチェック
		if(count | pr) {
			count += ((pr * groupLow) >> 26);
			res += param[handsSize * rev + (4 * 5 + count - 1) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0);
		// groupをチェック
		if(count | pr) {
			count += ((pr * groupLow) >> 26);
			res += param[handsSize * rev + (4 * 6 + count - 1) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0);
		// groupをチェック
		if(count | pr) {
			count += ((pr * groupLow) >> 26);
			res += param[handsSize * rev + (4 * 7 + count - 1) + groupIndex];
		}
		cardLow = (cardLow >> 1);
		cardHi = cardHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0);
		// groupをチェック
		if(count | pr) {
			count += ((pr * groupLow) >> 26);
			res += param[handsSize * rev + (4 * 8 + count - 1) + groupIndex];
		}
		cardLow = (cardLow >> 1);
		cardHi = cardHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0);
		// groupをチェック
		if(count | pr) {
			count += ((pr * groupLow) >> 26);
			res += param[handsSize * rev + (4 * 9 + count - 1) + groupIndex];
		}
		cardLow = (cardLow >> 1);
		cardHi = cardHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0);
		// groupをチェック
		if(count | pr) {
			count += ((pr * groupLow) >> 26);
			res += param[handsSize * rev + (4 * 10 + count - 1) + groupIndex];
		}
		cardLow = (cardLow >> 1);
		cardHi = cardHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0);
		// groupをチェック
		if(count | pr) {
			count += ((pr * groupLow) >> 26);
			res += param[handsSize * rev + (4 * 11 + count - 1) + groupIndex];
		}
		cardLow = (cardLow >> 1) | (cardHi << (32 - 1));
		cardHi = cardHi >> 1;
		// 強さiのカードの枚数を数える(bitCount)
		pr = (cardLow & groupLow);
		count = ((cardHi & groupHi) != 0);
		// groupをチェック
		if(count | pr) {
			count += ((pr * groupLow) >> 26);
			res += param[handsSize * rev + (4 * 12 + count - 1) + groupIndex];
		}
#endif
	}

	// JOKERの検出.
	if(myCards & (1LL << 52)) res += param[handsSize * rev + jokerIndex];
	return res;
}

// 次の状態を生成し，試合終了フラグを返す．
int getNextState(int result[5], fieldInfo* simField, int* current, int64 playersCard[5],
				 bitValidHandsArray vhcG[5], bitValidHandsArray vhcS[5],
				 const bitValidHand* vhb, int targetPlayer) {
	int j;
	// パスする場合の処理
	if(vhb->hands == 0ULL) {
		simulatePass(simField, current);
	}
	// カードを出す場合の処理
	else {
		// 現在のプレイヤーが上がる場合の処理
		if(simField->lest[*current] == vhb->qty) {
			if(targetPlayer == -1) {
				int count = 0;
				for(j = 0; j < 5; j++)
					count += ((simField->goal >> j) & 1);
				// rank = 5 - 既に上がった人数
				result[*current] = 5 - count;
				// 残り人数1人なら試合終了(あがり済み3人+今あがった1人＋残る1人)
				if(count == 3) {
					for(j = 0; j < 5; j++) {
						if(((simField->goal) & (1 << j)) || j == *current) continue;
						result[j] = 1;
						return 1;
					}
				} else {
					simField->goal |= (1 << *current);
				}
			} else {
				// 自分が上がったら順位をセットして終了
				if(*current == targetPlayer) {
					int count = 0;
					for(j = 0; j < 5; j++)
						count += ((simField->goal >> j) & 1);
					// rank = 5 - 既に上がった人数
					result[targetPlayer] = 5 - count;
					return 1;
				}
				// 上がったのが自分でなければ上がりフラグを更新
				else {
					simField->goal |= (1 << *current);
					// 自分以外みんなが上がってしまったら終了
					if((simField->goal | (1 << targetPlayer)) == (1 << 5) - 1) {
						result[targetPlayer] = 1;
						return 1;
					}
				}
			}
		}
		// カードを提出し，フィールドの情報を更新する
		removeHands(&vhcG[*current], &vhcS[*current], vhb->hands, playersCard[*current]);
		simulateSubmit(vhb, simField, playersCard, current);
	}

	while(((simField->pass >> *current) & 1) == 1 || simField->lest[*current] == 0) {
		if(simField->lest[*current] == 0) {
			simulatePass(simField, current);
			continue;
		}
		*current = (*current + 1) % 5;
	}

	return 0;
}

int getActionIndex(const bitValidHandsArray* vhb, const bitValidHandsArray* vhS, int64 myCard, int curRev, int lest) {
	int j;

	// あがれる手が存在すればそれを採用する
	for(j = vhb->size - 1; j >= 0; j--)
		if(vhb->hands[j].qty == lest) return j;

	// 行動が1択ならその行動をする
	if(vhb->size == 1) return 0;

	// 方策に従って確率的に行動を決定する

	double sumProb = 0.0;
	double prob[128];
	bitValidHandsArray bvha;
	bvha.size = 0;

	for(j = 0; j < vhS->size; j++) {
		if(vhS->hands[j].hands & (1LL << 52)) continue;
		bvha.hands[bvha.size++] = vhS->hands[j];
	}

	for(j = 0; j < vhb->size; j++) {
		int rev = curRev;
		if(vhb->hands[j].qty > 3 + vhb->hands[j].seq) rev = !rev;

		prob[j] = exp(calcValueUnrollVH(myCard ^ vhb->hands[j].hands, rev, &bvha));

		sumProb += prob[j];
	}

	double thr = sumProb * genrand_real2();
	double cursum = 0.0;

	for(j = 0; j < vhb->size - 1; j++) {
		cursum += prob[j];
		if(thr <= cursum) return j;
	}

	return vhb->size - 1;
}

void doSimulation(int result[5],
				  const int64 cards[5], const fieldInfo* finfo, int currentPlayer,
				  const bitValidHandsArray vhG[5],
				  const bitValidHandsArray vhS[5],
				  int targetPlayer) {
	int i, j;
	int64 playersCard[5];
	bitValidHandsArray vhb, vhcG[5], vhcS[5];
	fieldInfo simField = *finfo;
	int current = currentPlayer;
	double prob[128];

	for(i = 0; i < 5; i++) {
		result[i] = -1;
		playersCard[i] = cards[i];
		vhcG[i] = vhG[i];
		vhcS[i] = vhS[i];
	}

	int turnCount = 0;

	while(1) {

		// 出せるカードを全列挙する
		getAllValidHands(&vhb, &vhcG[current], &vhcS[current], &simField, playersCard[current]);

		// 場にカードがある場合はパスを考慮
		if(simField.onset == 0) {
			vhb.hands[vhb.size].hands = 0ULL;
			vhb.hands[vhb.size].qty = 0;
			vhb.hands[vhb.size++].seq = 0;
		}

		// 候補手のうちから確率的に行動を選択する
		int useIdx = getActionIndex(&vhb, &vhcS[current], playersCard[current], simField.rev, simField.lest[current]);

		// 次の状態を得る
		// 終了フラグが返されたらシミュレーション終わり
		if(getNextState(result, &simField, &current, playersCard, vhcG, vhcS, &vhb.hands[useIdx], targetPlayer))
			return;
	}
}

int nodeCount;

int lastSearch(int64 myCards, int64 oppCards, int current, const fieldInfo* finfo, int many, int depth) {
#ifdef USE_MAX_SEARCH_NODE
	if(nodeCount > MAX_SEARCH_NODE) return 0;
#endif
	nodeCount++;
	int i;
	// StackOverFlow回避．メモリ節約してこれ以上探索しても結果はほとんど変わらない．
	if(depth >= 30) return 0;
	bitValidHandsArray vha, vhG, vhS, vhcG[5], vhcS[5];
	if(finfo->mypos == current) {
		// 自分の出しうるカードを列挙
		generateAllHands(&vhG, &vhS, myCards);
		getAllValidHands(&vha, &vhG, &vhS, finfo, myCards);
	} else {
		if(many == 1) {
			// 相手に提出可能なカードがあれば負けと判断
			// 場に出ているカードの枚数が手札の枚数より多かったら提出できない
			if((finfo->onset == 1 || finfo->qty <= finfo->lest[current]) && (checkAllValidHands(finfo, oppCards) == 1))
				return 0;
			vha.size = 0;
		} else {
			// 相手の出しうるカードを列挙
			// 相手に対しては持ちうるカードすべてを考慮する
			generateAllHands(&vhG, &vhS, oppCards);
			getAllValidHands(&vha, &vhG, &vhS, finfo, oppCards);
		}
	}
	for(i = 0; i < vha.size; i++) {
		// あがれる手が存在するかチェック
		if(vha.hands[i].qty == finfo->lest[current]) {
			if(finfo->mypos == current)
				return 1; // 勝ち
			else
				return 0; // 負け
		}
	}
	if(finfo->onset == 0) {
		vha.hands[vha.size].hands = 0ULL;
		vha.hands[vha.size].qty = 0;
		vha.hands[vha.size].ord = 0;
		vha.hands[vha.size++].seq = 0;
	}
	int resultRank[5];
	int64 playersCard[5];
	for(i = 0; i < vha.size; i++) {
		// 手持ち枚数より枚数が多いものは提出できない(相手プレイヤ向け制約)
		if(vha.hands[i].qty > finfo->lest[current]) continue;
		int curPlayer = current;
		if(finfo->mypos == current) {
			playersCard[current] = myCards;
		} else {
			playersCard[current] = oppCards;
		}
		vhcG[current] = vhG;
		vhcS[current] = vhS;
		fieldInfo simField = (*finfo);
		getNextState(resultRank, &simField, &curPlayer, playersCard, vhcG, vhcS, &vha.hands[i], -1);
		if(finfo->mypos == current) {
			// 子ノードに必勝手が1個でもあれば必勝
			if(lastSearch(playersCard[current], oppCards, curPlayer, &simField, many, depth + 1) == 1) return 1;
		} else {
			// 負けになる手が1個でもあれば負け
			if(lastSearch(myCards, playersCard[current], curPlayer, &simField, many, depth + 1) == 0) return 0;
		}
	}
	// 自分：子ノードに勝てる手がない→負け
	// 相手：子ノードが必勝手のみ→勝ち
	return finfo->mypos != current;
}

// 必勝手の探索
int lastSearchTop(int64 myCards, int64 oppCards, const fieldInfo* finfo) {
	nodeCount = 0;
	int i;
	int many = (bitCount(oppCards) > 15); // 相手全員の残りカード合計が15枚以上なら相手のカード提出を許さない
	bitValidHandsArray vha, vhG, vhS, vhcG[5], vhcS[5];
	generateAllHands(&vhG, &vhS, myCards);
	getAllValidHands(&vha, &vhG, &vhS, finfo, myCards);
	int resultRank[5];
	int64 playersCard[5];
	if(finfo->onset == 0) {
		vha.hands[vha.size].hands = 0ULL;
		vha.hands[vha.size].qty = 0;
		vha.hands[vha.size].ord = 0;
		vha.hands[vha.size++].seq = 0;
	}
	for(i = 0; i < vha.size; i++) {
		int current = finfo->mypos;
		playersCard[current] = myCards;
		vhcG[current] = vhG;
		vhcS[current] = vhS;
		fieldInfo simField = (*finfo);
		getNextState(resultRank, &simField, &current, playersCard, vhcG, vhcS, &vha.hands[i], -1);
		if(lastSearch(playersCard[finfo->mypos], oppCards, current, &simField, many, 1) == 1) return i;
	}
	return -1;
}

// 現在の状況からモンテカルロ法で行動を決定する
// モンテカルロ法と言いつつ，事前に必勝手探索なんかも行なっている
void monteCarloSearch(int out_cards[8][15], int64 myCards, int64 oppCards, const changeInfo* cinfo, const fieldInfo* finfo) {
	int i, j;
	// 各プレイヤーの手札
	int64 playersCard[5];
	// 各手のシミュレーション回数
	int simulateCount[MAX_ARRAY_SIZE] = {0};
	// 各手のシミュレーションスコアの和
	double simulateScore[MAX_ARRAY_SIZE] = {0};
	// 各手のシミュレーションスコアの二乗和
	double simulateDScore[MAX_ARRAY_SIZE] = {0};
	// 可能な手を格納する構造体
	bitValidHandsArray vha, vhb, vhcG[5], vhcS[5], vhdG, vhdS;

	// 初手で取り得る行動をすべて列挙する
	generateAllHands(&vhdG, &vhdS, myCards);
	getAllValidHands(&vha, &vhdG, &vhdS, finfo, myCards);

	// 上がれる手が存在する場合はそれを採用する
	int decFlag = -1;
	for(i = 0; i < vha.size; i++) {
		if(vha.hands[i].qty == finfo->lest[finfo->mypos]) {
			decFlag = i;
		}
	}

	if(finfo->onset == 0) {
		vha.hands[vha.size].hands = 0ULL;
		vha.hands[vha.size].qty = 0;
		vha.hands[vha.size++].seq = 0;
	}

	// 行動が1通りしかなければそれを選ぶ
	if(vha.size == 1) decFlag = 0;

	// 必勝手が存在すればそれを選ぶ
	if(decFlag == -1)
		decFlag = lastSearchTop(myCards, oppCards, finfo);

	double score[MAX_ARRAY_SIZE];

	// i==vha.size の場合はパスとして考える
	if(decFlag == -1) {
		for(i = 0; i < SIMULATION_COUNT; i++) {
			for(j = 0; j < vha.size; j++) {
				// UCB1-TUNED により次にシミュレートする手を選ぶ
				if(simulateCount[j] == 0)
					score[j] = 1e12;
				else {
					double avg = simulateScore[j] / simulateCount[j];
					double avg2 = simulateDScore[j] / simulateCount[j];
					double V = avg2 - (avg * avg) + sqrt((2 * log((double)i)) / simulateCount[j]);
					if(V > 0.25) V = 0.25;
					score[j] = avg + sqrt(V * log((double)i) / simulateCount[j]);
				}
			}
			int idx = 0;
			for(j = 0; j < vha.size; j++) {
				if(score[idx] < score[j]) idx = j;
			}

			fieldInfo simField = *finfo;

			int current = finfo->mypos;

// カードを配分する．同時にジョーカー保持フラグも生成する．
#ifdef USE_ESTIMATE_HAND
			estimatedRandomCardDevide(playersCard, myCards, oppCards, cinfo, finfo);
#else
			randomCardDevide(playersCard, myCards, oppCards, cinfo, finfo);
#endif

			for(j = 0; j < 5; j++) {
				if(j == finfo->mypos) {
					int k;
					for(k = 0; k < vhdG.size; k++)
						vhcG[j].hands[k] = vhdG.hands[k];
					vhcG[j].size = vhdG.size;
					for(k = 0; k < vhdS.size; k++)
						vhcS[j].hands[k] = vhdS.hands[k];
					vhcS[j].size = vhdS.size;
				} else {
					if(finfo->lest[j] > 2)
						generateAllHands(&vhcG[j], &vhcS[j], playersCard[j]);
					else {
						vhcG[j].size = vhcS[j].size = 0;
						if(finfo->lest[j] > 0)
							generateGroup(&vhcG[j], playersCard[j]);
					}
				}
			}

			int resultRank[5];

			// プレイヤーの初手を処理する
			// 初手であがる手があればこのループに入ってこないので返り値は常にfalse
			getNextState(resultRank, &simField, &current, playersCard, vhcG, vhcS, &vha.hands[idx], finfo->mypos);

			doSimulation(resultRank, playersCard, &simField, current, vhcG, vhcS, finfo->mypos);

			int myRank = resultRank[finfo->mypos];

			// シミュレートの結果を更新
			simulateCount[idx]++;
			simulateScore[idx] += (myRank * myRank) / 25.0;
			simulateDScore[idx] += (myRank * myRank * myRank * myRank) / (25.0 * 25.0);
		}
	}

	int idx = 0;
	double maxScore = 0.0;

	// スコア最大の手を調べる
	if(decFlag == -1) {
		for(i = 0; i < vha.size + 1; i++) {
			double tmp = 0.0;
			// シミュレーション回数0の手のスコアは0として考える
			if(simulateCount[i] != 0)
				tmp = (double)simulateScore[i] / simulateCount[i];
			if(maxScore <= tmp) {
				maxScore = tmp;
				idx = i;
			}
		}
	} else {
		// シミュレーションによらず手が決まっていればそれを使う
		idx = decFlag;
	}

	if(vha.hands[idx].hands == 0LL) {
		// パスするとき
		for(i = 0; i < 8; i++) {
			for(j = 0; j < 15; j++)
				out_cards[i][j] = 0;
		}
	} else {
		// 提出するカードを配列に変換
		setSubmitCard(out_cards, &vha.hands[idx]);
	}
}
