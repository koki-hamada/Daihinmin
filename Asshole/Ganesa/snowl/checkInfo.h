/*checkInfo*/ 
#ifndef __fumiya__checkInfo__
#define __fumiya__checkInfo__

#include "mydef.h"

void checkState(fieldInfo *info, int cards[8][15]);
void checkField(fieldInfo *info, const changeInfo *cinfo, int cards[8][15], int64 oppCards);

#endif
