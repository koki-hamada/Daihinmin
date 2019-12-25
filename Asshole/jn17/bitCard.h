/*bitCard*/ 
#ifndef __fumiya__bitCard__
#define __fumiya__bitCard__

#include "mydef.h"

int bitCount(int64 cards);

int64 setBit(int cards[8][15]);
void bitToArray(int cards[8][15], int64 bitCards);
void setSubmitCard(int submit[8][15], const bitValidHand *vh);

void pushValidHands(bitValidHandsArray *vha, int64 hands, int qty, int sequence, int ord, int suit);

#endif
