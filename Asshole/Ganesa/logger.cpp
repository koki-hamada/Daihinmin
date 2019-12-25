#include "logger.h"
#include "def.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

const int64 allBitCard = (1LL << 53) - 1;

void logTable(int table[8][15]) {
	/*
	 引数で渡されたカードテーブルを出力する
	 */
	int i, j;
	for(i = 0; i < 8; i++) {
		for(j = 0; j < 15; j++) {
			printf("%i ", table[i][j]);
		}
		printf("\n");
	}
}

void logCard(int table[8][15]) {
	// テーブル形式で出力
	int i, j;
	char nums[15] = {' ', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A', '2', ' '};
	char suits[4] = {'S', 'H', 'D', 'C'};
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 15; j++) {
			if(table[i][j] == 1) {
				printf("%c%c  ", suits[i], nums[j]);
			} else if(table[i][j] == 2) {
				printf("JKR ");
			} else {
				printf("    ");
			}
		}
		printf("\n");
	}
	if(table[4][1] == 2) {
		printf("JOKER");
	}
	printf("\n");
}

void logCard2(int table[8][15]) {
	// 1行で出力
	int i, j;
	char nums[15] = {' ', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A', '2', ' '};
	char suits[4] = {'S', 'H', 'D', 'C'};
	for(j = 0; j < 15; j++) { // 数字の小さい順に並べる
		for(i = 0; i < 4; i++) {
			if(table[i][j] == 1) {
				printf("%c%c  ", suits[i], nums[j]);
			} else if(table[i][j] == 2) {
				printf("JOKER ");
			}
		}
	}
	if(table[4][1] == 2) {
		printf("JOKER");
	}
	printf("\n");
}

void PrintBitCard(int64 bitCard) {
	int i, j;
	char nums[15] = {' ', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A', '2', ' '};
	char suits[4] = {'S', 'H', 'D', 'C'};
	printf("[ ");
	for(j = 0; j < 13; j++) { // 数字の小さい順に並べる
		for(i = 0; i < 4; i++) {
			if(bitCard & (1LL << (i * 13 + j))) {
				printf("%c%c ", suits[i], nums[j + 1]);
			}
		}
	}
	if(bitCard == 0LL) {
		printf("pass ");
	}
	if(bitCard & (1LL << 52)) {
		printf("JOKER ");
	}
	printf("]");
}

void PrintBitValidHandsArray(bitValidHandsArray* vha) {
	if(vha->size < 0 || vha->size >= MAX_ARRAY_SIZE) {
		printf("hand size = %d !!!!!!\n", vha->size);
		exit(1);
	}
	printf("[ ");
	for(int i = 0; i < vha->size; i++) {
		PrintBitCard(vha->hands[i].hands);
		printf(" ");
	}
	printf("]\n");
}

void PrintFieldInfo(const fieldInfo* finfo) {
	if(finfo->onset) {
		printf("onset 1, rev %d\n", finfo->rev);
	} else {
		printf("ord %d, qty %d, lock %d, rev %d\n", finfo->ord, finfo->qty, finfo->lock, finfo->rev);
		printf("suit ");
		char c[4] = {'S', 'H', 'D', 'C'};
		for(int i = 0; i < 4; i++) {
			if(finfo->suit >> i & 1)
				printf("%c", c[i]);
		}
		printf("\n");
	}

	printf("pass ");
	for(int i = 0; i < MAX_PLAYER_NUM; i++) {
		//if (finfo->pass & (1 << finfo->seat[i])) printf("%d ", i);
		printf("%d ", (finfo->pass >> finfo->seat[i]) & 1);
	}
	printf("\n");
	printf("rest ");
	for(int i = 0; i < MAX_PLAYER_NUM; i++) {
		printf("%d ", finfo->lest[finfo->seat[i]]);
	}
	printf("\n");
	printf("goal ");
	for(int i = 0; i < MAX_PLAYER_NUM; i++) {
		//if (finfo->goal & (1 << finfo->seat[i])) printf("%d ", i);
		printf("%d ", (finfo->goal >> finfo->seat[i]) & 1);
	}

	printf("\n");
}

void PrintState(const State& s) {
	printf("myCards\n");
	PrintBitCard(s.myCards);
	printf("\n");
	printf("oppCards\n");
	PrintBitCard(s.oppCards);
	printf("\n");
	PrintFieldInfo(&s.finfo);
	printf("playerNum %d\n", s.playerNum);
	printf("lastPlayer %d\n", s.lastPlayer);
	printf("changePlayerNum %d\n", s.changePlayerNum);
	printf("having\n");
	PrintBitCard(s.havingChangeCards);
	printf("\n");
	printf("baCards\n");
	for(int i = 0; i < 8; i++) {
		for(int j = 0; j < 15; j++) {
			printf("%d ", s.baCards[i][j]);
		}
		printf("\n");
	}
	printf("submit\n");
	for(int i = 0; i < 5; i++) {
		PrintBitCard(s.submit[i]);
		printf("\n");
	}
	printf("\n");
}