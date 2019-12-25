/*handGenerator*/ 
#ifndef __fumiya__handGenerator__
#define __fumiya__handGenerator__

#include "mydef.h"

// vhaを使って合法手を計算し，resに結果を格納する
void getAllValidHands(bitValidHandsArray *res, const bitValidHandsArray*, const bitValidHandsArray*, const fieldInfo *info, int64 myCards);

// 提出カードの情報から，合法手候補の集合を更新する
void removeHands(bitValidHandsArray*, bitValidHandsArray*, int64 submit, int64 myCards);

// myCards中の合法手を全て生成する
// - jokerを含む手はjoker単体出しとsequenceのみを生成
void generateAllHands(bitValidHandsArray*, bitValidHandsArray*, int64 myCards);

void generateGroup(bitValidHandsArray *vha, int64 myCards);

int checkAllValidHands(const fieldInfo *info, int64 myCards);

#endif
