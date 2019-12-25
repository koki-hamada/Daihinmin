/*cardSelect*/

#ifndef __fumiya__cardSelect__
#define __fumiya__cardSelect__

#include "mydef.h"

void monteCarloSearch(int out_cards[8][15], int64 myCards, int64 oppCards, const changeInfo* cinfo, const fieldInfo* finfo);
int lastSearchTop(int64 myCards, int64 oppCards, const fieldInfo* finfo);
int checkEight(const bitValidHand* vh);
int checkRev(const bitValidHand* vh);
int checkSpade3(const bitValidHand* vh);
void simulatePass(fieldInfo* simField, int* current);
void simulateSubmit(const bitValidHand* vh, fieldInfo* simField, int64 playersCard[5], int* current);
int getNextState(int result[5], fieldInfo* simField, int* current, int64 playersCard[5], bitValidHandsArray vhcG[5], bitValidHandsArray vhcS[5],
				 const bitValidHand* vhb, int targetPlayer);
#endif
