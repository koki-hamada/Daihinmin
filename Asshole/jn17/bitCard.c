/* bitCard.c : ビット表現したカード集合に対する操作を定義 */ 
/* Author    : Fumiya Suto                                */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "bitCard.h"
#include "mydef.h"

// 与えられたビット配列中の1ビットの数を返す
int bitCount(int64 cards){
	int64 t;
	int res = 0;
	for(t=cards;t;t&=(t-1)) res++;
	return res;
}

// 配列表現されているカード集合をビット表現に変換する
int64 setBit(int cards[8][15]){
	int i, j, flag = 0;
	int64 result = 0;
	int64 jokerBit = (1LL << 52);
	for(i=0;i<5;i++){
		for(j=0;j<15;j++){
			if(cards[i][j] == 1){
				result |= (1LL << (13*i+(j-1)));
			}
			else if(cards[i][j] == 2){
				result |= jokerBit;
			}
		}
	}

	return result;
}

void bitToArray(int cards[8][15], int64 bitCards){
	int i, j;
	for(i=0;i<5;i++){
		for(j=0;j<15;j++){
			cards[i][j] = 0;
		}
	}
	for(i=0;i<4;i++){
		for(j=0;j<13;j++){
			if(((bitCards>>(i*13+j))&1) == 1) cards[i][j+1] = 1;
		}
	}
	if((bitCards>>52)==1) cards[4][1] = 2;
}

// vhが表す手を配列submit上に格納する
void setSubmitCard(int submit[8][15], const bitValidHand *vh){
	int i, j;
	for(i=0;i<5;i++){
		for(j=0;j<15;j++){
			submit[i][j] = 0;
		}
	}
	if(vh->qty == 1){
		if(((vh->hands >> 52)&1) == 1){
			submit[0][1] = 2;
			return ;
		} else {
			for(i=0;i<4;i++) if(((vh->suit >> i)&1)==1) break;
			if(1<=vh->ord&&vh->ord<=13)
				submit[i][vh->ord] = 1;
			else
				submit[0][1] = 1;
		}
	} else {
		if(vh->seq == 0){
			if(vh->qty == 5){
				for(i=0;i<4;i++)
					submit[i][vh->ord] = 1;
				submit[i][vh->ord] = 2;
			}
			else {
				for(i=0;i<4;i++){
					if(((vh->suit >> i)&1)==1){
						if(((vh->hands >> (13*i+vh->ord-1))&1)==1) submit[i][vh->ord] = 1;
						else                                       submit[i][vh->ord] = 2;
					}
				}
			}
		} else {
			for(i=0;i<4;i++) if(((vh->suit >> i)&1)==1) break;
			for(j=0;j<vh->qty;j++){
				if(1<=vh->ord+j&&vh->ord+j<=13&&((vh->hands >> (13*i+vh->ord+j-1))&1)==1)
					submit[i][vh->ord+j] = 1;
				else
					submit[i][vh->ord+j] = 2;
			}
		}
	}
}

void pushValidHands(bitValidHandsArray *vha, int64 hands, int qty, int seq, int ord, int suit){

	vha->hands[vha->size].hands = hands;
	vha->hands[vha->size].qty   = qty;
	vha->hands[vha->size].seq   = seq;
	vha->hands[vha->size].ord   = ord;
	vha->hands[vha->size].suit  = suit;

	vha->size++;
}
