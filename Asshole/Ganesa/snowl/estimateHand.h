/*estimateHand*/ 
#ifndef __fumiya__estimateHand__
#define __fumiya__estimateHand__

#include "mydef.h"

void initEnemysProb();
void updateProbs(const fieldInfo *finfo, const changeInfo *cinfo, bitValidHand *vha, int64 oppCards, int player);

void estimatedRandomCardDevide(int64 bitPlayers[5], int64 myCards, int64 oppCards, const changeInfo *cinfo, const fieldInfo *finfo);

#endif
