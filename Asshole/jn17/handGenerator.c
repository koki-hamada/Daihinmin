/* handGenerator.c : 可能な行動の列挙や存在判定を行なう */ 
/* Author          : Fumiya Suto                        */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "bitCard.h"
#include "mydef.h"

int fewBitCount(int64 t){
	int res;
	for(res=0;t;t&=t-1) res++;
	return res;
}

// 場が空の場合における1枚以上のグループからなる手を生成
void getAllValidGroup(bitValidHandsArray *res, const bitValidHandsArray *vha, const fieldInfo *info, int64 myCards){
	int i, j;
	int num[15];
	memset(num, -1, sizeof(num));
	// soloまたはgroupを構成するものをコピー
	for(i=0;i<vha->size;i++){
		res->hands[res->size] = vha->hands[i];
		res->size++;
		num[vha->hands[i].ord] = i;
	}
	// 各強さのグループに対し，jokerを加えたものを生成する
	if(myCards&(1LL<<52)){
		for(i=1;i<=13;i++){
			if(num[i]==-1) continue;
			res->hands[res->size] = vha->hands[num[i]];
			res->hands[res->size].hands |= (1LL<<52);
			res->hands[res->size].qty++;
			// マークが使われてない最初の場所にビットを立てる
			res->hands[res->size].suit |= ((vha->hands[num[i]].suit+1)&(~vha->hands[num[i]].suit));
			res->size++;
		}
		// jokerの単体出しを追加
		// 強さは通常時14，革命時0扱い
		pushValidHands(res, (1LL<<52), 1, 0, 14*(1-info->rev), 0);
	}
}

// 場が空の場合における階段からなる手を生成
void getAllValidSequence(bitValidHandsArray *res, const bitValidHandsArray *vha, const fieldInfo *info){
	int i;
	for(i=0;i<vha->size;i++){
		// 場が空の場合はjokerを3-2の範囲外で使う手を除く
		//  - 同等な効果を持つ手が必ず他に存在する
		if(1<=vha->hands[i].ord&&vha->hands[i].ord+vha->hands[i].qty-1<=13){
			res->hands[res->size] = vha->hands[i];
			res->size++;
		}
	}
}

void getAllFollowSolo(bitValidHandsArray *res, const bitValidHandsArray *vha, const fieldInfo *info, int64 myCards){
	int i;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	for(i=vha->size-1;i>=0;i--){
		if(vha->hands[i].ord <= ord) break;
		// 縛り発生時のマーク判定
		if(lock == 1 && suit != vha->hands[i].suit) continue; 
		// 1枚かつ場のカードより強いカードなら追加
		if(vha->hands[i].qty == 1){
			res->hands[res->size] = vha->hands[i];
			res->size++;
		}
	}
	// ジョーカー探し
	if(ord < 14 && ((myCards>>52)&1) == 1){
		pushValidHands(res, (1LL<<52), 1, 0, 14, 0);
	}
	// スペードの3探し
	if(ord==14 && (myCards&1)==1){
		pushValidHands(res, 1LL, 1, 0, 14, 0);
	}
}

void getAllFollowSoloRev(bitValidHandsArray *res, const bitValidHandsArray *vha, const fieldInfo *info, int64 myCards){
	int i;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	for(i=0;i<vha->size;i++){
		if(vha->hands[i].ord >= ord) break;
		// 縛り発生時のマーク判定
		if(lock == 1 && suit != vha->hands[i].suit) continue; 
		// 1枚かつ場のカードより強いカードなら追加		
		if(vha->hands[i].qty == 1){
			res->hands[res->size] = vha->hands[i];
			res->size++;
		}
	}
	// ジョーカー探し
	if(ord > 0 && ((myCards>>52)&1) == 1){
		pushValidHands(res, (1LL<<52), 1, 0, 0, 0);
	}
	// スペードの3探し
	if(ord==0 && (myCards&1)==1){
		pushValidHands(res, 1LL, 1, 0, 0, 0);
	}
}

void getAllFollowGroup(bitValidHandsArray *res, const bitValidHandsArray *vha, const fieldInfo *info, int64 myCards){
	int i, j;
	int tmp;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	int qty  = info->qty;
	int64 validCard;
	int num[15], bsuit[15];
	memset(num, 0, sizeof(num));
	memset(bsuit, 0, sizeof(bsuit));
	// 各強さのカードについて枚数とマーク集合を求める
	for(i=0;i<vha->size;i++){
		bsuit[vha->hands[i].ord] |= vha->hands[i].suit;
		num[vha->hands[i].ord] = vha->hands[i].qty;
	}

	// 提出できる手を探す
	for(i=0;i<vha->size;i++){
		// 場のカードより強いカードでなければ出せない
		if(vha->hands[i].ord <= ord) continue;
		// 場のカードと同じ枚数の時
		if(vha->hands[i].qty == qty){
			// 縛り時のマーク判定
			if(lock == 1 && vha->hands[i].suit != suit) continue;
			res->hands[res->size] = vha->hands[i];
			res->size++;
		}
		// jokerを加える手の検討
		if((myCards&(1LL<<52)) && vha->hands[i].qty == qty-1){
			// jokerを加える手を追加済みならcontinue;
			if(num[vha->hands[i].ord] == 0) continue;
			// jokerを加えずに縛る手が存在すればcontinue;
			if((bsuit[vha->hands[i].ord]&suit) == suit) continue;
			// vha->hands[i]のカードでカバーされないマーク集合
			tmp = suit^(vha->hands[i].suit&suit);
			// jokerを加えて縛る事ができる
			if(fewBitCount(tmp) == 1){
				res->hands[res->size] = vha->hands[i];
				res->hands[res->size].hands |= (1LL<<52);
				res->hands[res->size].qty++;
				res->hands[res->size].suit = suit;
				res->size++;
				num[vha->hands[i].ord] = 0;
			}
			// jokerを加えないと強さvha->hands[i].ordの合法手が無いとき
			else if(num[vha->hands[i].ord] == qty-1) {
				// 縛り状態なら合法手を作れない
				if(lock == 1) continue;
				res->hands[res->size] = vha->hands[i];
				res->hands[res->size].hands |= (1LL<<52);
				res->hands[res->size].qty++;
				// 適当なところにマークを設定
				for(j=0; ;j++)
					if(!(res->hands[res->size].suit&(1<<j))){
						res->hands[res->size].suit |= (1<<j);
						break;
					}
				res->size++;
				num[vha->hands[i].ord] = 0;
			}
		}
	}
}

void getAllFollowGroupRev(bitValidHandsArray *res, const bitValidHandsArray *vha, const fieldInfo *info, int64 myCards){
	int i, j;
	int tmp;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	int qty  = info->qty;
	int64 validCard;
	int num[15], bsuit[15];
	memset(num, 0, sizeof(num));
	memset(bsuit, 0, sizeof(bsuit));
	// 各強さのカードについて枚数とマーク集合を求める
	for(i=0;i<vha->size;i++){
		bsuit[vha->hands[i].ord] |= vha->hands[i].suit;
		num[vha->hands[i].ord] = vha->hands[i].qty;
	}

	// 提出できる手を探す
	for(i=0;i<vha->size;i++){
		// 場のカードより弱いカードでなければ出せない
		//  - (To do)通常時用の関数とこの行しか違わないので後で統合する
		if(vha->hands[i].ord >= ord) continue;
		// 場のカードと同じ枚数の時
		if(vha->hands[i].qty == qty){
			// 縛り時のマーク判定
			if(lock == 1 && vha->hands[i].suit != suit) continue;
			res->hands[res->size] = vha->hands[i];
			res->size++;
		}
		// jokerを加える手の検討
		if((myCards&(1LL<<52)) && vha->hands[i].qty == qty-1){
			// jokerを加える手を追加済みならcontinue;
			if(num[vha->hands[i].ord] == 0) continue;
			// jokerを加えずに縛る手が存在すればcontinue;
			if((bsuit[vha->hands[i].ord]&suit) == suit) continue;
			// vha->hands[i]のカードでカバーされないマーク集合
			tmp = suit^(vha->hands[i].suit&suit);
			// jokerを加えて縛る事ができる
			if(fewBitCount(tmp) == 1){
				res->hands[res->size] = vha->hands[i];
				res->hands[res->size].hands |= (1LL<<52);
				res->hands[res->size].qty++;
				res->hands[res->size].suit = suit;
				res->size++;
				num[vha->hands[i].ord] = 0;
			}
			// jokerを加えないと強さvha->hands[i].ordの合法手が無いとき
			else if(num[vha->hands[i].ord] == qty-1) {
				// 縛り状態なら合法手を作れない
				if(lock == 1) continue;
				res->hands[res->size] = vha->hands[i];
				res->hands[res->size].hands |= (1LL<<52);
				res->hands[res->size].qty++;
				// 適当なところにマークを設定
				for(j=0; ;j++)
					if(!(res->hands[res->size].suit&(1<<j))){
						res->hands[res->size].suit |= (1<<j);
						break;
					}
				res->size++;
				num[vha->hands[i].ord] = 0;
			}
		}
	}
}

void getAllFollowSequence(bitValidHandsArray *res, const bitValidHandsArray *vha, const fieldInfo *info, int64 myCards){
	int i;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	int qty  = info->qty;
	int64 mask = 0x0000008004002001ULL;
	for(i=0;i<vha->size;i++){
		// 縛り発生時はマーク判定をする
		if(lock==1 && vha->hands[i].suit != suit) continue;
		// 枚数チェック
		if(vha->hands[i].qty != qty) continue;
		// 強さチェック ord+qty以上である必要がある
		if(vha->hands[i].ord < ord+qty) continue;
		// 先頭のカードがjokerなら使わない
		if(!((mask << (vha->hands[i].ord-1))&myCards)) continue;
		// カードを追加
		res->hands[res->size] = vha->hands[i];
		res->size++;
	}
}

void getAllFollowSequenceRev(bitValidHandsArray *res, const bitValidHandsArray *vha, const fieldInfo *info, int64 myCards){
	int i;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	int qty  = info->qty;
	int64 mask = 0x0000008004002001ULL;
	for(i=0;i<vha->size;i++){
		// 縛り発生時はマーク判定をする
		if(lock==1 && vha->hands[i].suit != suit) continue;
		// 枚数チェック
		if(vha->hands[i].qty != qty) continue;
		// 強さチェック ord-qty以下である必要がある
		//  - 通常時用とここしか違わないので(ry
		if(vha->hands[i].ord > ord-qty) continue;
		// 最後のカードがjokerなら使わない
		if(!((mask << (vha->hands[i].ord+qty-2))&myCards)) continue;
		// カードを追加
		res->hands[res->size] = vha->hands[i];
		res->size++;
	}
}

void getAllValidHands(bitValidHandsArray *res, const bitValidHandsArray *group, const bitValidHandsArray *seq, const fieldInfo *info, int64 myCards){
	int i;
	res->size = 0;
	if(info->onset == 1){
		getAllValidGroup(res, group, info, myCards);
		getAllValidSequence(res, seq, info);
	} else {
		if(info->rev == 0){
			if(info->qty == 1){
				getAllFollowSolo(res, group, info, myCards);
			}
			else {
				if(info->seq == 0){
					getAllFollowGroup(res, group, info, myCards);
				} else {
					getAllFollowSequence(res, seq, info, myCards);
				}
			}
		} else {
			if(info->qty == 1){
				getAllFollowSoloRev(res, group, info, myCards);
			}
			else {
				if(info->seq == 0){
					getAllFollowGroupRev(res, group, info, myCards);
				} else {
					getAllFollowSequenceRev(res, seq, info, myCards);
				}
			}
		}
	}
}

// 提出カードの情報から，合法手の集合を更新する
void removeHands(bitValidHandsArray *group, bitValidHandsArray *seq, int64 submit, int64 myCards){
	int i;
	int nsize = 0;
	int64 mv;
	// 手札集合から提出カードを除き，ジョーカーが残るか調べる
	int joker = (int)((myCards^(myCards&submit)) >> 52);
	for(i=0;i<group->size;i++){
		// i番目の手に提出カードの影響が無い
		if((group->hands[i].hands&submit) == 0LL){
			if(nsize != i)
				group->hands[nsize] = group->hands[i];
			nsize++;
		}
	}

	// groupのサイズを更新
	group->size = nsize;

	nsize = 0;

	for(i=0;i<seq->size;i++){
		// i番目の手に提出カードの影響が無い
		if((seq->hands[i].hands&submit) == 0LL){
			if(nsize != i)
				seq->hands[nsize] = seq->hands[i];
			nsize++;
		}
		// 影響があるとき…階段ならjokerを使って維持できるかを調べる
		else {
			if(joker == 1){
				// 既にjokerを使っている手なら維持できない
				if(seq->hands[i].hands&(1LL<<52)) continue;
				// 提出カードで除かれるカードが1枚だけならjokerで置き換えて維持する
				mv = seq->hands[i].hands&submit;
				if(fewBitCount(mv)==1){
					seq->hands[i].hands ^= mv;
					seq->hands[i].hands |= (1LL<<52);
					seq->hands[nsize] = seq->hands[i];
					nsize++;
				}
			}
		}
	}
	// seqのサイズを更新
	seq->size = nsize;
}

// 手札から1枚以上の組を生成する
void generateGroup(bitValidHandsArray *vha, int64 myCards){
	int i;
	int maskA, maskB, maskC, maskD;
	int64 cardA, cardB, cardC, cardD;
	int cnt;
	int bitPos[4];
	// soloおよびgroupの生成
	for(i=1;i<=13;i++){
		cnt = 0;
		if((myCards >> (13*0+i-1))&1) bitPos[cnt++] = 0;
		if((myCards >> (13*1+i-1))&1) bitPos[cnt++] = 1;
		if((myCards >> (13*2+i-1))&1) bitPos[cnt++] = 2;
		if((myCards >> (13*3+i-1))&1) bitPos[cnt++] = 3;

		if(cnt > 0){
			maskA = (1 << bitPos[0]);
			cardA = (1LL << (13*bitPos[0]+i-1));
			pushValidHands(vha, cardA, 1, 0, i, maskA);
			if(cnt > 1){
				maskB = (1 << bitPos[1]);
				cardB = (1LL << (13*bitPos[1]+i-1));
				pushValidHands(vha,       cardB, 1, 0, i,       maskB);
				pushValidHands(vha, cardA|cardB, 2, 0, i, maskA|maskB);
				if(cnt > 2){
					maskC = (1 << bitPos[2]);
					cardC = (1LL << (13*bitPos[2]+i-1));
					pushValidHands(vha,             cardC, 1, 0, i,             maskC);
					pushValidHands(vha, cardA      |cardC, 2, 0, i, maskA      |maskC);
					pushValidHands(vha,       cardB|cardC, 2, 0, i,       maskB|maskC);
					pushValidHands(vha, cardA|cardB|cardC, 3, 0, i, maskA|maskB|maskC);
					if(cnt > 3){
						maskD = (1 << bitPos[3]);
						cardD = (1LL << (13*bitPos[3]+i-1));
						pushValidHands(vha,                   cardD, 1, 0, i,                   maskD);
						pushValidHands(vha, cardA            |cardD, 2, 0, i, maskA            |maskD);
						pushValidHands(vha,       cardB      |cardD, 2, 0, i,       maskB      |maskD);
						pushValidHands(vha, cardA|cardB      |cardD, 3, 0, i, maskA|maskB      |maskD);
						pushValidHands(vha,             cardC|cardD, 2, 0, i,             maskC|maskD);
						pushValidHands(vha, cardA      |cardC|cardD, 3, 0, i, maskA      |maskC|maskD);
						pushValidHands(vha,       cardB|cardC|cardD, 3, 0, i,       maskB|maskC|maskD);
						pushValidHands(vha, cardA|cardB|cardC|cardD, 4, 0, i, maskA|maskB|maskC|maskD);
					}
				}
			}
		}
	}
}

// 手札からjokerを含む3枚以上の階段を生成する
void generateSequence(bitValidHandsArray *vha, int64 myCards){
	int i, j, k;
	int joker = ((myCards >> 52)&1);
	int64 validCard;
	int count;
	for(i=0;i<4;i++){
		int hand = 2*(((int)(myCards >> (13*i)))&((1<<13)-1));
		// handに含まれるビット数が1個以下なら
		if((hand&(hand-1)) == 0) continue;
		for(j=0;j+3-1<15;j++){
			int mask = (hand&(7<<j));
			if((mask&(mask-1))==0) continue;
			count = 0;
			validCard = 0LL;
			for(k=0;j+k<15;k++){
				if(((hand >> (j+k))&1)==1){
					validCard |= (1LL << (13*i+j+k-1));
				} else {
					validCard |= (1LL << 52);
					count++;
				}
				if(count > joker) break;
				else if(k>=2) pushValidHands(vha, validCard, k+1, 1, j, (1<<i));
			}
		}
	}
}

// myCards中の合法手を全て生成する
// - jokerを含む手はjoker単体出しとsequenceのみを生成
void generateAllHands(bitValidHandsArray *group, bitValidHandsArray *seq, int64 myCards){
	// 初期化
	group->size = 0;
	seq->size   = 0;
	// soloおよびgroupの生成
	generateGroup(group, myCards);
	// sequenceの生成
	generateSequence(seq, myCards);
}

int checkFollowValidSolo(const fieldInfo *info, int64 myCards){
	int i, j;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	for(i=0;i<4;i++){
		if(lock == 1 && ((suit>>i)&1)==0) continue; 
		for(j=ord+1;j<14;j++){
			if(((myCards >> (13*i+j-1))&1)==1){
				return 1;
			}
		}
	}
	// ジョーカー探し
	if(ord < 14 && ((myCards>>52)&1) == 1){
		return 1;
	}
	// スペードの3探し
	if(ord==14 && (myCards&1)==1){
		return 1;
	}
	return 0;
}

int checkFollowValidGroup(const fieldInfo *info, int64 myCards){
	int i, j;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	int qty  = info->qty;
	int bitNum[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
	int joker = (int)((myCards >> 52)&1);
	for(i=ord+1;i<14;i++){
		int mask = 0;
		for(j=0;j<4;j++) mask |= (((myCards >> (13*j+i-1))&1) << j);
		// ジョーカー足しても枚数足りない
		if(bitNum[mask] + joker < qty) continue;
		// suitパターンでループ
		for(j=3;j<(1<<4);j++){
			// しばり発生時
			if(lock == 1 && j != suit) continue;
			// ビットパターンが枚数に一致しない
			if(bitNum[j] != qty) continue;
			// mask が提出ビットパターンを包含
			if((j&mask) == j){
				return 1;
			} 
			else if (joker == 1 && bitNum[j^(j&mask)] == 1){
				if(bitNum[suit^(mask&suit)] == 1&&j!=suit) continue;
				return 1;
			}
		}
	}
	return 0;
}

int checkFollowValidSequence(const fieldInfo *info, int64 myCards){ 
	int i, j, k;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	int qty  = info->qty;
	int joker = (int)((myCards >> 52)&1);
	int count;
	// マークに関するループ
	for(i=0;i<4;i++){
		// しばり発生時
		if(lock==1 && ((suit>>i)&1)==0) continue;
		// 強さjからqty枚のシーケンスを検索する
		for(j=ord+qty;j+qty-1<15;j++){
			count = (int)((myCards >> (13*i+j-1))&1);
			// Jokerを先頭にする組み合わせを除く
			if(count == 0) continue;
			for(k=1;k<qty;k++){
				if(j+k-1 >= 13) continue;
				count += (int)((myCards >> (13*i+j+k-1))&1);
			}
			// カードがqty枚並んでいる
			if(count+joker >= qty) return 1;
		}
	}
	return 0;
}

int checkFollowValidSoloRev(const fieldInfo *info, int64 myCards){
	int i, j;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	for(i=0;i<4;i++){
		if(lock == 1 && ((suit>>i)&1)==0) continue; 
		for(j=ord-1;j>0;j--){
			if(((myCards >> (13*i+j-1))&1)==1){
				return 1;
			}
		}
	}
	// ジョーカー探し
	if(ord > 0 && ((myCards>>52)&1) == 1) return 1;
	// スペードの3探し
	if(ord==0 && (myCards&1)==1) return 1;
	return 0;
}

int checkFollowValidGroupRev(const fieldInfo *info, int64 myCards){
	int i, j;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	int qty  = info->qty;
	int bitNum[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
	int joker = (int)((myCards >> 52)&1);
	for(i=ord-1;i>0;i--){
		int mask = 0;
		for(j=0;j<4;j++) mask |= (((myCards >> (13*j+i-1))&1) << j);
		// ジョーカー足しても枚数足りない
		if(bitNum[mask] + joker < qty) continue;
		for(j=3;j<(1<<4);j++){
			// しばり発生時
			if(lock == 1 && j != suit) continue;
			// ビットパターンが枚数に一致しない
			if(bitNum[j] != qty) continue;
			// mask が提出ビットパターンを包含
			if((j&mask) == j){
				return 1;
			} 
			else if (joker == 1 && bitNum[j^(j&mask)] == 1){
				if(bitNum[suit^(mask&suit)] == 1&&j!=suit) continue;
				return 1;
			}
		}
	}
	return 0;
}

int checkFollowValidSequenceRev(const fieldInfo *info, int64 myCards){
	int i, j, k;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	int qty  = info->qty;
	int joker = (int)((myCards >> 52)&1);
	int count;
	for(i=0;i<4;i++){
		if(lock==1 && ((suit>>i)&1)==0) continue;
		for(j=ord-qty;j>=0;j--){
			count = (int)((myCards >> (13*i+j+qty-1))&1);
			// 3-4-5のシーケンスでJokerを5代わりにする場合以外は，
			// Jokerを最後にする組み合わせを除く
			if(count == 0) continue;
			for(k=0;k<qty-1;k++){
				count += (int)((myCards >> (13*i+j+k-1))&1);
			}
			// カードがqty枚並んでいるか，jokerを足せばqty枚並べられる
			if(count+joker >= qty) return 1;
		}
	}
	return 0;
}

// 与えられた状況でmyCardsからパス以外にとれる行動があるかを判定する
int checkAllValidHands(const fieldInfo *info, int64 myCards){
	if(info->onset == 1){
		return (myCards != 0LL); // 場が空でカードを所持してれば何かしら出せる
	} else {
		if(info->rev == 0){
			if(info->qty == 1){
				return checkFollowValidSolo(info, myCards);
			}
			else {
				if(info->seq == 0){
					return checkFollowValidGroup(info, myCards);
				} else {
					return checkFollowValidSequence(info, myCards);
				}
			}
		} else {
			if(info->qty == 1){
				return checkFollowValidSoloRev(info, myCards);
			}
			else {
				if(info->seq == 0){
					return checkFollowValidGroupRev(info, myCards);
				} else {
					return checkFollowValidSequenceRev(info, myCards);
				}
			}
		}
	}
	return -1; // ここには来ない
}
