/*connection*/
#ifndef __default__connection__
#define __default__connection__

void checkArg(int argc, char* argv[]);
int startGame(int table[8][15], int idx);
int entryToGame(int idx);
void sendChangingCards(int cards[8][15], int idx);
int receiveCards(int cards[8][15], int idx);
int sendCards(int cards[8][15], int idx);
void lookField(int cards[8][15], int idx);
int beGameEnd(int idx);
int closeSocket(int idx);

#endif
