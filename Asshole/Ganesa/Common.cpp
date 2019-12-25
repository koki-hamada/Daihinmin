#include "Common.h"
#include "snowl/bitCard.h"
#include "snowl/cardSelect.h"
#include "snowl/handGenerator.h"
#include <algorithm>
#include <cstdlib>

int CanSubmitOpp(const fieldInfo* finfo, int64 oppCards, int plNum) {
	int maxOppQty = 0; // パスしていない相手の中での所持枚数の最大値
	for(int i = 0; i < MAX_PLAYER_NUM; i++) {
		if(i == plNum || (finfo->pass & (1 << finfo->seat[i])))
			continue;
		maxOppQty = std::max(maxOppQty, finfo->lest[finfo->seat[i]]);
	}
	// 相手が手を出せるか判定
	return (maxOppQty >= finfo->qty && checkAllValidHands(finfo, oppCards));
}

int findNextSeat(const fieldInfo* finfo, int lastPlayer) {
	int nextSeat = (finfo->seat[lastPlayer] + 1) % MAX_PLAYER_NUM;
	// まだあがっていない人を探す
	while(finfo->goal >> nextSeat & 1) {
		nextSeat = (nextSeat + 1) % MAX_PLAYER_NUM;
	}
	return nextSeat;
}

int CountGoal(const fieldInfo* finfo) {
	return bitCount(finfo->goal);
}

int simulateSubmit2(const bitValidHand* vh, fieldInfo* finfo, int64* myCards, int64 oppCards, int plNum, int lastPlayer) {
	// 出したカードを手札から除く．
	*myCards ^= vh->hands;
	// カードの残り枚数を更新する
	finfo->lest[finfo->seat[plNum]] -= vh->qty;

	if(checkEight(vh) || checkSpade3(vh)) { // 8切り・スペ3切り
		// プレイヤーの状態をリセット．上がったプレイヤーはパス扱い
		finfo->pass = finfo->goal;
		finfo->onset = 1;
		// ロック解除
		finfo->lock = 0;
		// マークをリセット
		finfo->suit = 0;
		// 革命判定
		if(checkRev(vh))
			finfo->rev = !(finfo->rev);
		// カード枚数・強さ・階段フラグはonset=1のとき使わない
		return 1;
	}
	else if(vh->hands == 0LL) { // パス
		int reset = 0;

		int allOppPass = finfo->pass == (((1 << MAX_PLAYER_NUM) - 1) ^ (1 << finfo->seat[plNum]));
		// 最後に出したのが自分で、自分以外がパスしていたら場が流れる
		if(lastPlayer == plNum && allOppPass) {
			reset = 1;
		}
		else {
			// 最後に出した人があがっていたとき
			if(finfo->lest[finfo->seat[lastPlayer]] == 0) {
				int nextSeat = findNextSeat(finfo, lastPlayer);
				// 次の手番が自分 && (自分以外パスしてる || または相手が何も出せない)ときリセット
				if(nextSeat == finfo->mypos && (allOppPass || !CanSubmitOpp(finfo, oppCards, plNum))) {
					reset = 1;
				}
			}
		}

		if(reset) {
			// プレイヤーの状態をリセット．上がったプレイヤーはパス扱い
			finfo->pass = finfo->goal;
			finfo->onset = 1;
			// ロック解除
			finfo->lock = 0;
			// マークをリセット
			finfo->suit = 0;
			// assert(temp.pass == finfo->pass);
			return 1;
		}
		finfo->pass |= 1 << finfo->seat[plNum];
		// assert(temp.pass == finfo->pass);

		return 0;
	}
	else {
		// 場の状況を更新
		// カードを出したので onset = 0
		finfo->onset = 0;
		// 場に出ているカード枚数
		finfo->qty = vh->qty;
		// ロック判定
		if(finfo->suit == vh->suit)
			finfo->lock = 1;
		// 場に出ているマーク
		finfo->suit = vh->suit;
		// 場に出ているカードの強さ
		finfo->ord = vh->ord;
		// 場に出ているカードが階段かどうか
		finfo->seq = vh->seq;
		// 革命判定
		if(checkRev(vh)) {
			finfo->rev = !(finfo->rev);
		}
		if(CanSubmitOpp(finfo, oppCards, plNum)) {
			return 0;
		}
		// 相手が手を出せなかったら自分以外パスの状態
		finfo->pass = ((1 << MAX_PLAYER_NUM) - 1) ^ (1 << finfo->seat[plNum]);
		return 1;
	}
	printf("simulate error\n");
	exit(1);
	return -1;
}

bool SameHand(const bitValidHand* vh1, const bitValidHand* vh2) {
	if(vh1->hands == 0 || vh2->hands == 0) {
		return vh1->hands == vh2->hands;
	}
	return vh1->hands == vh2->hands &&
		   vh1->qty == vh2->qty &&
		   vh1->seq == vh2->seq &&
		   vh1->ord == vh2->ord &&
		   vh1->suit == vh2->suit;
}
