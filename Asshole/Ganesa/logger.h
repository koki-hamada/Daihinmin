#ifndef LOGGER_H_
#define LOGGER_H_

#include "def.h"
#include "snowl/bitCard.h"
#include "snowl/mydef.h"

void logTable(int table[8][15]);
void logCard(int table[8][15]);
void logCard2(int table[8][15]);
void PrintBitCard(int64 bitCard);
void PrintBitValidHandsArray(bitValidHandsArray* vha);
void PrintFieldInfo(const fieldInfo* finfo);
void PrintState(const State& s);
#endif
