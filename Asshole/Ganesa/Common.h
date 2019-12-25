#ifndef COMMON_H_
#define COMMON_H_

#include "def.h"
#include <cmath>

bool nearlyEqual(float a, float b);

// 相手がカードを提出できるかチェック
int CanSubmitOpp(const fieldInfo* finfo, int64 oppCards, int plNum);
// 次のターンの人を調べる
int findNextSeat(const fieldInfo* finfo, int lastPlayer);
// あがった人数を数える
int CountGoal(const fieldInfo* finfo);
// カードの提出処理を行う
// 提出した手に対して相手が行動できる手がなかったら1を返す
int simulateSubmit2(const bitValidHand* vh, fieldInfo* finfo, int64* myCards, int64 oppCards, int plNum, int lastPlayer);

bool SameHand(const bitValidHand* vh1, const bitValidHand* vh2);

#endif