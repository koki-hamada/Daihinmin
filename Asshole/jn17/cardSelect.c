/* cardSelect.c : �����e�J�����Œ�o����J�[�h�����肷�� */ 
/* Author       : Fumiya Suto                            */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <math.h>

#include "bitCard.h"
#include "handGenerator.h"
#include "mt19937ar.h"

// �]���֐��ōl��������
const int seq5Size =  9;  // 5���K�i  3�`J��9�ʂ�
const int seq4Size = 10;  // 4���K�i  3�`Q��10�ʂ�
const int seq3Size = 11;  // 3���K�i  3�`K��11�ʂ�
const int groupSize = 52; // 1�`4���g 3�`2��13�ʂ� �~ (1�`4)�� = 52�ʂ�
const int jokerSize = 1;  // joker��1��

// �����e�J�������̃V�~�����[�V�����Ɏg���p�����[�^
double param[166];

// �I���������8�؂肪�������邩���`�F�b�N����
int checkEight(const bitValidHand *vh){
  if(vh->seq == 0){
    return vh->ord == 6;
  } else {
    return vh->ord <=6 && 6 <= vh->ord + vh->qty -1; 
  }
}

// �I��������Ŋv�����������邩���`�F�b�N����
int checkRev(const bitValidHand *vh){
  // �y�A�Ȃ�4���ȏ�C�K�i�Ȃ�5���ȏ�Ŋv��
  return (4 + vh->seq) <= vh->qty;
}

// �����_���ɃJ�[�h�𕪔z����
void randomCardDevide(int64 playersCard[5], int64 myCards, int64 oppCards, const changeInfo *cinfo, const fieldInfo *finfo){
  int i, j;
  int pos, sum, count = 0;
  int card[5];
  for(i=0;i<5;i++){
    card[i] = finfo->lest[i];
    playersCard[i] = 0LL;
    if(i != finfo->mypos) count += finfo->lest[i];
  }
  playersCard[finfo->mypos] = myCards;
	
  int64 opp = oppCards;
  int64 chg = ((cinfo->chgCards)&oppCards);
  // �����ɏo�����J�[�h��������m�肳����
  if(chg != 0LL){
    sum = 0;
    for(i=0;i<53;i++) sum += ((chg >> i)&1);
    for(i=0;i<5;i++){
      if(finfo->rank[i]+finfo->rank[finfo->mypos]==4) break;
    }
    playersCard[i] |= chg;
    opp ^= chg;
    card[i] -= sum;
    count -= sum;
  }
#if 1
  // �ŏ��ɍs�������v���C���Ƀ_�C����3�𕪔z����
  if((opp&(1LL<<26)) && cinfo->firstPlayer != -1){
    playersCard[cinfo->firstPlayer] |= (1LL << 26);
    opp ^= (1LL << 26);
    card[cinfo->firstPlayer]--;
    count--;
  }
  if(cinfo->chgCards != 0LL){
    // JOKER�𕽖��ȏ�̐l�ɕ��z����
    if(((opp>>52)&1)==1LL){
      int cnt = 0;
      for(i=0;i<5;i++)
	if(finfo->rank[i]<3&&i!=finfo->mypos) cnt += card[i];
      sum = 0;
      pos = genrand_int32()%cnt;
      for(i=0;i<5;i++){
	if(finfo->rank[i] > 2 || i == finfo->mypos) continue;
	sum += card[i];
	if(pos < sum) break;
      }
      playersCard[i] |= (1LL << 52);
      card[i]--;
      count--;
      opp ^= (1LL << 52);
    }
    // �X�y�[�h��2��n���ȏ�̐l�ɕ��z����
    if(((opp>>12)&1)==1LL){
      int cnt = 0;
      for(i=0;i<5;i++)
	if(finfo->rank[i]<4&&i!=finfo->mypos) cnt += card[i];
      sum = 0;
      pos = genrand_int32()%cnt;
      for(i=0;i<5;i++){
	if(finfo->rank[i] == 4 || i == finfo->mypos) continue;
	sum += card[i];
	if(pos < sum) break;
      }
      playersCard[i] |= (1LL << 12);
      card[i]--;
      count--;
      opp ^= (1LL << 12);
    }
  }
  // �������x���ȏ�̏ꍇ�C�������肪�����Ă��Ȃ����Ƃ�������J�[�h�𕪔z����
  if((opp&(cinfo->notCards))!=0LL){
    for(i=0;i<52;i++){
      if((((opp&(cinfo->notCards))>>i)&1)==0LL) continue;
      int cnt = 0;
      for(j=0;j<5;j++)
	if(finfo->rank[j]+finfo->rank[finfo->mypos]!=4&&j!=finfo->mypos) cnt += card[j];
      sum = 0;
      pos = genrand_int32()%cnt;
      for(j=0;j<5;j++){
	if(finfo->rank[j]+finfo->rank[finfo->mypos]==4||j==finfo->mypos) continue;
	sum += card[j];
	if(pos < sum) break;
      }
      playersCard[j] |= (1LL << i);
      card[j]--;
      count--;
      opp ^= (1LL << i);
    }
  }
#endif
  // �c��̃J�[�h�������_���ɕ��z����
  for(i=0;i<53;i++){
    if(((opp>>i)&1)==1&&count!=0){
      pos = genrand_int32()%count;
      sum = 0;
      for(j=0;j<5;j++){
	if(j == finfo->mypos) continue;
	sum += card[j];
	if(pos < sum) break;
      }
      playersCard[j] |= (1LL << i);
      card[j]--;
      count--;
    }
  }
}

// �p�X�̏������s��
void simulatePass(fieldInfo *simField, int *current){
  simField->pass |= (1 << (*current));
  // �S�����p�X�������
  if(simField->pass == (1<<5)-1){
    // �v���C���[�̏�Ԃ����Z�b�g�D�オ�����v���C���[�̓p�X����
    simField->pass  = simField->goal;
    simField->onset = 1;
    // ���b�N����
    simField->lock  = 0;
    // �}�[�N�����Z�b�g
    simField->suit  = 0;
  } else {
    *current = (*current + 1)%5;
  }
}

// �J�[�h�̒�o�������s��
void simulateSubmit(const bitValidHand *vh, fieldInfo *simField, int64 playersCard[5], int *current){
  // �o�����J�[�h����D���珜���D
  playersCard[*current] ^= vh->hands;
  // �J�[�h�̎c�薇�����X�V����
  simField->lest[*current] -= vh->qty;
	
  if(checkEight(vh) == 1){
    // �v���C���[�̏�Ԃ����Z�b�g�D�オ�����v���C���[�̓p�X����
    simField->pass  = simField->goal;
    simField->onset = 1;
    // ���b�N����
    simField->lock = 0;
    // �}�[�N�����Z�b�g
    simField->suit = 0;
    // �v������
    if(checkRev(vh)) simField->rev = !(simField->rev);
    // �J�[�h�����E�����E�K�i�t���O��onset=1�̂Ƃ��g��Ȃ�
    // current�͂��̂܂�
  } else {						
    // ��̏󋵂��X�V
    // �J�[�h���o�����̂� onset = 0
    simField->onset = 0;
    // ��ɏo�Ă���J�[�h����
    simField->qty   = vh->qty;
    // ���b�N����
    if(simField->suit == vh->suit) simField->lock = 1;
    // ��ɏo�Ă���}�[�N
    simField->suit  = vh->suit;
    // ��ɏo�Ă���J�[�h�̋���
    simField->ord   = vh->ord;
    // ��ɏo�Ă���J�[�h���K�i���ǂ���
    simField->seq   = vh->seq;
    // �v������
    if(checkRev(vh)) simField->rev = !(simField->rev);
    // ���Ԃ���i�߂�
    *current = ((*current)+1)%5;
  }
}


double calcValueUnrollVH(int64 myCards, int rev, bitValidHandsArray* vhS){

  int handsSize = seq5Size+seq4Size+seq3Size+groupSize+jokerSize; // �l���������̐�(9+10+11+52+1=83�ʂ�)

  // �]���֐���̎���̃C���f�b�N�X
  int seq5Index  = 0;
  int seq4Index  = seq5Index+seq5Size;   // = 0+9  = 9;
  int seq3Index  = seq4Index+seq4Size;   // = 9+10 = 19;
  int groupIndex = seq3Index+seq3Size;   // = 19+11 = 30;
  int jokerIndex = groupIndex+groupSize; // = 30+52 = 82;

  // �]���֐��ōl�����鍀��
  int revSize = 2; // �v�������ǂ����B�ʏ�E�v����2�ʂ�

  // ���v�p�����[�^��
  int paramNum = revSize*handsSize; // = 2*83 = 166;

  double res = 0.0;
  int64 solo = myCards;
  int vhSize = 0;
  int i;

  /* printf("in :calcValueUnrollVH \n"); */
  /* show_param(); */

  for(i=0;i<vhS->size;i++){
    if((vhS->hands[i].hands&myCards) != vhS->hands[i].hands) continue;
    vhSize++;
    unsigned char suit = vhS->hands[i].suit;
    unsigned char ord  = vhS->hands[i].ord;
    int j = i+1;
    while(1){
      if(j<vhS->size && vhS->hands[j].suit==suit && vhS->hands[j].ord==ord && ((vhS->hands[j].hands&myCards) == vhS->hands[j].hands)) j++;
      else {
	j--; break; 
      }
    }
    solo ^= vhS->hands[j].hands;
    unsigned char qty = vhS->hands[j].qty;
    if(vhS->hands[j].qty == 3){
      res += param[handsSize*rev+ord-1+seq3Index];
    }
    else if(vhS->hands[j].qty == 4){
      res += param[handsSize*rev+ord-1+seq4Index];
    }
    else{
      res += param[handsSize*rev+ord-1+seq5Index];
    }
    i = j+1;
    while(i<vhS->size && vhS->hands[i].suit==suit && vhS->hands[i].ord < ord+qty) i++;
    i--;
  }
  if(vhSize != 0){
#ifdef USE_BIT64
    int64 card = myCards;
    int64 group = 0x0000008004002001ULL;
    for(i=0;i<13;i++){
      int64 pr = (card&group);
      if(pr){
	int count = (int)(((pr*group)>>39)&7);
	if(count > 1)
	  res += param[handsSize*rev+(4*i+count-1)+groupIndex];
	else if(solo&group)
	  res += param[handsSize*rev+4*i+groupIndex];
      }
      card = card >> 1;
      solo = solo >> 1;
    }

#else
    // ��D��Group�𒲂ׂ�D����3�`2��13�ʂ�
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    unsigned int soloLow = (unsigned int)solo,
      soloHi = (unsigned int)(solo >> 32);
    unsigned int cardLow = (unsigned int)myCards,
      cardHi = (unsigned int)(myCards>>32);
    unsigned int groupLow = 0x04002001,
      groupHi = 0x00000080;

    unsigned int pr = (cardLow&groupLow);
    int count = ((cardHi&groupHi)!=0) + (((pr>>26)+(pr>>13)+pr)&3);

    // group���`�F�b�N
    if(count > 1){
      res += param[handsSize*rev+(4*0+count-1)+groupIndex];
    }
    else if((soloHi&groupHi)||(soloLow&groupLow)){
      res += param[handsSize*rev+(4*0)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    soloLow = (soloLow>>1)|(soloHi<<(32-1));
    soloHi  = soloHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0) + ((pr*groupLow)>>26);
    // group���`�F�b�N
    if(count > 1){
      res += param[handsSize*rev+(4*1+count-1)+groupIndex];
    }
    else if((soloHi&groupHi)||(soloLow&groupLow)){
      res += param[handsSize*rev+(4*1)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    soloLow = (soloLow>>1)|(soloHi<<(32-1));
    soloHi  = soloHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0) + ((pr*groupLow)>>26);
    // group���`�F�b�N
    if(count > 1){
      res += param[handsSize*rev+(4*2+count-1)+groupIndex];
    }
    else if((soloHi&groupHi)||(soloLow&groupLow)){
      res += param[handsSize*rev+(4*2)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    soloLow = (soloLow>>1)|(soloHi<<(32-1));
    soloHi  = soloHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0) + ((pr*groupLow)>>26);
    // group���`�F�b�N
    if(count > 1){
      res += param[handsSize*rev+(4*3+count-1)+groupIndex];
    }
    else if((soloHi&groupHi)||(soloLow&groupLow)){
      res += param[handsSize*rev+(4*3)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    soloLow = (soloLow>>1)|(soloHi<<(32-1));
    soloHi  = soloHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0) + ((pr*groupLow)>>26);
    // group���`�F�b�N
    if(count > 1){
      res += param[handsSize*rev+(4*4+count-1)+groupIndex];
    }
    else if((soloHi&groupHi)||(soloLow&groupLow)){
      res += param[handsSize*rev+(4*4)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    soloLow = (soloLow>>1)|(soloHi<<(32-1));
    soloHi  = soloHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0) + ((pr*groupLow)>>26);
    // group���`�F�b�N
    if(count > 1){
      res += param[handsSize*rev+(4*5+count-1)+groupIndex];
    }
    else if((soloHi&groupHi)||(soloLow&groupLow)){
      res += param[handsSize*rev+(4*5)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    soloLow = (soloLow>>1)|(soloHi<<(32-1));
    soloHi  = soloHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0) + ((pr*groupLow)>>26);
    // group���`�F�b�N
    if(count > 1){
      res += param[handsSize*rev+(4*6+count-1)+groupIndex];
    }
    else if((soloHi&groupHi)||(soloLow&groupLow)){
      res += param[handsSize*rev+(4*6)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    soloLow = (soloLow>>1)|(soloHi<<(32-1));
    soloHi  = soloHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0) + ((pr*groupLow)>>26);
    // group���`�F�b�N
    if(count > 1){
      res += param[handsSize*rev+(4*7+count-1)+groupIndex];
    }
    else if((soloHi&groupHi)||(soloLow&groupLow)){
      res += param[handsSize*rev+(4*7)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    soloLow = (soloLow>>1)|(soloHi<<(32-1));
    soloHi  = soloHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0) + ((pr*groupLow)>>26);
    // group���`�F�b�N
    if(count > 1){
      res += param[handsSize*rev+(4*8+count-1)+groupIndex];
    }
    else if((soloHi&groupHi)||(soloLow&groupLow)){
      res += param[handsSize*rev+(4*8)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    soloLow = (soloLow>>1)|(soloHi<<(32-1));
    soloHi  = soloHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0) + ((pr*groupLow)>>26);
    // group���`�F�b�N
    if(count > 1){
      res += param[handsSize*rev+(4*9+count-1)+groupIndex];
    }
    else if((soloHi&groupHi)||(soloLow&groupLow)){
      res += param[handsSize*rev+(4*9)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    soloLow = (soloLow>>1)|(soloHi<<(32-1));
    soloHi  = soloHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0) + ((pr*groupLow)>>26);
    // group���`�F�b�N
    if(count > 1){
      res += param[handsSize*rev+(4*10+count-1)+groupIndex];
    }
    else if((soloHi&groupHi)||(soloLow&groupLow)){
      res += param[handsSize*rev+(4*10)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    soloLow = (soloLow>>1)|(soloHi<<(32-1));
    soloHi  = soloHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0) + ((pr*groupLow)>>26);
    // group���`�F�b�N
    if(count > 1){
      res += param[handsSize*rev+(4*11+count-1)+groupIndex];
    }
    else if((soloHi&groupHi)||(soloLow&groupLow)){
      res += param[handsSize*rev+(4*11)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    soloLow = (soloLow>>1)|(soloHi<<(32-1));
    soloHi  = soloHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0) + ((pr*groupLow)>>26);
    // group���`�F�b�N
    if(count > 1){
      res += param[handsSize*rev+(4*12+count-1)+groupIndex];
    }
    else if((soloHi&groupHi)||(soloLow&groupLow)){
      res += param[handsSize*rev+(4*12)+groupIndex];
    }
#endif
  }
  else {
#ifdef USE_BIT64
    int64 card = myCards;
    int64 group = 0x0000008004002001ULL;
		
    for(i=0;i<13;i++){
      int64 pr = (card&group);
      if(pr){
	int count = (int)(((pr*group)>>39)&7);
	res += param[handsSize*rev+(4*i+count-1)+groupIndex];
      }
      card = card >> 1;
    }
#else
    // ��D��Group�𒲂ׂ�D����3�`2��13�ʂ�
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    unsigned int cardLow = (unsigned int)myCards, cardHi = (unsigned int)(myCards>>32);
    unsigned int groupLow = 0x04002001, groupHi = 0x00000080;

    unsigned int pr = (cardLow&groupLow);
    int count = ((cardHi&groupHi)!=0);

    if(count|pr){
      count += ((pr*groupLow)>>26);
      res += param[handsSize*rev+(4*0+count-1)+groupIndex];
    }
    // group���`�F�b�N
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0);
    // group���`�F�b�N
    if(count|pr){
      count += ((pr*groupLow)>>26);
      res += param[handsSize*rev+(4*1+count-1)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0);
    // group���`�F�b�N
    if(count|pr){
      count += ((pr*groupLow)>>26);
      res += param[handsSize*rev+(4*2+count-1)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0);
    // group���`�F�b�N
    if(count|pr){
      count += ((pr*groupLow)>>26);
      res += param[handsSize*rev+(4*3+count-1)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0);
    // group���`�F�b�N
    if(count|pr){
      count += ((pr*groupLow)>>26);
      res += param[handsSize*rev+(4*4+count-1)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0);
    // group���`�F�b�N
    if(count|pr){
      count += ((pr*groupLow)>>26);
      res += param[handsSize*rev+(4*5+count-1)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0);
    // group���`�F�b�N
    if(count|pr){
      count += ((pr*groupLow)>>26);
      res += param[handsSize*rev+(4*6+count-1)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0);
    // group���`�F�b�N
    if(count|pr){
      count += ((pr*groupLow)>>26);
      res += param[handsSize*rev+(4*7+count-1)+groupIndex];
    }
    cardLow = (cardLow>>1);
    cardHi  = cardHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0);
    // group���`�F�b�N
    if(count|pr){
      count += ((pr*groupLow)>>26);
      res += param[handsSize*rev+(4*8+count-1)+groupIndex];
    }
    cardLow = (cardLow>>1);
    cardHi  = cardHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0);
    // group���`�F�b�N
    if(count|pr){
      count += ((pr*groupLow)>>26);
      res += param[handsSize*rev+(4*9+count-1)+groupIndex];
    }
    cardLow = (cardLow>>1);
    cardHi  = cardHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0);
    // group���`�F�b�N
    if(count|pr){
      count += ((pr*groupLow)>>26);
      res += param[handsSize*rev+(4*10+count-1)+groupIndex];
    }
    cardLow = (cardLow>>1);
    cardHi  = cardHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0);
    // group���`�F�b�N
    if(count|pr){
      count += ((pr*groupLow)>>26);
      res += param[handsSize*rev+(4*11+count-1)+groupIndex];
    }
    cardLow = (cardLow>>1)|(cardHi<<(32-1));
    cardHi  = cardHi >> 1;
    // ����i�̃J�[�h�̖����𐔂���(bitCount)
    pr = (cardLow&groupLow);
    count = ((cardHi&groupHi)!=0);
    // group���`�F�b�N
    if(count|pr){
      count += ((pr*groupLow)>>26);
      res += param[handsSize*rev+(4*12+count-1)+groupIndex];
    }
#endif
  }
		
  // JOKER�̌��o. 
  if(myCards&(1LL<<52)) res += param[handsSize*rev+jokerIndex];
  return res;
}

// ���̏�Ԃ𐶐����C�����I���t���O��Ԃ��D
int getNextState(int result[5], fieldInfo *simField, int *current, int64 playersCard[5],
		 bitValidHandsArray vhcG[5], bitValidHandsArray vhcS[5],
		 const bitValidHand *vhb, int targetPlayer){
  int j;
  // �p�X����ꍇ�̏���
  if(vhb->hands==0ULL){
    simulatePass(simField, current);
  } 
  // �J�[�h���o���ꍇ�̏���
  else {
    // ���݂̃v���C���[���オ��ꍇ�̏���
    if(simField->lest[*current] == vhb->qty){
      if(targetPlayer == -1){
	int count = 0;
	for(j=0;j<5;j++) count += ((simField->goal>>j)&1);
	// rank = 5 - ���ɏオ�����l��
	result[*current] = 5-count;
	// �c��l��1�l�Ȃ玎���I��(������ς�3�l+����������1�l�{�c��1�l)
	if(count == 3){
	  for(j=0;j<5;j++){
	    if(((simField->goal)&(1<<j))||j==*current) continue;
	    result[j] = 1;
	    return 1;
	  }
	}
	else {
	  simField->goal |= (1<<*current);
	}
      }
      else {
	// �������オ�����珇�ʂ��Z�b�g���ďI��
	if(*current == targetPlayer){
	  int count = 0;
	  for(j=0;j<5;j++) count += ((simField->goal>>j)&1);
	  // rank = 5 - ���ɏオ�����l��
	  result[targetPlayer] = 5-count;
	  return 1;
	}
	// �オ�����̂������łȂ���Ώオ��t���O���X�V
	else {
	  simField->goal |= (1<<*current);
	  // �����ȊO�݂�Ȃ��オ���Ă��܂�����I��
	  if((simField->goal|(1<<targetPlayer))==(1<<5)-1){
	    result[targetPlayer] = 1;
	    return 1;
	  }
	}
      }
    }
    // �J�[�h���o���C�t�B�[���h�̏����X�V����
    removeHands(&vhcG[*current], &vhcS[*current], vhb->hands, playersCard[*current]);
    simulateSubmit(vhb, simField, playersCard, current);
  }

  while(((simField->pass>>*current)&1) == 1 || simField->lest[*current] == 0){
    if(simField->lest[*current] == 0){
      simulatePass(simField, current);
      continue;
    }
    *current = (*current+1)%5;
  }

  return 0;
}

int getActionIndex(const bitValidHandsArray *vhb, const bitValidHandsArray *vhS, int64 myCard, int curRev, int lest){
  int j;

  // �������肪���݂���΂�����̗p����
  for(j=vhb->size-1;j>=0;j--)
    if(vhb->hands[j].qty == lest) return j;

  // �s����1���Ȃ炻�̍s��������
  if(vhb->size == 1) return 0;

  // ����ɏ]���Ċm���I�ɍs�������肷��

  double sumProb = 0.0;
  double prob[128];
  bitValidHandsArray bvha; bvha.size = 0;

  for(j=0;j<vhS->size;j++){
    if(vhS->hands[j].hands&(1LL<<52)) continue;
    bvha.hands[bvha.size++] = vhS->hands[j];
  }

  for(j=0;j<vhb->size;j++){
    int rev = curRev;
    if(vhb->hands[j].qty>3+vhb->hands[j].seq) rev = !rev;

    prob[j] = exp(calcValueUnrollVH(myCard^vhb->hands[j].hands, rev, &bvha));

    sumProb += prob[j];
  }

  double thr = sumProb*genrand_real2();
  double cursum = 0.0;
					
  for(j=0;j<vhb->size-1;j++){
    cursum += prob[j];
    if(thr <= cursum) return j;
  }

  return vhb->size-1;
}


void doSimulation(int result[5], 
		  const int64 cards[5], const fieldInfo *finfo, int currentPlayer,
		  const bitValidHandsArray vhG[5],
		  const bitValidHandsArray vhS[5],
		  int targetPlayer){
  int i, j;
  int64 playersCard[5];
  bitValidHandsArray vhb, vhcG[5], vhcS[5];
  fieldInfo simField = *finfo;
  int current = currentPlayer;
  double prob[128];

  for(i=0;i<5;i++){
    result[i] = -1;
    playersCard[i] = cards[i];
    vhcG[i] = vhG[i];
    vhcS[i] = vhS[i];
  }

  int turnCount = 0;

  while(1){

    // �o����J�[�h��S�񋓂���
    getAllValidHands(&vhb, &vhcG[current], &vhcS[current], &simField, playersCard[current]);

    // ��ɃJ�[�h������ꍇ�̓p�X���l��
    if(simField.onset==0){
      vhb.hands[vhb.size  ].hands = 0ULL;
      vhb.hands[vhb.size  ].qty   = 0;
      vhb.hands[vhb.size++].seq   = 0;
    }

    // ����̂�������m���I�ɍs����I������
    int useIdx = getActionIndex(&vhb, &vhcS[current], playersCard[current], simField.rev, simField.lest[current]);

    // ���̏�Ԃ𓾂�
    // �I���t���O���Ԃ��ꂽ��V�~�����[�V�����I���
    if(getNextState(result, &simField, &current, playersCard, vhcG, vhcS, &vhb.hands[useIdx], targetPlayer))
      return ;
  }
}

int nodeCount;

int lastSearch(int64 myCards, int64 oppCards, int current, const fieldInfo *finfo, int many, int depth){
#ifdef USE_MAX_SEARCH_NODE
  if(nodeCount > MAX_SEARCH_NODE) return 0;
#endif
  nodeCount++;
  int i;
  // StackOverFlow����D�������ߖ񂵂Ă���ȏ�T�����Ă����ʂ͂قƂ�Ǖς��Ȃ��D
  if(depth >= 30) return 0;
  bitValidHandsArray vha, vhG, vhS, vhcG[5], vhcS[5];
  if(finfo->mypos == current){
    // �����̏o������J�[�h���
    generateAllHands(&vhG, &vhS, myCards);
    getAllValidHands(&vha, &vhG, &vhS, finfo, myCards);
  } else {
    if(many == 1){
      // ����ɒ�o�\�ȃJ�[�h������Ε����Ɣ��f
      // ��ɏo�Ă���J�[�h�̖�������D�̖�����葽���������o�ł��Ȃ�
      if((finfo->onset == 1 || finfo->qty <= finfo->lest[current]) && (checkAllValidHands(finfo, oppCards)==1))
	return 0;
      vha.size = 0;
    } else {
      // ����̏o������J�[�h���
      // ����ɑ΂��Ă͎�������J�[�h���ׂĂ��l������
      generateAllHands(&vhG, &vhS, oppCards);
      getAllValidHands(&vha, &vhG, &vhS, finfo, oppCards);
    }
  }
  for(i=0;i<vha.size;i++){
    // �������肪���݂��邩�`�F�b�N
    if(vha.hands[i].qty == finfo->lest[current]){
      if(finfo->mypos == current) return 1; // ����
      else                        return 0; // ����
    }
  }
  if(finfo->onset == 0){
    vha.hands[vha.size  ].hands = 0ULL;
    vha.hands[vha.size  ].qty   = 0;
    vha.hands[vha.size++].seq   = 0;
  }
  int resultRank[5];
  int64 playersCard[5];
  for(i=0;i<vha.size;i++){
    // �莝��������薇�����������̂͒�o�ł��Ȃ�(����v���C����������)
    if(vha.hands[i].qty > finfo->lest[current]) continue;
    int curPlayer = current;
    if(finfo->mypos == current){
      playersCard[current] = myCards;
    } else {
      playersCard[current] = oppCards;
    }
    vhcG[current] = vhG;
    vhcS[current] = vhS;
    fieldInfo simField = (*finfo);
    getNextState(resultRank, &simField, &curPlayer, playersCard, vhcG, vhcS, &vha.hands[i], -1);
    if(finfo->mypos == current){
      // �q�m�[�h�ɕK���肪1�ł�����ΕK��
      if(lastSearch(playersCard[current], oppCards, curPlayer, &simField, many, depth+1) == 1) return 1;
    } else {
      // �����ɂȂ�肪1�ł�����Ε���
      if(lastSearch(myCards, playersCard[current], curPlayer, &simField, many, depth+1) == 0) return 0;
    }
  }
  // �����F�q�m�[�h�ɏ��Ă�肪�Ȃ�������
  // ����F�q�m�[�h���K����̂݁�����
  return finfo->mypos != current;
}

// �K����̒T��
int lastSearchTop(int64 myCards, int64 oppCards, const fieldInfo *finfo){
  nodeCount = 0;
  int i;
  int many = (bitCount(oppCards) > 15); // ����S���̎c��J�[�h���v��15���ȏ�Ȃ瑊��̃J�[�h��o�������Ȃ�
  bitValidHandsArray vha, vhG, vhS, vhcG[5], vhcS[5];
  generateAllHands(&vhG, &vhS, myCards);
  getAllValidHands(&vha, &vhG, &vhS, finfo, myCards);
  int resultRank[5];
  int64 playersCard[5];
  if(finfo->onset == 0){
    vha.hands[vha.size  ].hands = 0ULL;
    vha.hands[vha.size  ].qty   = 0;
    vha.hands[vha.size++].seq   = 0;
  }
  for(i=0;i<vha.size;i++){
    int current = finfo->mypos;
    playersCard[current] = myCards;
    vhcG[current] = vhG;
    vhcS[current] = vhS;
    fieldInfo simField = (*finfo);
    getNextState(resultRank, &simField, &current, playersCard, vhcG, vhcS, &vha.hands[i], -1);
    if(lastSearch(playersCard[finfo->mypos], oppCards, current, &simField, many, 1) == 1) return i;
  }
  return -1;
}

void read_param(){
  // ----------------- reading params
  FILE *fp;
  static int done = 0;

  if(done ==1 ){
    //printf("no read \n");
    return;
  }
  done = 1;
  
  fp = fopen("param.dat", "r");
  int param_i;
  double df;
  
  //printf("hello\n");
  for(param_i=0; param_i<166; param_i++){
    //scanf(fp, "%lf", &param[param_i]);
    fscanf(fp, "%lf", &df);
    param[param_i] = df;
    //printf("%d %f\n",param_i, param[param_i]);
    //printf("%d %lf\n",param_i, df);
  }
  
  /* for(param_i=0; param_i<166; param_i++){ */
  /*   //if(param_i % 10 == 0) printf("\n"); */
  /*   printf("%d %20.17f\n", param_i, param[param_i]); */
  /* } */
  /* printf("\n"); */

  fclose(fp);
  // ----------------- end reading params
}

#if 0
int show_param(){
  // ----------------- reading params
  /* static done = 0; */

  /* if(done ==1 ) return; */
  /* done = 1; */
  
  int param_i;
  
  for(param_i=0; param_i<166; param_i++){
    //if(param_i % 10 == 0) printf("\n");
    printf("%d %20.17f\n", param_i, param[param_i]);
  }
  printf("\n");
  // ----------------- end reading params
}
#endif


// ���݂̏󋵂��烂���e�J�����@�ōs�������肷��
// �����e�J�����@�ƌ����C���O�ɕK����T���Ȃ񂩂��s�Ȃ��Ă���
void monteCarloSearch(int out_cards[8][15], int64 myCards, int64 oppCards, const changeInfo *cinfo, const fieldInfo *finfo){
  int i, j;
  // �e�v���C���[�̎�D
  int64 playersCard[5];
  // �e��̃V�~�����[�V������
  int simulateCount[MAX_ARRAY_SIZE] = {0};
  // �e��̃V�~�����[�V�����X�R�A�̘a
  double simulateScore[MAX_ARRAY_SIZE] = {0};
  // �e��̃V�~�����[�V�����X�R�A�̓��a
  double simulateDScore[MAX_ARRAY_SIZE] = {0};
  // �\�Ȏ���i�[����\����
  bitValidHandsArray vha, vhb, vhcG[5], vhcS[5], vhdG, vhdS;


  //printf("simulationCount[5] %d ", simulateCount[5]);

  read_param();
  //show_param();

  //printf("%d\n", finfo->rank[finfo->mypos] );
  //  printf("%llx\n", myCards);
  
  int csa[8][15];
  bitToArray(csa, myCards);
  int csi, csj;
  int cssum;
  cssum=0;
  for(csi=0; csi<5; csi++){
    for(csj=8; csj<13; csj++){
      cssum = cssum + csa[csi][csj];
    }
    for(csj=0; csj<5; csj++){
      cssum = cssum - csa[csi][csj];
    }
  }
  //printf("strong %d\n", cssum);

  //printf("lest %d \n", finfo->lest[finfo->mypos]);
  
  
  int nowrank;
  nowrank = finfo->rank[finfo->mypos];

  if(finfo->lest[finfo->mypos] > 5){
    if(cssum > 2) {
      //printf("                  strong %d\n", cssum);
      nowrank = nowrank - 1;
      if(nowrank <0) nowrank = 0;
    }
    
    if(cssum <= 1) {
      if(nowrank <= 2){
	//printf("                    weak %d\n", cssum);
	nowrank = nowrank + 1;
	if(nowrank >4) nowrank = 4;
      }
    }
  }

  
  nowrank = finfo->rank[finfo->mypos];
  
  double rankfanc[5];
  switch(nowrank){
  case 4:
    rankfanc[0]=0.0;
    rankfanc[1]=0.5;
    rankfanc[2]=0.98;
    rankfanc[3]=0.99;
    rankfanc[4]=1.0;
    break;

  case 3:
    rankfanc[0]=0.0;
    rankfanc[1]=0.01;
    rankfanc[2]=0.1;
    rankfanc[3]=0.99;
    rankfanc[4]=1.0;
    break;

  case 2:
    rankfanc[0]=0.00;
    rankfanc[1]=0.01;
    rankfanc[2]=0.02;
    rankfanc[3]=0.9;
    rankfanc[4]=1.0;
    break;

  case 1:
    rankfanc[0]=0.0;
    rankfanc[1]=0.4;
    rankfanc[2]=0.6;
    rankfanc[3]=0.8;
    rankfanc[4]=1.0;
    break;

  case 0:
    rankfanc[0]=0.0;
    rankfanc[1]=0.4;
    rankfanc[2]=0.6;
    rankfanc[3]=0.8;
    rankfanc[4]=1.0;
    break;

  default:
    rankfanc[0]=0.0001;
    rankfanc[1]=0.001;
    rankfanc[2]=0.01;
    rankfanc[3]=0.1;
    rankfanc[4]=0.0;
  }
    
  


  // ����Ŏ�蓾��s�������ׂė񋓂���
  generateAllHands(&vhdG, &vhdS, myCards);
  getAllValidHands(&vha, &vhdG, &vhdS, finfo, myCards);

  // �オ���肪���݂���ꍇ�͂�����̗p����
  int decFlag = -1;
  for(i=0;i<vha.size;i++){
    if(vha.hands[i].qty == finfo->lest[finfo->mypos]){
      decFlag = i;
    }
  }

  if(finfo->onset==0){
    vha.hands[vha.size  ].hands = 0ULL;
    vha.hands[vha.size  ].qty   = 0;
    vha.hands[vha.size++].seq   = 0;
  }

  // �s����1�ʂ肵���Ȃ���΂����I��
  if(vha.size == 1) decFlag = 0;

  // �K���肪���݂���΂����I��
  if(decFlag == -1)
    decFlag = lastSearchTop(myCards, oppCards, finfo);

  double score[MAX_ARRAY_SIZE];

  // i==vha.size �̏ꍇ�̓p�X�Ƃ��čl����
  if(decFlag == -1){
    for(i=0;i<SIMULATION_COUNT;i++){
      for(j=0;j<vha.size;j++){
	// UCB1-TUNED �ɂ�莟�ɃV�~�����[�g������I��
	if(simulateCount[j] == 0) score[j] = 1e12;
	else {
	  double avg  = simulateScore[j]/(double)simulateCount[j];
	  double avg2 = simulateDScore[j]/(double)simulateCount[j];
	  double V = avg2-(avg*avg)+sqrt((2*log((double)i))/simulateCount[j]);
	  //if(V > 0.25) V = 0.25;

	  double vvv = 0.5;
	  if(V > vvv) V = vvv;
	  //V=2.0;
	  score[j] = avg + sqrt(V*log((double)i)/(double)simulateCount[j]);

	}
      }
      int idx = 0;
      for(j=0;j<vha.size;j++){
        //printf("score[j] %f ", score[j]);
	if(score[idx] < score[j]) idx = j;
      }
      // �Ƃ��Ă����Ȃ���A��idx �� soft max �Ō��ߒ���
      //double soft_sum = 0.0;
      //for(j=0;j<vha.size;j++){
      //soft_sum = score[j];
      //      }
      //double soft_gate;
      
      //soft_gate = genrand_real3();
      
      //for(j=0;j<vha.size;j++){
      //soft_gate = soft_gate- score[j]/soft_sum;
      //if(soft_gate < 0.0) break;
      //}
      //idx = j;      
	
      //printf("      :      ");

      fieldInfo simField = *finfo;

      int current = finfo->mypos;

      // �J�[�h��z������D�����ɃW���[�J�[�ێ��t���O����������D
#ifdef USE_ESTIMATE_HAND
      //estimatedRandomCardDevide(playersCard, myCards, oppCards, cinfo, finfo);
#else
      randomCardDevide(playersCard, myCards, oppCards, cinfo, finfo);
#endif

      for(j=0;j<5;j++){
	if(j==finfo->mypos){
	  int k;
	  for(k=0;k<vhdG.size;k++) vhcG[j].hands[k] = vhdG.hands[k];
	  vhcG[j].size = vhdG.size;
	  for(k=0;k<vhdS.size;k++) vhcS[j].hands[k] = vhdS.hands[k];
	  vhcS[j].size = vhdS.size;
	}
	else{
	  if(finfo->lest[j] > 2)
	    generateAllHands(&vhcG[j], &vhcS[j], playersCard[j]);
	  else {
	    vhcG[j].size = vhcS[j].size = 0;
	    if(finfo->lest[j] > 0)
	      generateGroup(&vhcG[j], playersCard[j]);
	  }
	}
      }

      int resultRank[5];

      // �v���C���[�̏������������
      // ����ł�����肪����΂��̃��[�v�ɓ����Ă��Ȃ��̂ŕԂ�l�͏��false
      getNextState(resultRank, &simField, &current, playersCard, vhcG, vhcS, &vha.hands[idx], finfo->mypos);
		
      doSimulation(resultRank, playersCard, &simField, current, vhcG, vhcS, finfo->mypos);
			
      int myRank = resultRank[finfo->mypos];
      /* double rankfanc[]={0.2*0.2*0.2*0.2*0.2, */
      /* 			 0.4*0.4*0.4*0.4*0.4, */
      /* 			 0.6*0.6*0.6*0.6*0.6, */
      /* 			 0.8*0.8*0.8*0.8*0.8, */
      /* 			 1.0}; */
      double myRankdbl;

      myRankdbl=rankfanc[myRank-1];
      
      // �V�~�����[�g�̌��ʂ��X�V
      simulateCount[idx]++;
      simulateScore[idx] += myRankdbl;
      simulateDScore[idx] += myRankdbl*myRankdbl;
      //simulateScore[idx] += (myRank/5.0)*(myRank/5.0)*(myRank/5.0)*(myRank/5.0);
      //simulateDScore[idx] += (myRank/5.0)*(myRank/5.0)*(myRank/5.0)*(myRank/5.0)*(myRank/5.0)*(myRank/5.0)*(myRank/5.0)*(myRank/5.0);
      //simulateScore[idx] += (myRank*myRank)/25.0;
      //simulateDScore[idx] += (myRank*myRank*myRank*myRank)/(25.0*25.0);
      /* simulateScore[idx] += myRank*myRank*myRank/(5.0*25.0); */
      /* simulateDScore[idx] += (myRank*myRank*myRank*myRank*myRank)/(5.0*25.0*25.0); */
      //simulateScore[idx] += myRank*myRank/(25.0); 
      //simulateDScore[idx] += (myRank*myRank*myRank*myRank*myRank)/(5.0*25.0*25.0);
      ///simulateScore[idx] += myRank*myRank*myRank/(25.0*5.0); 
      ///simulateDScore[idx] += myRank*myRank*myRank*myRank/(25.0*25.0);
      //simulateScore[idx] += myRank*myRank*myRank*myRank*myRank/(25.0*5.0*5.0*5.0); 
      //simulateDScore[idx] += myRank*myRank*myRank*myRank*myRank*myRank*myRank*myRank/(25.0*25.0*25.0*25.0);
    }
  }

  int idx = 0;
  double maxScore = 0.0;

  // �X�R�A�ő�̎�𒲂ׂ�
  if(decFlag == -1){
    for(i=0;i<vha.size+1;i++){
      double tmp = 0.0;
      // �V�~�����[�V������0�̎�̃X�R�A��0�Ƃ��čl����
      if(simulateCount[i] != 0)
	tmp = (double)simulateScore[i]/(double)simulateCount[i];
      if(maxScore <= tmp){
	maxScore = tmp;
	idx = i;
      }
    }
  } else {
    // �V�~�����[�V�����ɂ�炸�肪���܂��Ă���΂�����g��
    idx = decFlag;
  }

  if(vha.hands[idx].hands == 0LL){
    // �p�X����Ƃ�
    for(i=0;i<8;i++){
      for(j=0;j<15;j++) out_cards[i][j] = 0;
    }
  } else {
    // ��o����J�[�h��z��ɕϊ�
    setSubmitCard(out_cards, &vha.hands[idx]);
  }
}
