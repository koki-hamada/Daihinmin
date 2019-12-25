/* checkInfo.c : ��̏��̓ǂݍ��݁E��͓����s���֐� */ 
/* Author      : Fumiya Suto                          */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>

//#include "estimateHand.h"
#include "bitCard.h"
#include "mydef.h"

void checkState(fieldInfo *info, int cards[8][15]){
	//���
	info->onset = (cards[5][4] > 0);
//	info->b11   = (cards[5][5] > 0);  // cards[5][5] : 11�o�b�N(�g���ĂȂ�)
	info->rev   = (cards[5][6] > 0);
	info->lock  = (cards[5][7] > 0);

	if(info->onset == 1){
		info->qty  = 0;
		info->ord  = 0;
		info->lock = 0;
		info->suit = 0;
		info->pass = 0;
	}
}

void checkField(fieldInfo *info, const changeInfo *cinfo, int cards[8][15], int64 oppCards){
	int i,j,count=0, joker=0;
	i=j=0;
  
#ifdef USE_ESTIMATE_HAND
	fieldInfo copy = (*info);
#endif

	//�J�[�h�̂���ʒu��T��
	while(j<15&&cards[i][j]==0){
		i++;
		if(i==4){
			j++;
			i=0;
		}
	}
	if(cards[i][j] == 2) joker = 1;
	//�K�i���ۂ�
	if(j<14){
		info->seq = (cards[i][j+1] > 0);
	}
	//�����𐔂��� �܂������𒲂ׂ�
	if(info->seq == 0){
		//�����g
		info->suit = 0;
		for(;i<5;i++){
			if(cards[i][j]>0){
				count++;
				info->suit |= (1<<i);
			}
		}
		if(j==0||j==14||(count==1&&joker==1)){
			if(info->rev == 0) info->ord = 14;
			else               info->ord = 0;
		}else{
			info->ord = j;
		}
	}else{
		//�K�i
		while(j+count<15 && cards[i][j+count]>0){
			count++;
		}
		// ���̒��ň�Ԏア�J�[�h�̋�������̋����Ƃ���
		info->ord = j;
		info->suit = (1<<i);
	}
	info->qty = count;
	if(info->qty > 0) info->onset = 0;

	// �p�X�E��������A�v���C���[�̎c��J�[�h�������X�V
	int cur = cards[5][3];
	if(cards[6][cur] == info->lest[info->seat[cur]]){
		info->pass |= (1 << info->seat[cur]);
	} else {
		info->lest[info->seat[cur]] = cards[6][cur];
		if(info->lest[info->seat[cur]] == 0)
			info->goal |= (1 << info->seat[cur]);
	}

#ifdef USE_ESTIMATE_HAND
	bitValidHand vh;
	if(cards[6][cur] == copy.lest[info->seat[cur]]){
		vh.hands = 0LL;
		vh.qty   = 0;
		vh.seq   = 0;
		vh.ord   = 0;
		vh.suit  = 0;
	} else {
		vh.hands = setBit(cards);
		vh.qty   = info->qty;
		vh.seq   = info->seq;
		vh.ord   = info->ord;
		vh.suit  = info->suit;
	}
	updateProbs(&copy, cinfo, &vh, oppCards, info->seat[cur]);
#endif

}
