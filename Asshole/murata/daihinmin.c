/*daifugo*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "daihinmin.h"
#include "connection.h"

//�󋵕\���p(1�̎��\���A0�̎���\��)
#define status_print 0

int seat_tactics_check_on = 0;
int seat_tactics_check_off = 0;
int ba_nagare_single = 0;
int ba_nagare_double = 0;
int ba_nagare_three_cards = 0;
int ba_nagare_max_count = 0;
int ba_nagare_max_count_single=0;
int ba_nagare_max_count_single_nomal=0;
int ba_nagare_max_count_single_nomal_rev=0;
int ba_nagare_max_count_single_sibari=0;
int ba_nagare_max_count_single_sibari_rev = 0;
int ba_nagare_max_count_double=0;
int ba_nagare_max_count_double_nomal=0;
int ba_nagare_max_count_double_nomal_rev = 0;
int ba_nagare_max_count_double_sibari=0;
int ba_nagare_max_count_double_sibari_rev=0;
int flag_count_normal=0;
int seat_tactics_check[100000][3] = {0};
int pass_check_count=0;
int ba_nagare_last_playernum = 10;

int seat_tactics_flag = 0;

int client_log[2][4][14][5] = { 0 };
int client_log_st_pass_off_count=0;


extern const int g_logging;

void getState(int cards[8][15]){
  /*
    �J�[�h�e�[�u�����瓾�������ǂݍ���
    �����͎�D�̃J�[�h�e�[�u��
    ���͍L��ϐ�state�Ɋi�[�����
  */
  int i;
  //���
  if(cards[5][4]>0) state.onset=1; //��ɃJ�[�h���Ȃ��Ƃ� 1
  else              state.onset=0;
  if(cards[5][6]>0) state.rev=1;   //�v����Ԃ̎� 1 
  else              state.rev=0;
  if(cards[5][5]>0) state.b11=1;   //11�o�b�N�� 1 ���g�p
  else              state.b11=0;
  if(cards[5][7]>0) state.lock=1;  //���΂莞 1
  else              state.lock=0;

  if(state.onset==1){   //�V���ȏ�̂Ƃ��D�̏������Z�b�g
    state.qty=0;
    state.ord=0;
    state.lock=0;
    for(i=0;i<5;i++)state.suit[i]=0;
  }
  
  for(i=0;i<5;i++) state.player_qty[i]=cards[6][i];   //�莝���̃J�[�h
  for(i=0;i<5;i++) state.player_rank[i]=cards[6][5+i];//�e�v���[���̃����N
  for(i=0;i<5;i++) state.seat[i]=cards[6][10+i];      //�N���ǂ̃V�[�g�ɍ����Ă��邩
                                                      //�V�[�gi�Ƀv���[�� STATE.SEAT[I]�������Ă���

  if(cards[4][1]==2) state.joker=1;     //Joker�����鎞 1
  else               state.joker=0;

  
}

void getField(int cards[8][15]){
  /*
    ��ɏo���J�[�h�̏��𓾂�B
    �����͏�ɏo���J�[�h�̃e�[�u��
    ���͍L��ϐ�state�Ɋi�[�����
  */
  int i,j,count=0;
  i=j=0;
  
  //�J�[�h�̂���ʒu��T��
  while(j<15&&cards[i][j]==0){
    state.suit[i]=0;
    i++;
    if(i==4){
      j++;
      i=0;
    }
  }
  //�K�i���ۂ�
  if(j<14){
    if(cards[i][j+1]>0) state.sequence=1;
    else state.sequence=0;
  }
  //�����𐔂��� �܂������𒲂ׂ�
  if(state.sequence==0){
    //�����g
    for(;i<5;i++){
      if(cards[i][j]>0){
	count++;
	state.suit[i]=1;
      }else{
	state.suit[i]=0;
      }
    }
    if(j==0||j==14){
      if(state.rev==0){
	state.ord=14;
      }else{
	state.ord=0;
      }
    }else{
      state.ord=j;
    }
  }else{
    //�K�i
    while(j+count<15 && cards[i][j+count]>0){
      count++;
    }
    if((state.rev==0 && state.b11==0 )||( state.rev==1 && state.b11==1 )){
      state.ord=j+count-1;
    }else{
      state.ord=j;
    }
    state.suit[i]=1;
    }
  //�������L��
  state.qty=count;
 
  if(state.qty>0){ //������0���傫���Ƃ� �V������̃t���O��0�ɂ���
    state.onset=0;
  }
}

void showState(struct state_type *state){
  /*�����œn���ꂽ���state�̓��e��\������*/
  int i;
  printf("state rev   : %d\n",state->rev);
  printf("state lock  : %d\n",state->lock);
  printf("state joker : %d\n",state->joker);
  
  printf("state qty   : %d\n",state->qty);
  printf("state ord   : %d\n",state->ord);
  printf("state seq   : %d\n",state->sequence);
  printf("state onset : %d\n",state->onset);
  printf("state suit :");
  for(i=0;i<4;i++)printf("%d ",state->suit[i]);
  printf("\n"); printf("state player qty :");
  for(i=0;i<5;i++)printf("%d ",state->player_qty[i]);
  printf("\n"); printf("state player rank :");
  for(i=0;i<5;i++)printf("%d ",state->player_rank[i]);
  printf("\n"); printf("state player_num on seat :");
  for(i=0;i<5;i++)printf("%d ",state->seat[i]);
  printf("\n");
}

//���ꂼ��J�[�h�̘a ���� ���� �t�] ���Ƃ�
void cardsOr(int cards1[8][15],int cards2[8][15]){
  /*
    cards1��cards2�ɂ���J�[�h��������
  */
  int i,j;
  
  for(i=0;i<15;i++)
    for(j=0;j<5;j++)
      if(cards2[j][i]>0)cards1[j][i]=1; 
}

void cardsAnd(int cards1[8][15],int cards2[8][15]){ 
  /*
    cards1�̃J�[�h�̂����Acards2�ɂ�����̂�����cards1�ɂ̂����B
  */
  int i,j;
  
  for(i=0;i<15;i++)
    for(j=0;j<5;j++)
      if(cards1[j][i]==1&&cards2[j][i]==1) cards1[j][i]=1;
      else cards1[j][i]=0;
}

void cardsDiff(int cards1[8][15],int cards2[8][15]){ 
  /*
    cards1����cards2�ɂ���J�[�h���폜����
  */
  int i,j;
  
  for(i=0;i<15;i++)
    for(j=0;j<5;j++)
      if(cards2[j][i]==1) cards1[j][i]=0;
}

void cardsNot(int cards[8][15]){ 
  /*
    �J�[�h�̗L���𔽓]������
  */
  int i,j;
  
  for(i=0;i<15;i++)
    for(j=0;j<5;j++)
      if(cards[j][i]==1) cards[j][i]=0;
      else cards[j][i]=1;
}

void outputTable(int table[8][15]){ 
  /*
    �����œn���ꂽ�J�[�h�e�[�u�����o�͂���
  */
  int i,j;
  for(i=0;i<8;i++){
    for(j=0;j<15;j++){
      printf("%i ",table[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}

void copyTable(int dest_table[8][15], int org_table[8][15]){ 
  /*
    �����œn���ꂽ�J�[�h�e�[�u��org_table��
    �J�[�h�e�[�u��dest_table�ɃR�s�[����
  */
  int i,j;
  for(i=0;i<8;i++){
    for(j=0;j<15;j++){
      dest_table[i][j]=org_table[i][j];
    }
  }
} 

void copyCards(int dest_cards[8][15],int org_cards[8][15]){ 
  /*
    �����œn���ꂽ�J�[�h�e�[�u��org_cards�̃J�[�h���̕�����
    �J�[�h�e�[�u��dest_cards�ɃR�s�[����
  */
  int i,j;
  for(i=0;i<5;i++){
    for(j=0;j<15;j++){
      dest_cards[i][j]=org_cards[i][j];
    }
  }
}

void clearCards(int cards[8][15]){  
  /*
    �����œn���ꂽ�J�[�h�e�[�u��cards�̃J�[�h���̕�����S��0�ɂ��A�J�[�h���ꖇ��������Ԃɂ���B
  */
  int s,t;
  
  for(s=0;s<5;s++){
    for(t=0;t<15;t++){
      cards[s][t]=0;
    }   
  }
}

void clearTable(int cards[8][15]){ 
  /*
    �����œn���ꂽ�J�[�h�e�[�u��cards��S��0�ɂ���B
  */
  int s,t;
  
  for(s=0;s<8;s++){
    for(t=0;t<15;t++){
      cards[s][t]=0;
    }   
  }
}

int beEmptyCards(int cards[8][15]){  
  /*
    �����œn���ꂽ�J�[�h�e�[�u��cards�̊܂ރJ�[�h�̖�����0�̂Ƃ�1���A
    ����ȊO�̂Ƃ�0��Ԃ�
  */
  int i,j,f=1;
  
  for(i=0;i<5;i++){
    for(j=0;j<15;j++){
      if(cards[i][j]>0)f=0;
    }
  }
  return f;
}

int qtyOfCards(int cards[8][15]){  
  /*
    �����œn���ꂽ�J�[�h�e�[�u��cards�̊܂ރJ�[�h�̖�����Ԃ�
  */
  int i,j,count=0;
  
  for(i=0;i<5;i++)
    for(j=0;j<15;j++)
      if(cards[i][j]>0)count++;
  
  return count;
}

void makeJKaidanTable(int tgt_cards[][15], int my_cards[][15]){
  /*
    �n���ꂽ�J�[�h�e�[�u��my_cards����A�W���[�J�[���l�����K�i�ŏo���邩�ǂ�������͂��A
    ���ʂ��e�[�u��tgt_cards�Ɋi�[����B
  */
  int i,j;
  int count,noJcount;    //�W���[�J�[���g�p�����ꍇ�̃J�[�h�̖���,�g�p���Ȃ�����
  
  clearTable(tgt_cards);         //�e�[�u���̃N���A
  if(state.joker==1){            //joker������Ƃ�
    for(i=0;i<4;i++){            //�e�X�[�g���ɑ�����
      count=1;
      noJcount=0; 
      for(j=13;j>=0;j--){        //���Ԃɂ݂�
	if(my_cards[i][j]==1){   //�J�[�h������Ƃ�
	  count++;               //2�̃J�E���^��i�߂�
	  noJcount++;
	}
	else{                    //�J�[�h���Ȃ��Ƃ�
	  count=noJcount+1;      //�W���[�J�[����̊K�i�̖����ɃW���[�J�[���𑫂� 
	  noJcount=0;            //�W���[�J�[�Ȃ��̊K�i�̖��������Z�b�g����
	}
	
	if(count>2){              //3���ȏ�̂Ƃ�
	  tgt_cards[i][j]=count;  //���̖������e�[�u���Ɋi�[
	}
	else{
	  tgt_cards[i][j]=0;      //���̑���0�ɂ���
	}
      }
    }
  }

  if(g_logging==1){
    printf("make Joker kaidan \n");
    outputTable(tgt_cards);
  }
}

void makeKaidanTable(int tgt_cards[][15], int my_cards[][15]){
  /*
    �n���ꂽ�J�[�h�e�[�u��my_cards����A�K�i�ŏo���邩�ǂ�������͂��A
    ���ʂ��e�[�u��tgt_cards�Ɋi�[����B
  */
  int i,j;
  int count;
  
  clearTable(tgt_cards);
  for(i=0;i<4;i++){             //�e�X�[�g���ɑ�����
    for(j=13,count=0;j>0;j--){  //���Ԃɂ݂�
      if(my_cards[i][j]==1){    //�J�[�h������Ƃ�
	count++;                //�J�E���^��i��
      }
      else{
	count=0;                //�J�[�h���Ȃ��Ƃ����Z�b�g����
      }
      
      if(count>2){              //3���ȏ�̂Ƃ����̖������e�[�u���Ɋi�[
	tgt_cards[i][j]=count;   
      }
      else{
	tgt_cards[i][j]=0;     //���̑���0�ɂ���
      }
    }
  }
  if(g_logging==1){
    printf("make kaidan \n");
    outputTable(tgt_cards);
  }
}

void makeGroupTable(int tgt_cards[][15], int my_cards[][15]){
  /*
    �n���ꂽ�J�[�h�e�[�u��my_cards����A2���ȏ�̖����g�ŏo���邩�ǂ�������͂��A
    ���ʂ��e�[�u��tgt_cards�Ɋi�[����B
  */
  int i,j;
  int count;
  
  
  clearTable(tgt_cards);
  for(i=0;i<15;i++){  //���ꂻ��̋����̃J�[�h�̖����𐔂�
    count=my_cards[0][i]+my_cards[1][i]+my_cards[2][i]+my_cards[3][i];
    if(count>1){      //������2���ȏ�̂Ƃ�
      for(j=0;j<4;j++){
		if(my_cards[j][i]==1){    //�J�[�h�������Ă��镔����
			tgt_cards[j][i]=count; //���̖������i�[
		}
      }
    }
  }
  if(g_logging==1){
    printf("make group \n");
    outputTable(tgt_cards);
  }
}

void makeJGroupTable(int tgt_cards[][15], int my_cards[][15]){
  /*
    �n���ꂽ�J�[�h�e�[�u��my_cards����A
    �W���[�J�[���l����2���ȏ�̖����g�ŏo���邩�ǂ�������͂��A
    ���ʂ��e�[�u��tgt_cards�Ɋi�[����B
  */
  int i,j;
  int count;
 
  clearTable(tgt_cards);
  if(state.joker!=0){ 
    for(i=0;i<14;i++){ //���ꂻ��̋����̃J�[�h�̖����𐔂� �W���[�J�[�̕���������
      count=my_cards[0][i]+my_cards[1][i]+my_cards[2][i]+my_cards[3][i]+1;
      if(count>1){     //������2���ȏ�̂Ƃ�
	for(j=0;j<4;j++){
	  if(my_cards[j][i]==1){   //�J�[�h�������Ă��镔����
	    tgt_cards[j][i]=count; //���̖������i�[
	  }
	}
      }
    }
  }
  if(g_logging==1){
    printf("make Joker group \n");
    outputTable(tgt_cards);
  }
}

void lowCards(int out_cards[8][15],int my_cards[8][15],int threshold){
  /*
    �n���ꂽ�J�[�h�e�[�u��my_cards�̃J�[�h������
    threshold�ȏ�̕�����0�ł���,threshold���Ⴂ�������̂����A
    �J�[�h�e�[�u��out_cards�Ɋi�[����B
  */
  int i;
  copyTable(out_cards,my_cards); //my_cards���R�s�[����
  for(i=threshold;i<15;i++){    //threshold����15�܂�
    out_cards[0][i]=0;          //0�ł��߂�
    out_cards[1][i]=0;
    out_cards[2][i]=0;
    out_cards[3][i]=0;
  }
}

void highCards(int out_cards[8][15],int my_cards[8][15],int threshold){
  /*
    �n���ꂽ�J�[�h�e�[�u��my_cards�̃J�[�h������
    threshold�ȉ��̕�����0�ł���,threshold��荂���������̂���
    �J�[�h�e�[�u��out_cards�Ɋi�[����
  */
  int i;
  copyTable(out_cards,my_cards); //my_cards���R�s�[����
  for(i=0;i<=threshold;i++){    //0����threshold�܂�
    out_cards[0][i]=0;          //0�ł��߂�
    out_cards[1][i]=0;
    out_cards[2][i]=0;
    out_cards[3][i]=0;
  } 
}

int nCards(int n_cards[8][15],int target[8][15],int n){
  /*
    n���̃y�A���邢�͊K�i�݂̂�n_cards �ɂ̂����B���̂Ƃ��e�[�u���ɂ̂鐔����n�̂݁B
    �J�[�h�������Ƃ���0,����Ƃ���1���������B
  */
  int i,j,flag=0;         
  clearTable(n_cards);          //�e�[�u�����N���A
  for(i=0;i<4;i++)           
    for(j=0;j<15;j++)           //�e�[�u���S�̂𑖍���
      if(target[i][j]==(int)n){ //n�ƂȂ���̂��݂����Ƃ�
      	n_cards[i][j]=n;
	flag=1;                 //�t���O������
      }else{                    //n�ȊO�̏ꏊ��
	n_cards[i][j]=0;        //0�Ŗ��߂�B
      }
  return flag;
}

void lockCards(int target_cards[8][15],int suit[5]){
  /*
    ���ϐ�state.suit�̂P�������Ă���X�[�g�̂݃J�[�h�e�[�u��target_cards�Ɏc���B
  */
  int i,j;
  for(i=0;i<4;i++)
    for(j=0;j<15;j++)
      target_cards[i][j]*=suit[i]; //suit[i]==1 �̂Ƃ��͂��̂܂�,==0�̂Ƃ�0�ł���B
}

void lowGroup(int out_cards[8][15],int my_cards[8][15],int group[8][15]) {
  /*
    �n���ꂽ�����g�ŏo����J�[�h�̏����̂���group�ƃJ�[�h�e�[�u��my_cards����
    �ł��Ⴂ�����g��T���A��������J�[�h�e�[�u��out_cards�ɂ��̃J�[�h���ڂ���B
  */
  int i, j;					//�J�E���^
  int count = 0,qty=0;				//�J�[�h�̖���,����
  
  clearTable(out_cards);
  for(j=1; j<14; j++) {	                        //�����N���Ⴂ���ɒT������ 
    for(i=0; i<4; i++) {
      if(group[i][j] >1 ) {	        	//group�e�[�u����2�ȏ�̐����𔭌�������
	out_cards[i][j] = 1;			//out_cards�Ƀt���O�𗧂Ă�
	count++;
	qty=group[i][j];
      }
    }
    if(count >0) break;			//���[�v�E�o�p�t���O�������Ă�����
  }
  
  for(i=0; count<qty; i++) {
    if(my_cards[i][j] == 0 && (state.lock==0||state.suit[i]==1)){	
      out_cards[i][j] = 2;		//�W���[�J�[�p�t���O�𗧂Ă� 
      count++;
    }
  }
}

void highGroup(int out_cards[8][15],int my_cards[8][15],int group[8][15]) {
  /*
    �n���ꂽ�����g�ŏo����J�[�h�̏����̂���group�ƃJ�[�h�e�[�u��my_cards����
    �ł����������g��T���A��������J�[�h�e�[�u��out_cards�ɂ��̃J�[�h���ڂ���B
  */
  int i, j;					//�J�E���^
  int count = 0,qty=0;				//�J�[�h�̖���,����
  
  clearTable(out_cards);
  for(j=13; j>0; j--) {	                        //�����N���Ⴂ���ɒT������ 
    for(i=0; i<4; i++) {
      if(group[i][j] > 1) {	        	//group�e�[�u����2�ȏ�̐����𔭌�������
	out_cards[i][j] = 1;			//out_cards�Ƀt���O�𗧂Ă�
	count++;
	qty=group[i][j];
      }
    }
    if(count >0) break;			//���[�v�E�o�p�t���O�������Ă�����
  }
  
  for(i=0; count<qty; i++) {
    if(my_cards[i][j] == 0 && (state.lock==0||state.suit[i]==1)){
      out_cards[i][j] = 2;		//�W���[�J�[�p�t���O�𗧂Ă� 
      count++;
    }
  }
}

void lowSequence(int out_cards[8][15],int my_cards[8][15],int sequence[8][15]){
  /*
    �n���ꂽ�K�i�ŏo����J�[�h�̏����̂���group�ƃJ�[�h�e�[�u��my_cards����
    �ł��Ⴂ�K�i��T���A��������J�[�h�e�[�u��out_cards�ɂ��̃J�[�h���ڂ���B
  */
  int i,j,lowvalue,lowline=0,lowcolumn=0;
  
  lowvalue = 0;
  
  clearTable(out_cards);
  i = 0;
  
  //lowsequence�̔���
  while((i < 15) && (lowvalue == 0)){ //�K�i�e�[�u�����ɊK�i��������܂ŌJ��Ԃ�
    j = 0;
    while(j < 4){
      if(sequence[j][i] != 0){      //�Ⴂ�������璲��,�K�i�e�[�u����0�ȊO�������番��
	if(sequence[j][i] > lowvalue){ //�����������N�_�Ƃ��č����K�i�̒��ōŒ����ۂ�
	  lowvalue = sequence[j][i];   //�Œ���������l�Əꏊ��ۑ�
	  lowline = j;
	  lowcolumn = i;
	}
      }
      j++;
    }
    if(lowvalue == 0){
      i++;
    }
  }
  
  //out_cards�ւ̏��o��
  if(lowvalue != 0){              //�K�i��������Ȃ�������out_cards�ɂ͏��o���Ȃ�
    for(i = lowcolumn; i < (lowcolumn+lowvalue); i++){
      if(my_cards[lowline][i] == 1){
	out_cards[lowline][i] = 1;   //���ʂ̎�D�Ƃ��Ď����Ă�����1�𗧂Ă�
      }
      else{
	out_cards[lowline][i] = 2;        //�����Ă��Ȃ�������W���[�J�[�Ȃ̂�2�𗧂Ă�
      }
    }
  }
}

void highSequence (int out_cards[8][15],int my_cards[8][15],int sequence[8][15]){
  /*
    �n���ꂽ�K�i�ŏo����J�[�h�̏����̂���group�ƃJ�[�h�e�[�u��my_cards����
    �ł������K�i��T���A��������J�[�h�e�[�u��out_cards�ɂ��̃J�[�h���ڂ���B
  */
  int i,j,k,highvalue,highline=0,highcolumn=0,prevalue;
  highvalue = 0;

  clearTable(out_cards);
  i = 14;
  
  //highsequence�̔���
  while((i > 0) && (highvalue == 0)){  //�K�i�e�[�u�����ɊK�i��������܂ŌJ��Ԃ�
    j = 0;
    while(j < 4){
      k = -1;
      if((sequence[j][i] != 0) && (my_cards[j][i] != 0)){//�����������璲��,�K�i�e�[�u����0�ȊO�������番��
	do{                       //�������K�i�̍ō��l����,�Œ��̊K�i��T��
	  if(sequence[j][i-k] >= highvalue){ //�����ō��l�����K�i�̒��ōŒ����ۂ�
	    highvalue = sequence[j][i-k];  //�Œ���������L�^
	    highline = j;
	    highcolumn = i-k;
	  }
	  prevalue = sequence[j][i-k];
	  k++;
	}while(prevalue <= sequence[j][i-k]);
	
      }	
      j++;	
    }
    if(highvalue == 0){
      i--;
    }
  }

  //out_cards�ւ̏��o��
  for(i = highcolumn; i < (highcolumn+highvalue); i++){
    if(my_cards[highline][i] == 1){
      out_cards[highline][i] = 1; //���ʂ̎�D�Ƃ��Ď����Ă�����1�𗧂Ă�
    }
    else{
      out_cards[highline][i] = 2;     //�����Ă��Ȃ�������W���[�J�[�Ȃ̂�2�𗧂Ă�
    }
  } 
}

//my_cards(��D)����y�A,�K�i���̖��̃J�[�h�������������̂�out_card�Ɋi�[���� 
void removeGroup(int out_cards[8][15],int my_cards[8][15],int group[8][15]){
  /*
    �n���ꂽ�����g�ŏo����J�[�h�̏����̂���group�ƃJ�[�h�e�[�u��my_cards����
    �����g�ȊO�̃J�[�h��T���A�J�[�h�e�[�u��out_cards�ɂ��̃J�[�h���ڂ���B
  */
  int i,j;
  
  for (i = 0; i < 15; i++){
    for(j = 0; j < 4; j++){
      if((my_cards[j][i] == 1) && (group[j][i] == 0)){
	out_cards[j][i] = 1;         //mycards�ɑ��݂�,�����e�[�u���ɂȂ��ꍇ1
      }
      else{
	out_cards[j][i] = 0;             //����ȊO(mycards�ɂȂ���,���e�[�u���ɂ���)�̏ꍇ0
      }
    }
  }
}

void removeSequence(int out_cards[8][15],int my_cards[8][15],int sequence[8][15]){
  /*
    �n���ꂽ�K�i�ŏo����J�[�h�̏����̂���group�ƃJ�[�h�e�[�u��my_cards����
    �K�i�ȊO�̃J�[�h��T���A�J�[�h�e�[�u��out_cards�ɂ��̃J�[�h���ڂ���B
  */
  int i,j,k;
  
  for(j = 0; j < 4; j++){
    for (i = 0; i < 15; i++){
      if((my_cards[j][i] == 1) && (sequence[j][i] == 0)){
	out_cards[j][i] = 1;           //mycards�ɑ��݂�,�����e�[�u���ɂȂ��ꍇ1
      }else if(sequence[j][i] > 2){
      	for(k=0;k < sequence[j][i];k++){
      	  out_cards[j][i+k] = 0;
      	}
      	i += k-1;
      } 
      else {
	out_cards[j][i] = 0;      //����ȊO(mycards�ɂȂ���,���e�[�u���ɂ���)�̏ꍇ0
      }
    }
  }
}

void lowSolo(int out_cards[8][15],int my_cards[8][15],int joker_flag){
  /*
    �Ⴂ������T����,�ŏ��Ɍ������J�[�h���ꖇout_cards�ɂ̂���B
    joker_flag��1�̂Ƃ�,�J�[�h��������Ȃ����,joker���ꖇout_cards�ɂ̂���B
  */
  int i,j,find_flag=0;

  clearTable(out_cards);                  //�e�[�u�����N���A
  for(j=1;j<14&&find_flag==0;j++){        //�Ⴂ�����炳����
    for(i=0;i<4&&find_flag==0;i++){
      if(my_cards[i][j]==1){              //�J�[�h����������               
	find_flag=1;                      //�t���O�𗧂�
	out_cards[i][j]=my_cards[i][j];   //out_cards�ɂ̂�,���[�v�𔲂���B
      }
    }
  }
  if(find_flag==0&&joker_flag==1){       //������Ȃ������Ƃ�
    out_cards[0][15]=2;                  //�W���[�J�[���̂���
  }
}

void highSolo(int out_cards[8][15],int my_cards[8][15],int joker_flag){
  /*
    ����������T����,�ŏ��Ɍ������J�[�h���ꖇout_cards�ɂ̂���B
    joker_flag������Ƃ�,�J�[�h��������Ȃ����,joker���ꖇout_cards�ɂ̂���B
  */
  int i,j,find_flag=0;
  
  clearTable(out_cards);                 //�e�[�u�����N���A
  for(j=13;j>0&&find_flag==0;j--){       //���������炳����
    for(i=0;i<4&&find_flag==0;i++){
      if(my_cards[i][j]==1){              //�J�[�h����������
	find_flag=1;                      //�t���O�𗧂�
	out_cards[i][j]=my_cards[i][j];   //out_cards�ɂ̂�,���[�v�𔲂���B
      }
    }
  }
  if(find_flag==0&&joker_flag==1){       //������Ȃ������Ƃ�
    out_cards[0][0]=2;                   //�W���[�J�[���̂���
  }
}

void change(int out_cards[8][15],int my_cards[8][15],int num_of_change){
  /*
    �J�[�h�������̃A���S���Y��
    ��x�����邢�͕x�����A��n�����邢�͕n���ɃJ�[�h��n�����̃J�[�h��
    �J�[�h�e�[�u��my_cards�ƌ�������num_of_change�ɉ����āA
    �Ⴂ�ق�����I�уJ�[�h�e�[�u��out_cards�ɂ̂���
  */
  int count=0;
  int one_card[8][15];
  
  clearTable(out_cards);
  while(count<num_of_change){
    lowSolo(one_card,my_cards,0);
    cardsDiff(my_cards,one_card);
    cardsOr(out_cards,one_card);
    count++;
  }
}

void lead(int out_cards[8][15],int my_cards[8][15]){
  /*
    �V�����J�[�h���o����Ƃ��̑I�����[�`��
    �J�[�h�e�[�u��my_cards����K�i=>�y�A=>�ꖇ�̏��Ŗ����̑����ق����瑖����,
    �Ⴂ�J�[�h����݂āA�͂��߂Č��������̂� out_cards�ɂ̂���B
  */
  int group[8][15];           //�����g�𒲂ׂ邽�߂̃e�[�u��
  int sequence[8][15];        //�K�i�𒲂ׂ邽�߂̃e�[�u��
  int temp[8][15];            //�ꎞ�g�p�p�̃e�[�u��
  int i,find_flag=0;          //��D�������������ۂ��̃t���O

  clearTable(group);
  clearTable(sequence);
  clearTable(temp);
  if(state.joker==1){                      //�W���[�J�[������Ƃ�,�W���[�J�[���l����,
    makeJGroupTable(group,my_cards);       //�K�i�Ɩ����g�����邩�𒲂�,
    makeJKaidanTable(sequence,my_cards);   //�e�[�u���Ɋi�[����
  }else{
    makeGroupTable(group,my_cards);        //�W���[�J�[���Ȃ��Ƃ��̊K�i�Ɩ����g��
    makeKaidanTable(sequence,my_cards);    //�󋵂��e�[�u���Ɋi�[����
  }

  for(i=15;i>=3&&find_flag==0;i--){         //�����̑傫��������,������܂�
    find_flag=nCards(temp,sequence,i);      //�K�i�����邩�������,
    
    if(find_flag==1){                       //���������Ƃ�
      lowSequence(out_cards,my_cards,temp); //���̂Ȃ��ōł��Ⴂ���̂�out_cards
    }                                       //�ɂ̂���         
  }
  for(i=5;i>=2&&find_flag==0;i--){          //�����̑傫��������,������܂�
    find_flag=nCards(temp,group,i);         //�����g�����邩�𒲂�,
    if(find_flag==1){                       //���������Ƃ�
      lowGroup(out_cards,my_cards,temp);    //���̂Ȃ��ōł��Ⴂ���̂�out_cards
    }                                       //�̂���
  }
  if(find_flag==0){                          //�܂�������Ȃ��Ƃ�
    lowSolo(out_cards,my_cards,state.joker); //�ł��Ⴂ�J�[�h��out_cards�ɂ̂���B
  }
}

void leadRev(int out_cards[8][15],int my_cards[8][15]){
  /*
    �v�����p�̐V�����J�[�h���o����Ƃ��̑I�����[�`��
    �J�[�h�e�[�u��my_cards����K�i=>�y�A=>�ꖇ�̏��Ŗ����̑����ق����瑖����,
    �����J�[�h����݂āA�͂��߂Č��������̂� out_cards�ɂ̂���B
  */
  int group[8][15];           //�����g�𒲂ׂ邽�߂̃e�[�u��
  int sequence[8][15];        //�K�i�𒲂ׂ邽�߂̃e�[�u��
  int temp[8][15];            //�ꎞ�g�p�p�̃e�[�u��
  int i,find_flag=0;          //��D�������������ۂ��̃t���O
  //clearTable(group);
  //clearTable(sequence);
  //clearTable(temp);
  if(state.joker==1){                        //�W���[�J�[������Ƃ�,�W���[�J�[���l����,
    makeJGroupTable(group,my_cards);         //�K�i�Ɩ����g�����邩�𒲂�,
    makeJKaidanTable(sequence,my_cards);     //�e�[�u���Ɋi�[����
  }else{
    makeGroupTable(group,my_cards);          //�W���[�J�[���Ȃ��Ƃ��̊K�i�Ɩ����g��
    makeKaidanTable(sequence,my_cards);      //�󋵂��e�[�u���Ɋi�[����
  }		
  for(i=15;i>=3&&find_flag==0;i--){          //�����̑傫��������,������܂�
    
    find_flag=nCards(temp,sequence,i);       //�K�i�����邩�������,
    
    if(find_flag==1){                        //���������Ƃ�
      highSequence(out_cards,my_cards,temp); //���̂Ȃ��ōł��������̂�out_cards
    }                                        //�ɂ̂���
  }
  for(i=5;i>=2&&find_flag==0;i--){           //�����̑傫��������,������܂�
    find_flag=nCards(temp,group,i);          //�����g�����邩�𒲂�,
    if(find_flag==1){                        //���������Ƃ�
      highGroup(out_cards,my_cards,temp);    //���̂Ȃ��ōł��������̂�out_cards
    }                                        //�ɂ̂���       
  }
  if(find_flag==0){                          //�܂�������Ȃ��Ƃ�
    highSolo(out_cards,my_cards,state.joker);//�ł������J�[�h��out_cards�ɂ̂���
  }
}
 
void followSolo(int out_cards[8][15],int my_cards[8][15],int joker_flag){
  /*
    ���̃v���[���[�ɑ����ăJ�[�h���ꖇ�ŏo���Ƃ��̃��[�`��
    joker_flag��1�̎��W���[�J�[���g�����Ƃ���
    ��o����J�[�h�̓J�[�h�e�[�u��out_cards�Ɋi�[�����
  */
  int group[8][15];       //�����g�𒲂ׂ邽�߂̃e�[�u��
  int sequence[8][15];    //�K�i�𒲂ׂ邽�߂̃e�[�u��
  int temp[8][15];        //�ꎞ�g�p�p�̃e�[�u��
  
  makeGroupTable(group,my_cards);           //�����g�������o��
  makeKaidanTable(sequence,my_cards);       //�K�i�������o��
  
  removeSequence(temp,my_cards,sequence);   // �K�i������
  removeGroup(out_cards,temp,group);        // �����g������
  
  highCards(temp,out_cards,state.ord);      // ��̃J�[�h���ア�J�[�h������

  if(state.lock==1){               
    lockCards(temp,state.suit);             //���b�N����Ă���Ƃ��o���Ȃ��J�[�h������
  }
  lowSolo(out_cards,temp,state.joker);      //�c�����J�[�h����ア�J�[�h�𔲂��o��
}

void followGroup(int out_cards[8][15],int my_cards[8][15],int joker_flag){
  /*  
    ���̃v���[���[�ɑ����ăJ�[�h�𖇐��g�ŏo���Ƃ��̃��[�`��
    joker_flag��1�̎��W���[�J�[���g�����Ƃ���
    ��o����J�[�h�̓J�[�h�e�[�u��out_cards�Ɋi�[�����
  */
  int group[8][15];
  int ngroup[8][15];
  int temp[8][15];
  
  highCards(temp,my_cards,state.ord);          //���苭���J�[�h���c�� 
  if(state.lock==1){                           //���b�N����Ă���Ƃ�
    lockCards(temp,state.suit);                //�o���Ȃ��J�[�h������
  }
  makeGroupTable(group,temp);                  //�c�������̂��疇���g�������o��
  if(nCards(ngroup,group,state.qty)==0&&state.joker==1){
    //��Ɠ��������̑g�������Ƃ��W���[�J�[���g���ĒT��
    makeJGroupTable(group,temp);               
    nCards(ngroup,group,state.qty);     //��Ɠ��������̑g�݂̂̂����B 
  }
  lowGroup(out_cards,my_cards,ngroup);  //��Ԏア�g�𔲂��o��
}

void followSequence(int out_cards[8][15],int my_cards[8][15],int joker_flag){
  /*
    ���̃v���[���[�ɑ����ăJ�[�h���K�i�ŏo���Ƃ��̃��[�`��
    joker_flag��1�̎��W���[�J�[���g�����Ƃ���
    ��o����J�[�h�̓J�[�h�e�[�u��out_cards�Ɋi�[�����
  */
  int seq[8][15];
  int nseq[8][15];
  int temp[8][15];
  
  highCards(temp,my_cards,state.ord);          //���苭���J�[�h���c��
  if(state.lock==1){                           //���b�N����Ă���Ƃ�
    lockCards(temp,state.suit);                //�o���Ȃ��J�[�h������
  }
  makeKaidanTable(seq,temp);                   //�K�i�������o��
  if(nCards(nseq,seq,state.qty)==0&&state.joker==1){
    //��Ɠ��������̊K�i�������Ƃ��W���[�J�[���g���ĒT��
    makeJKaidanTable(seq,temp);
    nCards(nseq,seq,state.qty);          //��Ɠ��������̑g�݂̂̂����B
  }
  lowSequence(out_cards,my_cards,nseq);  //��Ԏア�K�i��
}

void followSoloRev(int out_cards[8][15],int my_cards[8][15],int joker_flag){
  /*
    �v����Ԃ̂Ƃ��ɑ��̃v���[���[�ɑ����ăJ�[�h���ꖇ�ŏo���Ƃ��̃��[�`��
    joker_flag��1�̎��W���[�J�[���g�����Ƃ���
    ��o����J�[�h�̓J�[�h�e�[�u��out_cards�Ɋi�[�����
  */
  int group[8][15];
  int sequence[8][15];
  int temp[8][15];
  
  makeGroupTable(group,my_cards);            //�����g�������o��
  makeKaidanTable(sequence,my_cards);        //�K�i�������o��
  
  removeSequence(temp,my_cards,sequence);    // �K�i������
  removeGroup(out_cards,temp,group);         // �����g������
  lowCards(temp,out_cards,state.ord);        // ��̃J�[�h��苭���J�[�h������
  if(state.lock==1){
    lockCards(temp,state.suit);          //���b�N����Ă���Ƃ��o���Ȃ��J�[�h������
  }
  highSolo(out_cards,temp,state.joker);  //�c�����J�[�h���狭���J�[�h�𔲂��o��
}

void followGroupRev(int out_cards[8][15],int my_cards[8][15],int joker_flag){
  /*
    �v����Ԃ̂Ƃ��ɑ��̃v���[���[�ɑ����ăJ�[�h�𖇐��g�ŏo���Ƃ��̃��[�`��
    joker_flag��1�̎��W���[�J�[���g�����Ƃ���
    ��o����J�[�h�̓J�[�h�e�[�u��out_cards�Ɋi�[�����
  */
  int group[8][15];
  int ngroup[8][15];
  int temp[8][15];
  
  lowCards(temp,my_cards,state.ord);          //����ア�J�[�h���c��
  if(state.lock==1){                          //���b�N����Ă���Ƃ�
    lockCards(temp,state.suit);               //�o���Ȃ��J�[�h������
  }
  makeGroupTable(group,temp);                 //�����g�������o��
  if(nCards(ngroup,group,state.qty)==0&&state.joker==1){
    //��Ɠ��������̑g�������Ƃ��W���[�J�[���g���ĒT��
    makeJGroupTable(group,temp);
    nCards(ngroup,group,state.qty);      //��Ɠ��������̑g�݂̂̂����B
  }
  highGroup(out_cards,my_cards,ngroup);  //�c�������̂����ԋ����g�𔲂��o��
}

void followSequenceRev(int out_cards[8][15],int my_cards[8][15],int joker_flag){
  /*
    �v����Ԃ̂Ƃ��ɑ��̃v���[���[�ɑ����ăJ�[�h���K�i�ŏo���Ƃ��̃��[�`��
    joker_flag��1�̎��W���[�J�[���g�����Ƃ���
    ��o����J�[�h�̓J�[�h�e�[�u��out_cards�Ɋi�[�����
  */
  int seq[8][15];
  int nseq[8][15];
  int temp[8][15];
  
  lowCards(temp,my_cards,state.ord);          //����ア�J�[�h���c��
  if(state.lock==1){                          //���b�N����Ă���Ƃ�
    lockCards(temp,state.suit);               //�o���Ȃ��J�[�h������
  }
  makeKaidanTable(seq,temp);                  //�K�i�������o��
  if(nCards(nseq,seq,state.qty)==0&&state.joker==1){
    //��Ɠ��������̊K�i�������Ƃ��W���[�J�[���g���ĒT��
    makeJKaidanTable(seq,temp);
    nCards(nseq,seq,state.qty);          //��Ɠ��������̊K�i�݂̂̂����B
  } 
  highSequence(out_cards,my_cards,nseq); //�c�������̂����ԋ����g�𔲂��o��
}

void follow(int out_cards[8][15],int my_cards[8][15]){
  /*
    ���̃v���[���[�ɑ����ăJ�[�h���o���Ƃ��̃��[�`��
    ��̏��state�ɉ����Ĉꖇ�A�����g�A�K�i�̏ꍇ�ɕ�����
    �Ή������֐����Ăяo��
    ��o����J�[�h�̓J�[�h�e�[�u��out_cards�Ɋi�[�����
  */
  clearTable(out_cards);
  if(state.qty==1){
    followSolo(out_cards,my_cards,state.joker);    //�ꖇ�̂Ƃ�
  }else{
    if(state.sequence==0){
      followGroup(out_cards,my_cards,state.joker);  //�����g�̂Ƃ�
    }else{
      followSequence(out_cards,my_cards,state.joker); //�K�i�̂Ƃ�
    }
  }
}

void followRev(int out_cards[8][15],int my_cards[8][15]){
  /*
    ���̃v���[���[�ɑ����ăJ�[�h���o���Ƃ��̃��[�`��
    ��̏��state�ɉ����Ĉꖇ�A�����g�A�K�i�̏ꍇ�ɕ�����
    �Ή������֐����Ăяo��
    ��o����J�[�h�̓J�[�h�e�[�u��out_cards�Ɋi�[�����
  */
  clearTable(out_cards);
  if(state.qty==1){
    followSoloRev(out_cards,my_cards,state.joker);    //�ꖇ�̂Ƃ�
  }else{
    if(state.sequence==0){
      followGroupRev(out_cards,my_cards,state.joker);  //�����g�̂Ƃ�
    }else{
      followSequenceRev(out_cards,my_cards,state.joker); //�K�i�̂Ƃ�
    }
  }
}

int cmpCards(int cards1[8][15],int  cards2[8][15]){
  /*
    �J�[�h�e�[�u��cards1�Acards2�̃J�[�h�������r���A
    �قȂ��Ă����1�A��v���Ă����0��Ԃ�
  */
  int i,j,flag=0;
  
  for(i=0;i<5;i++)
    for(j=0;j<15;j++)
      if(cards1[i][j]!=cards2[i][j])
	flag=1;
  
  return flag;
}

int cmpState(struct state_type* state1,struct state_type* state2){
  /*
    ��Ԃ��i�[����state1��state2���r���A��v�����0���A
    �قȂ��Ă���΂���ȊO��Ԃ�
  */
  int i,flag=0;
  if(state1->ord != state2->ord) flag+=1;
  if(state1->qty != state2->qty) flag+=2;
  if(state1->sequence != state2->sequence) flag+=4;
  for(i=0;i<5;i++)
    if(state1->suit[i]!=state2->suit[i]) flag+=8;
  if(state1->onset != state2->onset) flag+=16;
  return flag;
}

int getLastPlayerNum(int ba_cards[8][15]){
  /*
    �Ō�p�X�ȊO�̃J�[�h��o�������v���[���[�̔ԍ���Ԃ��B
    ���̊֐��𐳏�ɓ��삳���邽�߂ɂ́A
    �T�[�o�����ɏo���J�[�h�����炤�x��
    ���̊֐����Ăяo���K�v������B
  */
  static struct state_type last_state;
  static int last_player_num=-1;
  
  if(g_logging==1){  //���O�̕\��
    printf("Now state \n");
    showState(&state);
    printf(" Last state \n");
    showState(&last_state);
  }
  
  if(cmpState(&last_state,&state)!=0){ //��̏�Ԃɕω����N������
    last_player_num =ba_cards[5][3];   //�Ō�̃v���[����
    last_state=state;                  //�ŐV�̏�Ԃ��X�V����
  }
  
  if(g_logging==1){ //���O�̕\��
    printf("last player num : %d\n",last_player_num);
  }
  
  return last_player_num;
}

//----------------------------------------------------------------------------------
//--------------------------------�������玩��̊֐�--------------------------------
//----------------------------------------------------------------------------------

int setmake(int out_cards[][15],int my_cards[8][15],int joker_flag){
//�ł��邾���g�������Ȃ��ł���悤�K�i�����A�K�i���i�[�������̂ƁA���v�̑g����Ԃ�
//out_cards[0]~[3]�ɂ͊K�i�ŏo����ꍇ�͉E�[�̏ꏊ�ɊK�i�̖���������A�K�i�ȊO�̃J�[�h��1������
//out_cards[4]�ɂ͊K�i�Ɏg�p���Ă��Ȃ��J�[�h�̖���������
  int tgt_cards[8][15];//�e�[�u��
  int i,j,k;
  int count=0;
  //out_cards[4][14]=0;
  clearTable(out_cards);
  clearTable(tgt_cards);
  for(i=0;i<4;i++){
    for(j=1;j<=13;j++){
      tgt_cards[5][j]+=my_cards[i][j];//�e�����̖������L�^
    }
  }
  for(i=0;i<4;i++){             //�e�X�[�g���ɑ�����
    for(j=1;j<=12;j++){         //A�܂ŏ��Ԃɂ݂�
      if(my_cards[i][j]==1){    //�J�[�h������Ƃ�
	count++;                //�J�E���^��i��
      }
      else{
	count=0;                //�J�[�h���Ȃ��Ƃ����Z�b�g����
      }
      if(count>2){              //3���ȏ�̂Ƃ����̖������e�[�u���Ɋi�[
	tgt_cards[i][j]=count;
      }
      else{
	tgt_cards[i][j]=0;      //���̑���0�ɂ���
      }
    }
    count=0;
  }
  count=0;
  //�K�i���ł����ꍇ
  //�Ⴆ�΃X�y�[�h5�`7�̏ꍇtgt_cards��[0][5]��3,[0][3],[0][4]��0�ƂȂ��Ă���
  //�X�y�[�h5�`8�̏ꍇtgt_cards��[0][6]��4,[0][5]��3,[0][3],[0][4]��0�ƂȂ��Ă���
  for(i=0;i<4;i++){
    for(j=13;j>0;j--){
      if(tgt_cards[i][j]>=3){//�K�i�𔭌������Ƃ�
        count=0;
        for(k=0;k<tgt_cards[i][j];k++){//���̊K�i�̐������݂�
          if(tgt_cards[5][j-k]>=2 && j!=6){//8�̃J�[�h�ȊO�ŊK�i�ȊO�ɂ����̐����������
            count++;//�J�E���^��i�߂�
          }
        }
        if(j>=6 && j-tgt_cards[i][j]<6){//8���܂܂�Ă���ꍇ
          if(count+1==tgt_cards[i][j]){//8�̃J�[�h�ȊO�̊K�i�̐��������ׂăy�A�ƂȂ�ꍇ�A�K�i�����
             out_cards[i][j]=my_cards[i][j];//�J�[�h���c��
          }
          else{
            out_cards[i][j]=tgt_cards[i][j];//�K�i���c��(���)
            for(k=0;k<tgt_cards[i][j];k++){
              tgt_cards[5][j-k]-=1;//���̊K�i�̐����̖��������炷
            }
            j-=k;//�K�i�̍��[��1���Ɉړ�
          }
        }
        else{//8���܂܂�ĂȂ��ꍇ
          if(count==tgt_cards[i][j]){//�K�i�̐��������ׂăy�A�ƂȂ�ꍇ�A�K�i�����
            out_cards[i][j]=my_cards[i][j];//�J�[�h���c��
          }
          else{
            out_cards[i][j]=tgt_cards[i][j];//�K�i���c��(���)
            for(k=0;k<tgt_cards[i][j];k++){
              tgt_cards[5][j-k]-=1;//���̊K�i�̐����̖��������炷
            }
            j-=k;//�K�i�̍��[��1���Ɉړ�
          }
        }
      }//�K�i�𔭌������Ƃ��̔��肱���܂�
      else{//�K�i�łȂ�
        out_cards[i][j]=my_cards[i][j];//�J�[�h���c��
      }
    }
  }
  count=0;
  //out_cards�ɊK�i�łȂ�������1,�K�i�̕����̉E�[�ɂ��̖����̐���������
  //�X�y�[�h6�`8�̏ꍇout_cards��[0][6]��3,[0][4],[0][5]��0�ƂȂ��Ă���
  //�X�y�[�h6�`9�̏ꍇout_cards��[0][7]��4,[0][4],[0][5],[0][6]��0�ƂȂ��Ă���

  //�W���[�J�[������Ƃ�,�K�i��{��,����
  clearCards(tgt_cards);         //�J�[�h�������Z�b�g
  k=0;
  if(joker_flag==1){             //joker������Ƃ�
    for(i=0;i<4;i++){            //�e�X�[�g���ɑ�����
      for(j=1;j<=12;j++){        //A�܂ŏ��Ԃɂ݂�
	if(my_cards[i][j]==1 && (tgt_cards[5][j]==1 || j==6)){//�J�[�h������A�P�̂܂���8�̃J�[�h�̂Ƃ�
	  count++;               //�J�E���^��i�߂�
	}
	else if(k==0 && count!=0 && my_cards[i][j]!=1){//�W���[�J�[���g�p�ŃJ�[�h���Ȃ��Ƃ�
	  count++;               //�J�E���^��i�߂�
	  k=j;                   //�W���[�J�[�̏ꏊ���L�^
	}
	else{//�T����A�K�i���ł��邩����
          if(count>2){           //3���ȏ�̂Ƃ��K�i�����,���[�v���甲����
            out_cards[i][j-1]=count;//�E�[�ɖ����i�[
            if(k!=j-1){//�W���[�J�[�̈ʒu=�K�i�̉E�[(2�ɋ߂���)�łȂ��Ƃ�
              tgt_cards[5][j-1]-=1;//���̊K�i�̐����̖��������炷(���̔���ł͉E�[�𒲂ׂȂ�����)
            }
            for(;count>1;count--){
              out_cards[i][j-count]=0;
              if(k!=j-count){
                tgt_cards[5][j-count]-=1;//���̊K�i�̐����̖��������炷
              }
            }
            out_cards[4][14]=2;
            i=4;
            j=13;
          }
          else{//2���ȉ��Ȃ烊�Z�b�g
            count=0;
            k=0;
          }
        }
	if(j==12){//A�܂Œ��ׂĒ[�ɂ����Ƃ�
          if(count>2){//�ȍ~�A���[�v���̊K�i����Ɠ�������
            out_cards[i][j]=count;
            if(k!=j){
              tgt_cards[5][j]-=1;
            }
            for(count=count-1;count>0;count--){
              out_cards[i][j-count]=0;
              if(k!=j-count){
                tgt_cards[5][j-count]-=1;
              }
            }
            out_cards[4][14]=2;
            i=4;
            j=13;
          }
          else{
            count=0;
            k=0;
          }
        }
      }//�������[�v�����܂�
    }//�X�[�g���[�v�����܂�
  }
  //�X�y�[�h6,8�ƃW���[�J�[�ō�����ꍇout_cards��[0][6]��3,[0][4],[0][5]��0,k=5�ƂȂ��Ă���
  if(out_cards[4][14]!=2){//�W���[�J�[�ŊK�i�����Ȃ��ꍇ�A�W���[�J�[���i�[
    out_cards[4][14]=state.joker;
  }
  //�g�����v�Z
  count=0;
  for(i=0;i<4;i++){            //�e�X�[�g���ɑ�����
    for(j=1;j<=13;j++){        //���Ԃɂ݂�
      if(out_cards[i][j]>2){   //�K�i�������
        count++;               //�J�E���g
      }
    }
  }
  for(j=1;j<=13;j++){
    out_cards[4][j]=0;
    for(i=0;i<4;i++){
      if(out_cards[i][j]==1){
        out_cards[4][j]+=1;//�e�����̖������L�^
      }
    }
  }
  for(j=1;j<=13;j++){
    if(out_cards[4][j]>0){  //�K�i�ȊO�ŃJ�[�h�������
      count++;              //�J�E���g
    }
  }
  return count;
}

int kaidanhand(int select_cards[][15],int search[8][15],int own_cards[8][15]){
//�K�i��search����T���A���ʂ�select_cards�Ɋi�[����
  clearTable(select_cards);
  int i,j,k;
  for(j=3;j<=13;j++){
    for(i=0;i<4;i++){
      if(search[i][j]>=3){
        for(k=0;k<search[i][j];k++){
          if(own_cards[i][j-k]==0){
            select_cards[i][j-k]=2;
          }
          else{
            select_cards[i][j-k]=1;
          }
        }
        return j;//�J�[�h�̋�����Ԃ�
      }
    }
  }
  return 0;//�Ȃ����0��Ԃ�
}

int grouphand(int select_cards[][15],int search[8][15],int n){
//�w��̖����̖����g��T���A���ʂ�select_cards�Ɋi�[����
  clearTable(select_cards);
  int i,j;
  if(state.rev==0){
    for(j=1;j<=13;j++){
      if(n!=0){
        //�������w�肳��Ă���ꍇ
        if(search[4][j]==n){
          for(i=0;i<=3;i++){
            if(search[i][j]==1){
              select_cards[i][j]=1;
            }
          }
          return j;//�J�[�h�̋�����Ԃ�
        }
      }
      else{
        //�������w�肳��Ă��Ȃ��ꍇ
        if(search[4][j]>0){
          for(i=0;i<=3;i++){
            if(search[i][j]==1){
              select_cards[i][j]=1;
            }
          }
          return j;//�J�[�h�̋�����Ԃ�
        }
      }
    }
  }
  else{
    //�v�����������悤�ɑI��
    for(j=13;j>=1;j--){
      if(n!=0){
        if(search[4][j]==n){
          for(i=0;i<=3;i++){
            if(search[i][j]==1){
              select_cards[i][j]=1;
            }
          }
          return j;//�J�[�h�̋�����Ԃ�
        }
      }
      else{
        if(search[4][j]>0){
          for(i=0;i<=3;i++){
            if(search[i][j]==1){
              select_cards[i][j]=1;
            }
          }
          return j;//�J�[�h�̋�����Ԃ�
        }
      }
    }
  }
  return 0;//�J�[�h���Ȃ����0��Ԃ�
}

void kou_lead(int select_cards[][15],int own_cards[8][15],int max,int used_cards[8][15]){
//�V�����J�[�h���o���Ƃ��̎v�l
  int i,j,k,l,m;
  int search[8][15];
  clearTable(search);
  int pattern[13][7]={{0}};
  pat_make(pattern,own_cards,max,used_cards,state.joker);
  if(own_cards[4][1]==2){//�W���[�J�[������ꍇ
    int target_pattern[13][7]={{0}};//��D�g�̌�������
    int count[15]={0};//�J�[�h�̖��� or �W���[�J�[�y�A�𔻒f���邩(���Ȃ��ꍇ-1)
    for(i=0;i<4;i++){
      for(j=1;j<=13;j++){
        if(state.rev==0 && max<=j){
          count[j]=-1;
        }
        else if(state.rev==1 && max>=j){
          count[j]=-1;
        }
        else if(own_cards[i][j]==1){
          count[j]++;
        }
      }
    }
    for(i=0;i<4;i++){
      for(j=0;j<=14;j++){
        k=0;
        if(own_cards[i][j]==0){//�W���[�J�[���p���邩����
          if(j<=11){
            if(own_cards[i][j+1]==1 && own_cards[i][j+2]==1){//�E2�ӏ��ɃJ�[�h������
              k=1;
            }
          }
          if(j>=3){
            if(own_cards[i][j-1]==1 && own_cards[i][j-2]==1){//��2�ӏ��ɃJ�[�h������
              k=1;
            }
          }
          if(k==0 && own_cards[i][j-1]==1 && own_cards[i][j+1]==1){//���E�ɃJ�[�h������
            k=1;
          }
          if(k==0 && count[j]>=1){//���������̃J�[�h��1���ȏ�(1��̂�)
            k=2;
            count[j]=-1;
          }
        }//�W���[�J�[���p���邩���肱���܂�
        if(k>=1){//���肷��ꍇ,����O��pattern�Ɣ�r���ėǂ����ǂ������f
          //�ϊ�
          own_cards[i][j]=1;
          pat_make(target_pattern,own_cards,max,used_cards,0);
          own_cards[i][j]=0;
          //������
          if(target_pattern[11][3]>=STRONG){//�������̏ꍇ
            if(target_pattern[11][3]>pattern[11][3]){//2min������
              k=3;
            }
          }
          else if(target_pattern[11][0]<pattern[11][0] && k==1){//�g������
            k=3;
          }
          else if(target_pattern[11][0]==pattern[11][0] && target_pattern[11][4]<pattern[11][4] && k==1){//�g��������z���������Ȃ�
            k=3;
          }
        }
        for(l=0;l<13;l++){
          for(m=0;m<7;m++){
            if(k==3){
              pattern[l][m]=target_pattern[l][m];
            }
            target_pattern[l][m]=0;
          }
        }
        if(pattern[11][3]>=100){//�m���ɏ��Ă�ꍇ�͔���I��
          i=4;
          j=15;
        }
      }//j���[�v
    }//i���[�v
  }//�W���[�J�[�̎g�p�ʒu���肱���܂�
  for(i=0;i<pattern[11][0];i++){//�v���肪���邩�ǂ���
    if(pattern[i][1]>=5 || (pattern[i][1]==4 && (pattern[i][3]==0 || pattern[i][3]==14) ) ){
      pattern[11][1]=1;
    }
  }
  if(pattern[11][2]==-1){
    //�����\�z�O�̓��삪�N�����ꍇ,default�̊֐����g��
    if(state.rev==0){
      lead(select_cards,own_cards);    //�ʏ펞�̒�o�p
    }else{
      leadRev(select_cards,own_cards); //�v�����̒�o�p
    }
  }
  else{
    if(pattern[11][1]==0){//�v���肪�Ȃ��Ƃ�,�܂��͂����蔻��̎��͒ʏ�̔���Ɉڂ�
      //�������D�Ɣ��肵�Ă��Ȃ��ꍇ
      j=0;
      if(pattern[11][3]<STRONG && pattern[11][0]<=4){
        for(i=0;i<pattern[11][0];i++){
          if(pattern[i][1]>=3){
            j++;
          }
        }
      }
      if(j!=0 && pattern[11][0]-j<=3){//�g�̐�-�K�i�̑g�̐�<=3�ł���ΊK�i�D��
        for(i=0;i<pattern[11][0];i++){
          if(pattern[i][1]>=3){
            if(state.rev==0 && pattern[i][0]+2<max){
              pattern[i][6]=90;
            }
            else if(state.rev==1 && pattern[i][0]-2>max){
              pattern[i][6]=90;
            }
          }
        }
      }
      k=2;//�ʏ픻��Ɉڂ�
    }
    else if(pattern[11][0]<=2){
      k=1;//�g��2�ȉ��̂Ƃ��͊v�����N����
    }
    else{
      j=1;
      for(i=0;i<pattern[11][0];i++){
        if(pattern[i][4]<=100 && pattern[i][3]!=14 && pattern[i][1]<=2 && j<pattern[i][4]){
          j=pattern[i][4];//j=2���ȉ��̑g��x���ő�̂��̂��L�^
        }
      }
      if(j<=90 && pattern[11][4]>=pattern[11][5]){
        k=1;//�����J�[�h�Ȃ��A�v�����̂ق�������
      }
      else if(j<=90 && pattern[11][4]<pattern[11][5]){
        k=2;//�����J�[�h�Ȃ��A�v�����̂ق����ア
      }
      else if(j>=91 && pattern[11][4]>=pattern[11][5]){
        k=3;//�����J�[�h����A�v�����̂ق�������
      }
      else{
        k=4;//�����J�[�h����A�v�����̂ق����ア
      }
    }
    if(k==1){
      //�v�����N����
      for(i=0;i<pattern[11][0];i++){
        if(pattern[i][1]>=5 || (pattern[i][1]==4 && (pattern[i][3]==0 || pattern[i][3]==14) ) ){
          pattern[i][6]=90;
        }
        else if(pattern[i][4]>=STRONG && pattern[i][4]<=100 && pattern[i][3]!=14){
          pattern[i][6]=91;
        }
      }
    }
    else if(k==4){
      //�v�����N�����Ȃ�
      for(i=0;i<pattern[11][0];i++){
        if(pattern[i][1]>=5 || (pattern[i][1]==4 && (pattern[i][3]==0 || pattern[i][3]==14) ) ){
          pattern[i][6]=8;
        }
      }
    }
    else if(k==3){
      //�ア�J�[�h���o���Ȃ��悤�l����(���̎��_�ł͋N�����Ȃ�)
      for(i=0;i<pattern[11][0];i++){
        if(pattern[i][0]<=5){
          pattern[i][6]=7;
        }
        if(pattern[i][1]>=5 || (pattern[i][1]==4 && (pattern[i][3]==0 || pattern[i][3]==14) ) ){
          pattern[i][6]=8;
        }
      }
    }
    else{//k==2
      //���ɕύX�Ȃ�
    }

    //�\��
        if(PRINT_PAT==2){
          for(i=0;i<11;i++){
            if(pattern[i][1]==0){
              i=11;
            }
            else{
              fprintf( stderr, "[ord%2d][qty%d][sui%2d][Jok%2d][x%3d][rx%3d][y%2d]\n",pattern[i][0],pattern[i][1],pattern[i][2],pattern[i][3],pattern[i][4],pattern[i][5],pattern[i][6]);
            }
          }
          fprintf( stderr, "[set%2d][rev%d][1min%3d][2min%3d][z%3d][rz%3d][r2m%3d]\n",pattern[11][0],pattern[11][1],pattern[11][2],pattern[11][3],pattern[11][4],pattern[11][5],pattern[11][6]);
          fprintf( stderr, "\n");
        }

    pattern[12][5]=-1;
    for(i=0;i<pattern[11][0];i++){
      //��D�̑g����y����ԑ傫�����̂�I��
      if(pattern[12][6]<pattern[i][6]){
        pattern[12][0]=pattern[i][0];
        pattern[12][1]=pattern[i][1];
        pattern[12][2]=pattern[i][2];
        pattern[12][3]=pattern[i][3];
        pattern[12][4]=pattern[i][4];
        pattern[12][5]=pattern[i][5];
        pattern[12][6]=pattern[i][6];
      }
    }
    if(pattern[11][3]==201 && pattern[12][3]==0){//�c��1�g
      j=0;
      for(i=0;i<pattern[11][0];i++){
        if(pattern[i][3]==14 && pattern[i][2]==0){
          j=1;
        }
      }
      if(j==1){//�W���[�J�[���܂߂ďo��
        pattern[12][1]+=1;
        pattern[12][3]=14;
        if(pattern[12][2]%2!=1){
          pattern[12][2]+=1;
        }
        else if(pattern[12][2]%4<=1){
          pattern[12][2]+=2;
        }
        else if(pattern[12][2]%8<=3){
          pattern[12][2]+=4;
        }
        else if(pattern[12][2]<8){
          pattern[12][2]+=8;
        }
      }
    }
    else if(pattern[12][4]>=STRONG && pattern[12][4]<=100 && pattern[12][3]==0 && pattern[12][1]==1){//�������I���̏ꍇ(�P��)
      j=0;
      pattern[12][1]=0;
      pattern[12][2]=0;
      for(i=0;i<pattern[11][0];i++){
        if(pattern[i][0]==pattern[12][0] && pattern[i][3]==0){
          pattern[12][1]+=1;
          pattern[12][2]+=pattern[i][2];
        }
        if(pattern[i][3]==14 && pattern[i][2]==0){//�W���[�J�[���K�i�Ɏg�p���Ă��Ȃ��ꍇ
          j=1;
        }
      }
      if(pattern[12][1]==4){//4������Ƃ��̓X�y�[�h�ƃn�[�g��2���̂ݏo��
        pattern[12][1]=2;
        pattern[12][2]=3;
      }
      if(pattern[12][1]!=3 && j==1){
        //�W���[�J�[���܂߂ďo��
        pattern[12][1]+=1;
        pattern[12][3]=14;
        if(pattern[12][2]%2!=1){
          pattern[12][2]+=1;
        }
        else if(pattern[12][2]%4<=1){
          pattern[12][2]+=2;
        }
        else if(pattern[12][2]%8<=3){
          pattern[12][2]+=4;
        }
        else if(pattern[12][2]<8){
          pattern[12][2]+=8;
        }
      }
    }
    //pattern����select_cards�ɒ�o�������
    if(pattern[12][3]==14 && pattern[12][2]==0){//�W���[�J�[�̏ꍇ
      select_cards[0][0]=2;
    }
    else if(pattern[12][3]==14 && pattern[12][2]!=0){//�W���[�J�[���̖����g
      if(pattern[12][2]%2==1){
        select_cards[0][pattern[12][0]]=1;
        if(own_cards[0][pattern[12][0]]==0){
          select_cards[0][pattern[12][0]]=2;
        }
      }
      if(pattern[12][2]%4>=2){
        select_cards[1][pattern[12][0]]=1;
        if(own_cards[1][pattern[12][0]]==0){
          select_cards[1][pattern[12][0]]=2;
        }
      }
      if(pattern[12][2]%8>=4){
        select_cards[2][pattern[12][0]]=1;
        if(own_cards[2][pattern[12][0]]==0){
          select_cards[2][pattern[12][0]]=2;
        }
      }
      if(pattern[12][2]>=8){
        select_cards[3][pattern[12][0]]=1;
         if(own_cards[3][pattern[12][0]]==0){
          select_cards[3][pattern[12][0]]=2;
        }
      }
    }
    else if(pattern[12][3]!=0){//�K�i�g
      if(pattern[12][2]==1){
        k=0;
      }
      else if(pattern[12][2]==2){
        k=1;
      }
      else if(pattern[12][2]==4){
        k=2;
      }
      else{
        k=3;
      }
      for(j=0;j<pattern[12][1];j++){
        if(own_cards[k][pattern[12][0]-j]==1){
          select_cards[k][pattern[12][0]-j]=1;
        }
        else{
          select_cards[k][pattern[12][0]-j]=2;
        }
      }
    }
    else{//�W���[�J�[�Ȃ��̖����g
      if(pattern[12][2]%2==1){
        select_cards[0][pattern[12][0]]=1;
      }
      if(pattern[12][2]%4>=2){
        select_cards[1][pattern[12][0]]=1;
      }
      if(pattern[12][2]%8>=4){
        select_cards[2][pattern[12][0]]=1;
      }
      if(pattern[12][2]>=8){
        select_cards[3][pattern[12][0]]=1;
      }
    }
    for(i=0;i<4;i++){
      for(j=0;j<=14;j++){
        if(select_cards[i][j]==1 && own_cards[i][j]==0){
          select_cards[i][j]=2;
        }
      }
    }
  }
}

void kou_followgroup(int select_cards[][15],int own_cards[8][15],int max,int used_cards[8][15],int my_player_num,int seat_order[5],int last_playnum, int ba_cards[8][15],int game_count){
//�����ăJ�[�h�𖇐��g�ŏo���Ƃ��̎v�l
  int i,j,k;
  int p=0;//�v���C����
  int ord=state.ord;//���݌��Ă���J�[�h�̋���
  int card_flag=1;//�T���I���܂�1
  int target_cards[8][15];//�o���J�[�h�̗D���������
  int use_own_cards[8][15];//�o������̏�Ԃ�����
  int search[8][15];
  int suit[5]={0};//�X�[�g(suit[4]�̓X�[�g�̑g�ݍ��킹�p�^�[���̏���)
  int pattern[13][7]={{0}};//�J�[�h��I�ԑO�̎�D�̃p�^�[��
  int use_pattern[13][7]={{0}};//�J�[�h��I�яo������̎�D�̃p�^�[��
  int own_num[15]={0};//�����̃J�[�h�̖������L�^
  for(i=1;i<=13;i++){
    own_num[i]=own_cards[0][i]+own_cards[1][i]+own_cards[2][i]+own_cards[3][i];
  }
  pat_make(pattern,own_cards,max,used_cards,state.joker);//���݂̎�D�̑g�����
  if(state.lock==1){//����̂Ƃ�
    for(i=0;i<4;i++){
      suit[i]=state.suit[i];
    }
  }
  for(i=0;i<5;i++){//�������Ă��Ȃ��l���J�E���g
    if(own_cards[6][i]>0){
      p++;
    }
  }
  if(state.rev==0){
    ord++;
  }
  else{
    ord--;
  }
  //��̃J�[�h�̎��ɋ����J�[�h�̋�������T��
  clearTable(select_cards);
  clearTable(target_cards);
  suit[4]=0;
  if(ord>13 || ord<1){
    card_flag=0;
  }
  while(card_flag==1)//�t���O��1�̊�
  {
    //�I�ԃX�[�g�̃p�^�[�������ׂĒ��ׂ�
    clearTable(target_cards);
    copyCards(search,own_cards);//search��own������
    if(state.lock==1){//����̏ꍇ��1�p�^�[���̂�
      for(i=0;i<4;i++){
        suit[i]=state.suit[i];
      }
    }
    else{//���ׂĂ̏o����p�^�[���𒲂ׂ�
      if(state.qty==1){
        suit[0]=0;
        suit[1]=0;
        suit[2]=0;
        suit[3]=0;
        suit[suit[4]]=1;
      }
      if(state.qty==3){
        suit[0]=1;
        suit[1]=1;
        suit[2]=1;
        suit[3]=1;
        suit[suit[4]]=0;
      }
      if(state.qty==2){
        if(suit[4]<=2){
          suit[0]=1;
        }
        else{
          suit[0]=0;
        }
        if(suit[4]==0 || suit[4]==3 || suit[4]==4){
          suit[1]=1;
        }
        else{
          suit[1]=0;
        }
        if(suit[4]%2==1){
          suit[2]=1;
        }
        else{
          suit[2]=0;
        }
        if(suit[4]==2 || suit[4]==4 || suit[4]==5){
          suit[3]=1;
        }
        else{
          suit[3]=0;
        }
      }
      if(state.qty==4){
        suit[0]=1;
        suit[1]=1;
        suit[2]=1;
        suit[3]=1;
      }
    }//�X�[�g�̃p�^�[���̒T�������܂�
    if(pattern[12][2]>=1){
      pattern[12][2]=1;
    }
    //�o���邩�ǂ����𔻒肷��
    for(i=0;i<4;i++){
      if(suit[i]==1 && own_cards[i][ord]==0){
        if(pattern[12][2]==0){//�W���[�J�[�Ȃ�
          i=6;
        }
        else if(pattern[12][2]==1){//�W���[�J�[������ꍇ
          if(state.qty==1){//��̖�����1��
            i=6;//�W���[�J�[�P�͕̂ʂ̔��������̂ŁA�����ł͑I�΂Ȃ�
          }
          else{
            pattern[12][2]=3;//�g�����ƍl����
          }
        }
        else if(pattern[12][2]==3){//�����Ă��Ȃ��X�[�g��2�ȏ�
          pattern[12][2]=1;
          i=6;
        }
      }
    }
    if(i<=5)//�o����ꍇ
    {
      for(i=0;i<4;i++){
        if(suit[i]==1 && own_cards[i][ord]==1){
          target_cards[i][ord]=1;
        }
        else if(suit[i]==1 && own_cards[i][ord]==0){
          target_cards[i][ord]=2;
        }
      }
      clearTable(use_own_cards);
      copyCards(use_own_cards,own_cards);//use_own��own������
      cardsDiff(use_own_cards,target_cards);//use_own����target(�I��������)������
      clearTable(search);
      copyCards(search,used_cards);//search��used_cards(���łɏ�ɏo���J�[�h)������
      for(i=0;i<=4;i++){//search�ɏo���J�[�h������(�I����������o�����ꍇ�̏�ɏo���J�[�h)
        for(j=0;j<=14;j++){
          if(target_cards[i][j]==1){
            search[i][j]=1;
          }
          if(target_cards[i][j]==2){
            search[4][14]=1;
          }
        }
      }
      for(j=1;j<=13;j++){
        search[4][j]=search[0][j]+search[1][j]+search[2][j]+search[3][j];
      }
      for(i=0;i<5;i++){
        use_own_cards[6][i]=own_cards[6][i];//��D�����̏�������
      }
      use_pattern[12][2]=pattern[12][2];//�W���[�J�[���g�p�������ǂ���������
      pat_make(use_pattern,use_own_cards,max,search,pattern[12][2]);//��o�����ꍇ�̎�D�̑g�����
      //����̔���
      if(ord==6){//8�؂�̏ꍇ
        i=6;
      }
      else if(state.rev==0 && ord>=max){//��ň�ԋ����J�[�h�̏ꍇ
        i=6;
      }
      else if(state.rev==1 && ord<=max){
        i=6;
      }
      else if(state.lock==1){//�����Ԃ̏ꍇ
        i=5;//����̔��������
      }
      else{//����łȂ��ꍇ
        for(i=0;i<4;i++){//�o���ꍇ,����ɂȂ邩�ǂ������ׂ�
          if(state.suit[i]!=suit[i]){//����ɂȂ�Ȃ��ꍇ
            i=6;
          }
        }
      }
      /*
      i=5�c����̏ꍇ�̔��������
      i=6�c�ʏ�(����łȂ��ꍇ)�̔��������
      */
      if(i<=5 && state.qty!=1 && p>=3){//�����Ԃ�����̏ꍇ�ŏꂪ2���ȏ�A���肪2�l�ȏ�Ȃ�ϋɓI�ɏo��
        i=5;
      }
      else if(i<=5){//�����Ԃ�����ɂ���ꍇ
        if(state.rev==0){//�ʏ펞
          for(j=max;j>ord;j--){
            for(i=0;i<4;i++){
              if(suit[i]>0 && used_cards[i][j]+own_cards[i][j]!=0){//���肪�o����\�������邩�ǂ������ׂ�
                i=6;
              }
            }
            if(i<=5){//���肪�o����\��������ꍇ
              //if(state.lock==1)
                i=6;//�����Ԃ̂Ƃ��͏o���D��x�͕ς��Ȃ�
              //else
              //  i=7;//�ϋɓI�ɂ͏o���Ȃ�
              j=0;//���肪�o����\��������Ƃ��͂������[�v�𔲂���
            }
            else{//���肪�o����\�����Ȃ��ꍇ
              for(i=0;i<4;i++){
                if(suit[i]>0 && own_cards[i][j]==0){//�����������X�[�g�ŋ����J�[�h�������Ă��Ȃ��ꍇ
                  i=6;
                }
              }
              if(i<=5){//�����������X�[�g�ŋ����J�[�h�������Ă���ꍇ
                if(use_pattern[12][2]==3){
                  i=6;//�W���[�J�[�g�p�̏ꍇ�͏o���D��x�͕ς��Ȃ�
                }
                else{
                  i=5;//�ϋɓI�ɏo��
                }
                j=0;//���[�v�𔲂���
              }
              else if(ord==j-1){//�����X�[�g�ŋ����J�[�h���Ȃ��ꍇ(�o���J�[�h�����̃X�[�g�ň�ԋ����ꍇ)
                i=5;//�ϋɓI�ɏo��
                j=0;//���[�v�𔲂���
              }
            }
          }
        }
        else{//�v�����������悤�ɔ��f
          for(j=max;j<ord;j++){
            for(i=0;i<4;i++){
              if(suit[i]>0 && used_cards[i][j]+own_cards[i][j]!=0){
                i=6;
              }
            }
            if(i<=5){
              //if(state.lock==1)
                i=6;
              //else
              //  i=7;
              j=14;
            }
            else{
              for(i=0;i<4;i++){
                if(suit[i]>0 && own_cards[i][j]==0){
                  i=6;
                }
              }
              if(i<=5){
                if(use_pattern[12][2]==3){
                  i=6;
                }
                else{
                  i=5;
                }
                j=14;
              }
              else if(ord==j+1){
                i=5;
                j=14;
              }
            }
          }
        }
      }
      else{//����łȂ��ꍇ
        i=6;
      }//����̔��肱���܂�
      /*
      i=5�c�������Ŕ���ꍇ��,1���Ŕ���ꍇ�ň�ԋ����J�[�h�������Ă���ꍇ
      i=7�c1���Ŕ���ꍇ�ň�ԋ����J�[�h�������Ă��Ȃ��ꍇ(�폜)
      i=6�c��L�ȊO
      */
      //�I�񂾃J�[�h�̕]���l(x')�̌���
      if(i==6){
        use_pattern[12][0]=100;//�
        if(ord==6){//8�؂�
          use_pattern[12][0]=101;
        }
        else if(state.qty==1){//�P�̂̏ꍇ�A���肪�o����g������Ƃ�-30����
          if(state.rev==0){
            for(j=max;j>ord;j--){
              if((4-used_cards[4][j]-own_num[j])>=1){
                use_pattern[12][0]-=30;
              }
            }
          }
          else{
            for(j=max;j<ord;j++){
              if((4-used_cards[4][j]-own_num[j])>=1){
                use_pattern[12][0]-=30;
              }
            }
          }
          if(used_cards[4][14]==0 && state.joker==0){
            use_pattern[12][0]-=1;
          }
        }//�P�̂̏ꍇ�̔��肱���܂�
        else{//�������̏ꍇ
          if(state.rev==0){
            for(j=max;j>ord;j--){
              if((4-used_cards[4][j]-own_num[j])==(state.qty-1) && used_cards[4][14]==0 && state.joker==0){//�W���[�J�[���܂߂�Əo�����ꍇ
                use_pattern[12][0]-=1;//1�������炷
              }
              else if((4-used_cards[4][j]-own_num[j])<=(state.qty-1)){//�o�����p�^�[���Ȃ�
                //���炳�Ȃ�
              }
              else{
                k=(4-used_cards[4][j]-own_num[j])-state.qty-p+5;
                if(p==2){
                  use_pattern[12][0]-=30;
                }
                else if(k<=0){use_pattern[12][0]-=4;}
                else if(k==1){use_pattern[12][0]-=9;}
                else if(k==2){use_pattern[12][0]-=15;}
                else{use_pattern[12][0]-=24;}
              }
            }
          }
          else{
            for(j=max;j<ord;j++){
              if((4-used_cards[4][j]-own_num[j])==(state.qty-1) && used_cards[4][14]==0 && state.joker==0){//�W���[�J�[���܂߂�Əo�����ꍇ
                use_pattern[12][0]-=1;//1�������炷
              }
              else if((4-used_cards[4][j]-own_num[j])<=(state.qty-1)){//�o�����p�^�[���Ȃ�
                //���炳�Ȃ�
              }
              else{
                k=(4-used_cards[4][j]-own_num[j])-state.qty-p+5;
                if(p==2){
                  use_pattern[12][0]-=30;
                }
                else if(k<=0){use_pattern[12][0]-=4;}
                else if(k==1){use_pattern[12][0]-=9;}
                else if(k==2){use_pattern[12][0]-=15;}
                else{use_pattern[12][0]-=24;}
              }
            }
          }
        }//�������̏ꍇ�̔��肱���܂�
        if(use_pattern[12][0]<=0){
          use_pattern[12][0]=1;
        }
      }
      else if(i==5){//�������Ŕ���ꍇ��,1���Ŕ���ꍇ�ň�ԋ����J�[�h�������Ă���ꍇ��x'=STRONG
        use_pattern[12][0]=STRONG;
      }
      else{//�����\�z�O�̓��삪�N�����ꍇ
        use_pattern[12][0]=1;//x'=1�ɂ���
      }
      //�o������̑S�̕]���l(z')�̌���
      use_pattern[12][1]=use_pattern[11][4];
      //x'�ɂ����z'��ω�
      if(use_pattern[12][0]>=STRONG){//��𗬂���\���������ꍇ1���炷
        use_pattern[12][1]-=1;
      }
      if(i!=5 && use_pattern[12][0]<STRONG && use_pattern[11][0]>=pattern[11][0]){//��𗬂���\�����Ⴍ�A�g��������Ȃ��ꍇ1���₷
        use_pattern[12][1]+=1;
      }
      //�o�����ǂ�������(�_��2408��o���ׂ��}�[�N�t�^�̕���)///////////////////////////////////////////////////////////
      j=1;
	 
	  //------------------------------------------------------//
	  //���슄���݁mj=0�ŏo���Ȃ��悤�Ɂi�p�X�j�n
	  if (status_print == 1) {
		  printf("\n\nstart/////////////////////////////////////////\n");
	  }

	  int seat_tactics_select = seat_tactics(own_cards, my_player_num, seat_order,last_playnum,used_cards,game_count);

	  if (seat_tactics_select == 1) {
		  if (status_print == 1) {
			  printf("\nseat_tactics <<<ON>>>\n\n");
		  }
	  }
	  else {
		  if (status_print == 1) {
			  printf("\nseat_tacticks <<<OFF>>>\n\n");
		  }
	  }

	  if (status_print == 1) {
		  printf("end-------------------------------------------\n");
	  }
	  //------------------------------------------------------//
	  
      if(use_pattern[12][0]>=STRONG && use_pattern[11][3]>=STRONG){
        //___(1)___x>=STRONG,2min>=STRONG ���o��(������̌`�̏ꍇ�͏o��)
        j=1;
      }
      else if(use_pattern[12][0]==101 && use_pattern[11][0]<=pattern[11][0]){
        //___(2)___8���܂ޑg�őg�������邩���� ���o��(8�̏ꍇ�A�g���������Ȃ��ꍇ�͏o��)
        j=1;
      }
      else if(use_pattern[12][0]>STRONG && use_pattern[11][0]>=4 && use_pattern[11][0]-1<=use_pattern[12][1]){
        //___(3)___x>=STRONG,�g��4�ȏ�,�g��-1<=z'���o���Ȃ�(��𗬂���ꍇ�ŁA���ɋ����g���Ȃ��ꍇ�o���Ȃ�)
        j=0;
      }
      else if(use_pattern[11][2]<=50 && use_pattern[11][3]==201 && use_pattern[12][0]<=60){
		//___(4)___1min<=50,2min=201,x'<=60 ���o���Ȃ�(�c��1�g�̏ꍇ�ŏ�𗬂���\�����Ⴂ�ꍇ�͏o���Ȃ�)
		j=0;
      }
      else if(use_pattern[12][2]==3 && use_pattern[12][0]<=STRONG ){
        //___(�_���O)___�W���[�J�[�g�p�̏ꍇ��x<=STRONG(2min>=STRONG,x=STRONG�͏o��) ���o���Ȃ�
        j=0;
      }
      else if(use_pattern[11][3]>=STRONG){
        //___(5)___2min>=STRONG ���o��(���̏ꍇ�ȊO�ŁA������̌`�ɂȂ�ꍇ�͏o��)
        j=1;
      }
      //else if(state.qty==1 && use_pattern[12][0]==31 && use_pattern[12][1]<=pattern[11][4]){
      //  j=2;//����ŋ����J�[�h�������Ă��Ȃ��ꍇ�͏o�����A�D��x�͒Ⴍ����
      //}
      else if(state.qty==1 && use_pattern[12][1]>=4 && use_pattern[12][0]>=40 && use_pattern[12][0]<=70){
		//___(6)___//qty=1,z'>=4,40<=x'<=70 ���o���Ȃ�(��D�����������͒P�̂�K��A�͏o���Ȃ�)
		j=0;
      }
      else if(use_pattern[12][1]>pattern[11][4]){
        //___(7)___z'>z ���o���Ȃ�(���ȊO�ŁAz���傫���Ȃ�ꍇ�͏o���Ȃ�)
        j=0;
      }
	  else if (seat_tactics_select == 1) {
		  //���슄����----------------------------------------------------
		  j = 0;
	  }
      else{
        j=1;//�g��������,z���傫���Ȃ�Ȃ��ꍇ�͑�̏o��
      }
	  //�o�����ǂ������f_�����_///////////////////////////////////////////////////////////////////////////////////////
      if(PRINT_PAT==2){
        fprintf( stderr, "ord%2d,suit%4d ",ord,(suit[0]*1000)+(suit[1]*100)+(suit[2]*10)+suit[3]);
        fprintf( stderr, "1min%3d,2min%3d ",use_pattern[11][2],use_pattern[11][3],use_pattern[11][4]);
        fprintf( stderr, "x'%3d\n",use_pattern[12][0]);
        fprintf( stderr, "pair%2d->%2d,z'%3d->%3d [check%2d]\n",pattern[11][0],use_pattern[11][0],pattern[11][4],use_pattern[12][1],j);
      }
      if(beEmptyCards(select_cards)==1 && j>=1){//����܂ł̔���ŏo����₪�Ȃ��ꍇ
          copyCards(select_cards,target_cards);//�o������
          pattern[12][0]=use_pattern[12][0];
          pattern[12][1]=use_pattern[12][1];//+j-1
      }
      else if(j>=1){//�o����₪����ꍇ
        if(use_pattern[11][3]>=STRONG && pattern[12][0]>=STRONG && use_pattern[12][0]<STRONG){
          //2min>=STRONG,�ύX�O�̎��x>=STRONG �ύX��̎��x<STRONG���o���Ȃ�
        }
        else if(use_pattern[11][3]>=STRONG && use_pattern[12][0]>=STRONG && pattern[12][0]<STRONG){
          //2min>=STRONG,�ύX��̎��x>=STRONG,�ύX�O�̎��x<STRONG ���o��
          clearCards(select_cards);
          copyCards(select_cards,target_cards);//�o������
          pattern[12][0]=use_pattern[12][0];
          pattern[12][1]=use_pattern[12][1];
        }
        else if(use_pattern[11][3]>=STRONG && use_pattern[12][0]>STRONG && pattern[12][0]==STRONG){
          //2min>=STRONG,�ύX��̎��x>STRONG,�ύX�O�̎��x=STRONG ���o��(�����蔻��̂Ƃ��́u����v�ł͂Ȃ��A�����J�[�h�ŏ�𗬂�)
          clearCards(select_cards);
          copyCards(select_cards,target_cards);//�o������
          pattern[12][0]=use_pattern[12][0];
          pattern[12][1]=use_pattern[12][1];
        }
        else if((use_pattern[12][1]+j-1)<pattern[12][1]){
          //�ύX��̎��z<�ύX�O�̎��z ���o��
          clearCards(select_cards);
          copyCards(select_cards,target_cards);//�o������
          pattern[12][0]=use_pattern[12][0];
          pattern[12][1]=use_pattern[12][1];
        }
      }
    }//�I�񂾃J�[�h���o����ꍇ�̔��肱���܂�
//���̃X�[�g�̑g�ݍ��킹���o��
    clearTable(target_cards);
    suit[4]++;
    if(state.lock==1){
      suit[4]=0;
    }
    else if(state.qty==1 || state.qty==3){
      if(suit[4]==4){
        suit[4]=0;
      }
    }
    else if(state.qty==2 && suit[4]==6){
      suit[4]=0;
    }
    else if(state.qty==4){
      suit[4]=0;
    }
    if(suit[4]==0 && state.rev==0){//�X�[�g�̑g�ݍ��킹�𒲂׏I������玟�̐����ɂ���
      ord++;
    }
    else if(suit[4]==0){
      ord--;
    }
    if(ord>13 || ord<1){//�o����g�ݍ��킹�𒲂׏I������烋�[�v�𔲂���
      card_flag=0;
    }
  }//�W���[�J�[�P�̈ȊO�̔���I��

//��̃J�[�h��1���̂Ƃ��W���[�J�[�P�̂ɂ��čl����
  if(state.qty==1 && state.joker==1){
    //use_pattern[12][2]=3;//�W���[�J�[�g�p
    pat_make(use_pattern,own_cards,max,used_cards,0);//�W���[�J�[�g�p���̃p�^�[�������
    if((pattern[11][0]>=use_pattern[11][0] && state.ord==max) || use_pattern[11][3]>=STRONG){//��̐������ő�őg���������Ȃ� �܂��� 2min>=STRONG
      if(used_cards[0][1]+own_cards[0][1]==1){//�X�y�[�h3���o�Ă��Ȃ�
        if(beEmptyCards(select_cards)==1 && j>=1){//����܂ł̔���ŏo����₪�Ȃ��ꍇ
          clearTable(select_cards);
          select_cards[0][0]=2;//�o��
        }
        else if(pattern[12][0]<STRONG){
          //�I�����Ă������x<STRONG ���o��
          clearTable(select_cards);
          select_cards[0][0]=2;//�o��
        }
      }
    }
  }
}

void kou_followsequence(int select_cards[][15],int own_cards[8][15],int max,int used_cards[8][15]){
//�K�i�ŏo���Ƃ��̎v�l
  int i,j;
  int suit,ord;//���݌��Ă���X�[�g,����
  int count;
  int jk=-1;//�W���[�J�[�ʒu
  int card_flag=1;//�o����J�[�h�������1
  int target_cards[8][15];//�o���J�[�h�̗D���������
  int use_own_cards[8][15];//�o������̏�Ԃ�����
  int search[8][15];
  int pattern[13][7]={{0}};//�J�[�h��I�ԑO�̎�D�̃p�^�[��
  int use_pattern[13][7]={{0}};//�J�[�h��I�яo������̎�D�̃p�^�[��
  pat_make(pattern,own_cards,max,used_cards,state.joker);//���݂̎�D�̃p�^�[�������
  int seq_max;
  int seq_min;
  if(state.rev==0){
    seq_max=14;
    seq_min=state.ord+state.qty;
  }
  else{
    seq_max=state.ord-1;
    seq_min=state.qty-1;
  }
  clearTable(select_cards);
  clearTable(target_cards);
  clearTable(search);

    for(suit=0;suit<4;suit++){
      if(state.lock==1){//����̂Ƃ�
        while(state.suit[suit]!=1 && suit<4){
          suit++;
        }
      }
      for(ord=seq_max;ord>=seq_min && suit<4;ord--){
        jk=-1;
        count=0;
        clearTable(target_cards);
        while(count!=state.qty && (ord-count)>=0){
          if(own_cards[suit][ord-count]==1){
            target_cards[suit][ord-count]=1;
            count++;
          }
          else if(jk==-1 && own_cards[4][1]==2){
            target_cards[suit][ord-count]=2;
            jk=ord-count;
            count++;
          }
          else{
            break;
          }
        }
        if(count==state.qty){//�K�i�����̏ꍇ

      clearTable(use_own_cards);
      copyCards(use_own_cards,own_cards);//use_own��own������
      cardsDiff(use_own_cards,target_cards);//use_own����target(�I��������)������
      clearTable(search);
      copyCards(search,used_cards);//search��used_cards(���łɏ�ɏo���J�[�h)������
      for(i=0;i<=4;i++){//search�ɏo���J�[�h������(�I����������o�����ꍇ�̏�ɏo���J�[�h)
        for(j=0;j<=14;j++){
          if(target_cards[i][j]==1){
            search[i][j]=1;
          }
          if(target_cards[i][j]==2){
            search[4][14]=1;
          }
        }
      }
      for(j=1;j<=13;j++){
        search[4][j]=search[0][j]+search[1][j]+search[2][j]+search[3][j];
      }
      for(i=0;i<5;i++){
        use_own_cards[6][i]=own_cards[6][i];//��D�����̏�������
      }
      if(jk!=-1){//�W���[�J�[���g�p�����ꍇ
        use_pattern[12][2]=3;
      }
      pat_make(use_pattern,use_own_cards,max,search,use_pattern[12][2]);//��o�����ꍇ�̎�D�̃p�^�[�������
        //�o�����ǂ�������
        j=0;
        if(use_pattern[11][3]>=STRONG){
          //2min>=STRONG ���o��(������̌`�ɂȂ�ꍇ�͏o��)
          j=1;
        }
        else if(use_pattern[11][4]<=pattern[11][4]+1){
          j=1;
        }
        if(beEmptyCards(select_cards)==1 && j>=1){//����܂ł̔���ŏo����₪�Ȃ��ꍇ
          copyCards(select_cards,target_cards);//�o������
          //pattern[12][0]=99;
          pattern[12][1]=use_pattern[11][4];
        }
        else if(j>=1){//�o����₪����ꍇ
          if(use_pattern[12][1]<pattern[12][1]){
            //�ύX��̎��z<�ύX�O�̎��z ���o��
            clearCards(select_cards);
            copyCards(select_cards,target_cards);//�o������
            //pattern[12][0]=99;
            pattern[12][1]=use_pattern[11][4];
          }
        }

        }//�K�i�����̏ꍇ�����܂�
      }
    }
}

void kou_change(int out_cards[8][15],int my_cards[8][15],int num_of_change){
//�J�[�h�����̔���
  int i,j,k;
  int search[8][15];
  int cards[15][3]={{0}};//[0]����[1]�X�[�g(0,1,2,3)[2]��D����I�񂾃J�[�h���������ꍇ��z
  int used_cards[8][15]={{0}};//��o��̃J�[�h ���������ĂȂ��̂�0
  int pattern[13][7]={{0}};
  clearTable(out_cards);
  clearTable(search);
  int c;
  k=0;
  for(j=1;j<=12;j++){//3����A�܂�
    for(i=0;i<4;i++){
      if(my_cards[i][j]==1){
        cards[k][0]=j;
        cards[k][1]=i;
        k++;
      }
    }
  }
  for(i=0;i<4;i++){
    my_cards[i][13]=0;
  }
  for(i=0;i<k;i++){
    clearTable(search);
    copyTable(search,my_cards);
    search[cards[i][1]][cards[i][0]]=0;
    pat_make(pattern,search,13,used_cards,state.joker);
    cards[i][2]=(pattern[11][4]*10)+cards[i][0];
    if(cards[i][0]==1){
      cards[i][2]+=5;
      if(cards[i][1]==0 || cards[i][2]==2){
        cards[i][2]+=20;
      }
    }
    if(cards[i][0]==6){
      cards[i][2]+=30;
    }
    if(cards[i][0]==11){
      cards[i][2]+=15;
    }
    if(cards[i][0]==12){
      cards[i][2]+=30;
    }
  }
  for(i=0;i<k;i++){
    for(j=i;j>0;j--){
      if(cards[j][2]<cards[j-1][2]){
        cards[14][0]=cards[j][0];
        cards[14][1]=cards[j][1];
        cards[14][2]=cards[j][2];
        cards[j][0]=cards[j-1][0];
        cards[j][1]=cards[j-1][1];
        cards[j][2]=cards[j-1][2];
        cards[j-1][0]=cards[14][0];
        cards[j-1][1]=cards[14][1];
        cards[j-1][2]=cards[14][2];
      }
    }
  }
  out_cards[cards[0][1]][cards[0][0]]=1;
  if(num_of_change==2){
    out_cards[cards[1][1]][cards[1][0]]=1;
  }
}

void pat_make(int pattern[][7],int own_cards[8][15],int max,int used_cards[8][15],int joker_flag){
//��D�̑g���ƕ]���l�̌���
            int i,j,k;
            int select_cards[8][15];
            int search[8][15];
            clearTable(search);
            for(i=0;i<13;i++){
              for(j=0;j<7;j++){
                pattern[i][j]=0;
              }
            }
            int own_cards_copy[8][15];//�T���p��D�e�[�u��
            copyTable(own_cards_copy,own_cards);
            if(joker_flag==2){//�t���O��2(�O�̈�)�̏ꍇ1
              joker_flag=1;
            }
            if(joker_flag==3){//�t���O��3(��o��Ŏg�p)�̏ꍇ0
              joker_flag=0;
            }
            //�J�[�h�̑g�����
            i=0;
            j=1;
            while(j!=0){
              k=setmake(search,own_cards_copy,joker_flag);
              j=kaidanhand(select_cards,search,own_cards_copy);//�K�i�����
              if(j!=0){//�K�i
                cardsDiff(own_cards_copy,select_cards);
                pattern[i][0]=j;
                pattern[i][3]=0;
                for(pattern[i][2]=0;select_cards[pattern[i][2]][j]==0;pattern[i][2]++){
                  //�X�[�g�𒲂ׂ�
                }
                for(;j>=0;j--){
                  if(select_cards[pattern[i][2]][j]==2){
                    pattern[i][3]=j;
                    pattern[12][2]=2;
                    joker_flag=0;
                  }
                  else if(select_cards[pattern[i][2]][j]==0){
                    pattern[i][1]=pattern[i][0]-j;
                    j=0;
                  }
                }
                if(pattern[i][2]==0){pattern[i][2]=1;}
                else if(pattern[i][2]==1){pattern[i][2]=2;}
                else if(pattern[i][2]==2){pattern[i][2]=4;}
                else if(pattern[i][2]==3){pattern[i][2]=8;}
                if(pattern[i][3]==0){
                  pattern[i][3]=15;
                }
                i++;
                j=-1;
              }
              else{//�����g
                clearTable(select_cards);
                j=grouphand(select_cards,search,0);//�ア�����g�����

                if(j>=1){//8,���J�[�h�ȊO�̖����g
                  cardsDiff(own_cards_copy,select_cards);
                  pattern[i][0]=j;
                  pattern[i][1]=select_cards[0][j]+select_cards[1][j]+select_cards[2][j]+select_cards[3][j];
                  pattern[i][3]=0;
                  if(pattern[i][1]==4 && state.rev==0 && j>=max){//���J�[�h4���̏ꍇ,2�����̑g�ɂ���
                    pattern[i][1]=2;
                    pattern[i][2]=3;
                    i++;
                    pattern[i][0]=j;
                    pattern[i][1]=2;
                    pattern[i][2]=12;
                    pattern[i][3]=0;
                  }
                  else if(pattern[i][1]==4 && state.rev==1 && j<=max){//���J�[�h4���̏ꍇ(�v����)
                    pattern[i][1]=2;
                    pattern[i][2]=3;
                    i++;
                    pattern[i][0]=j;
                    pattern[i][1]=2;
                    pattern[i][2]=12;
                    pattern[i][3]=0;
                  }
                  else{
                    pattern[i][2]=0;
                    if(select_cards[0][j]==1){pattern[i][2]+=1;}
                    if(select_cards[1][j]==1){pattern[i][2]+=2;}
                    if(select_cards[2][j]==1){pattern[i][2]+=4;}
                    if(select_cards[3][j]==1){pattern[i][2]+=8;}
                  }
                  i++;
                  j=-1;
                }
                if(j==0 && joker_flag==1){//�W���[�J�[�P��
                  pattern[i][1]=1;
                  pattern[i][3]=14;
                  pattern[12][2]=1;
                  joker_flag=0;
                  i++;
                }
              }
              clearTable(search);
              clearTable(select_cards);
            }
//�g���i�[
            for(i=0;i<11;i++){
              if(pattern[i][1]==0){
                pattern[11][0]=i;
                i=12;
              }
            }
            if(i==11){
              pattern[11][0]=11;
            }
//x,revx,z,revz,1min,2min�̌���
            value_strong(pattern,own_cards,used_cards,state.rev,4);
            value_strong(pattern,own_cards,used_cards,(state.rev+1)%2,5);
//y�]���ly�̌���
            for(i=0;i<pattern[11][0];i++){
              if(pattern[i][3]==14){
                pattern[i][6]=3;
              }
              else if(pattern[i][4]>=STRONG && pattern[i][4]<=100){//�����\���������ꍇ
                pattern[i][6]=110-pattern[i][4];//10�`15
              }
              else if(pattern[i][4]>100 && pattern[i][3]!=0){//�K�i
                pattern[i][6]=9;
              }
              else if(pattern[i][4]>=1 && pattern[i][4]<=200){
                if(state.rev==0){
                  pattern[i][6]=40+(2*(14-pattern[i][0]));//44�`64
                }
                else{
                  pattern[i][6]=40+(2*pattern[i][0]);//44�`64
                }
                if(pattern[i][4]>100){//8���܂܂��ꍇ
                  pattern[i][6]+=8;
                }
                else{//�܂܂�Ȃ��ꍇ
                  for(j=0;j<11;j++){
                    if(pattern[i][1]==pattern[j][1] && pattern[i][3]==0 && pattern[j][3]==0){
                      pattern[i][6]+=4;
                    }
                  }
                }
              }
              else{
                pattern[i][6]=-1;
              }
            }
//�Ŏ�J�[�h�̕]���ly�̕ω�+follow�ŏo���Ȃ��g�̕]���ly�̕ω�
            if(pattern[11][0]>=5){
              if(state.rev==0){
                for(i=0;i<11;i++){
                  if(pattern[i][0]==1){
                    pattern[i][6]-=7;
                    if(pattern[11][pattern[11][0]-1]>=100 || pattern[11][pattern[11][0]-2]>=100){
                      pattern[i][6]-=8;
                    }
                  }
                  else if(pattern[i][5]>=97 && pattern[i][5]<=100){
                    pattern[i][6]+=13;
                  }
                }
              }
              else if(state.rev==1){
                for(i=0;i<11;i++){
                  if(pattern[i][0]==13){
                    pattern[i][6]-=7;
                    if(pattern[11][pattern[11][0]-1]>=100 || pattern[11][pattern[11][0]-2]>=100){
                      pattern[i][6]-=8;
                    }
                  }
                  else if(pattern[i][5]>=97 && pattern[i][5]<=100){
                    pattern[i][6]+=13;
                  }
                }
              }
            }
//���X�N(revx�������g�݂̂̏ꍇ,���Ă��D�Ɣ��f���Ȃ�)
            if(pattern[11][3]>=STRONG && pattern[11][3]<=94){
              j=0;
              for(i=0;i<5;i++){
                if(own_cards[6][i]>0){
                  j++;
                }
              }
              if(j>=4){
                j=202;
                for(i=0;i<pattern[11][0];i++){
                  if(j>pattern[i][5]){
                    j=pattern[i][5];
                  }
                }
                if(j>50){
                  pattern[11][3]=STRONG-1;
                }
              }
            }
//2min�ɂ��]���ly�̕ω�
            if(pattern[11][3]>=STRONG){
              for(i=0;i<11;i++){
                if(pattern[i][4]>=1 && pattern[i][4]<STRONG){
                  pattern[i][6]=2;
                }
                else if(pattern[i][3]==14 && pattern[i][2]==0){
                  if(pattern[i][4]==100){
                    pattern[i][6]=3;
                  }
                  else{
                    pattern[i][6]=1;
                  }
                }
                if(pattern[i][4]>=STRONG){
                  if(pattern[11][3]>=100){
                    pattern[i][6]=pattern[i][4]-60;
                    if(pattern[i][4]>100){
                      pattern[i][6]=41;
                    }
                  }
                  else{
                    pattern[i][6]=110-pattern[i][4];
                    if(pattern[i][4]>100){
                      pattern[i][6]=9;
                    }
                  }
                }
              }
            }
            else if(pattern[11][6]>=STRONG && pattern[11][1]==1){
              pattern[11][1]=0;//lead�̔���ύX
              if(pattern[i][1]>=5 || (pattern[i][1]==4 && (pattern[i][3]==0 || pattern[i][3]==14))){//�v����
                pattern[i][6]=9;
              }
              else if(pattern[i][4]>100){//8�؂�
                pattern[i][6]=7;
              }
              else if(pattern[i][4]>=STRONG && pattern[i][4]>pattern[i][5] ){//x>=STRONG ���� x>revx
                pattern[i][6]=110-pattern[i][4];
              }
              else{
                pattern[i][6]=8;
              }
            }
            if(pattern[11][0]>=3 && pattern[pattern[11][0]-1][3]==14 && pattern[pattern[11][0]-1][2]==0){
               pattern[11][0]-=1;//�W���[�J�[�P�̂͑g���Ɋ܂߂Ȃ�
            }
}

void value_strong(int pattern[][7],int own_cards[8][15],int used_cards[8][15],int rev,int n){//�����]���l�ƑS�̕]���l�v�Z
  //x,revx,z,revz,1min,2min
  //n��pattern�ւ̊i�[�ʒu(4��5)
  if(n!=4 && n!=5){
    n=5;
  }
  int i,j,k,jk,suit;
  int p=0;//�c��l��
  for(i=0;i<5;i++){
    if(own_cards[6][i]>0){
      p++;
    }
  }
  int own_num[15]={0};//�����̃J�[�h�̖������L�^
  for(i=1;i<=13;i++){
    own_num[i]=own_cards[0][i]+own_cards[1][i]+own_cards[2][i]+own_cards[3][i];
  }
  int max=0;
  if(rev==0){
    for(i=13;i>0 && max==0;i--){
      if(own_num[i]+used_cards[4][i]!=4){//���������̃J�[�h�����̎�D�ɂ����
        max=i;//���̋������L�^
      }
    }
  }
  else{
    for(i=1;i<14 && max==0;i++){
      if(own_num[i]+used_cards[4][i]!=4){//���������̃J�[�h�����̎�D�ɂ����
        max=i;//���̋������L�^
      }
    }
  }
  if(max==0){
    if(rev==0){
      max=1;
    }
    else{
      max=13;
    }
  }

  for(i=0;i<11;i++){
    if(pattern[i][3]!=14 && pattern[i][3]!=0){
    //�K�i�̏ꍇ
      if(rev==0){//�ʏ펞
        pattern[i][n]=100;
        for(j=pattern[i][0]+pattern[i][1];j<=(max+1);j++){
          for(suit=0;suit<4;suit++){
            jk=1-used_cards[4][14]-state.joker;//�W���[�J�[���܂܂��\��
            for(k=0;k<pattern[i][1];k++){
              if(used_cards[suit][j-k]==1 || own_cards[suit][j-k]==1){
                if(jk==1){
                  jk=0;
                }
                else{
                  jk=2;
                }
              }
            }
            if(jk!=2){
              if(p==2){
                pattern[i][n]-=15;
              }
              else if(jk==0){
                pattern[i][n]-=1;
              }
              else{
                pattern[i][n]-=6-p;
              }
            }
          }
        }
      }
      else{//�v����
        pattern[i][n]=100;
        for(j=pattern[i][0]-(2*pattern[i][1])+1;j>=(max-1);j--){//j=�J�[�h����-(2*����)+1=��ɏo���Ƃ��ɕK�v�ȋ���(3�ɋ߂���)
          for(suit=0;suit<4;suit++){
            jk=1-used_cards[4][14]-state.joker;
            for(k=0;k<pattern[i][1];k++){
              if(used_cards[suit][j+k]==1 || own_cards[suit][j+k]==1){
                if(jk==1){
                  jk=0;
                }
                else{
                  jk=2;
                }
              }
            }
            if(jk!=2){
              if(p==2){
                pattern[i][n]-=15;
              }
              else if(jk==0){
                pattern[i][n]-=1;
              }
              else{
                pattern[i][n]-=6-p;
              }
            }
          }
        }
      }
      if(pattern[i][0]>=6 && (pattern[i][0]-pattern[i][1]<6)){//8���܂ޏꍇ
        pattern[i][n]+=100;
      }
    }
    else if(pattern[i][3]==14 && pattern[i][2]==0){
    //�W���[�J�[�P��
      if(used_cards[0][1]+own_cards[0][1]==1){//�X�y�[�h3�̉\���Ȃ�
        pattern[i][n]=100;
      }
      else{
        pattern[i][n]=1;
      }
    }
    else if(pattern[i][0]==0){
    //�J�[�h����
      pattern[i][0]=0;
      pattern[i][1]=0;
      pattern[i][2]=0;
      pattern[i][3]=0;
      pattern[i][n]=-1;
    }
    else{
    //�����g
      pattern[i][n]=100;//�
      if(pattern[i][1]==1){
      //�P�̂̏ꍇ�A���肪�o����g������Ƃ�-30����
        if(rev==0){
          for(j=max;j>pattern[i][0];j--){
            if((4-used_cards[4][j]-own_num[j])>=1){
              pattern[i][n]-=30;
            }
          }
        }
        else{
          for(j=max;j<pattern[i][0];j++){
            if((4-used_cards[4][j]-own_num[j])>=1){
              pattern[i][n]-=30;
            }
          }
        }
        if(used_cards[4][14]==0 && state.joker==0){//�W���[�J�[������ꍇ
          pattern[i][n]-=1;
        }
      }
      else{
      //�������̏ꍇ
        if(rev==0){
          for(j=max;j>pattern[i][0];j--){
            if((4-used_cards[4][j]-own_num[j])==(pattern[i][1]-1) && used_cards[4][14]==0 && state.joker==0){//�W���[�J�[���܂߂�Əo�����ꍇ
              pattern[i][n]-=1;
            }
            else if((4-used_cards[4][j]-own_num[j])<=(pattern[i][1]-1)){//�o�����\���Ȃ�
              //���炳�Ȃ�
            }
            else{
              k=(4-used_cards[4][j]-own_num[j])-pattern[i][1]-p+5;
              if(p==2){
                pattern[i][n]-=30;
              }
              else if(k<=0){pattern[i][n]-=4;}
              else if(k==1){pattern[i][n]-=9;}
              else if(k==2){pattern[i][n]-=15;}
              else{pattern[i][n]-=24;}
            }
          }
        }
        else{
          for(j=max;j<pattern[i][0];j++){
            if((4-used_cards[4][j]-own_num[j])==(pattern[i][1]-1) && used_cards[4][14]==0 && state.joker==0){//�W���[�J�[���܂߂�Əo�����ꍇ
              pattern[i][n]-=1;
            }
            else if((4-used_cards[4][j]-own_num[j])<=(pattern[i][1]-1)){//�o�����\���Ȃ�
              //���炳�Ȃ�
            }
            else{
              k=(4-used_cards[4][j]-own_num[j])-pattern[i][1]-p+5;
              if(p==2){
                pattern[i][n]-=30;
              }
              else if(k<=0){pattern[i][n]-=4;}
              else if(k==1){pattern[i][n]-=9;}
              else if(k==2){pattern[i][n]-=15;}
              else{pattern[i][n]-=24;}
            }
          }
        }
      }
      if(pattern[i][n]<=0){
        pattern[i][n]=1;
      }
      if(pattern[i][0]==6){//8�؂�
        pattern[i][n]+=100;
      }
    }
  }//x�v�Z�����܂�

  for(i=0;i<11;i++){
    if(n==4){//1min,2min�̌���
      if(i==0){
        pattern[11][2]=pattern[i][4];
        pattern[11][3]=201;
      }
      else if(pattern[i][4]==-1 || (pattern[i][3]==14 && pattern[i][2]==0) ){//�W���[�J�[�P��
      }
      else{
        if(pattern[i][4]<pattern[11][2]){
          pattern[11][3]=pattern[11][2];
          pattern[11][2]=pattern[i][4];
        }
        else if(pattern[i][4]<pattern[11][3]){
          pattern[11][3]=pattern[i][4];
        }
      }
    }
    if(n==5){//rev2m�̌���
      if(i==0){
        j=pattern[i][5];
        pattern[11][6]=201;
      }
      else if(pattern[i][4]>=STRONG && pattern[i][4]>pattern[i][5] ){//x>=STRONG ���� x>revx
      }
      else if(pattern[i][1]>=5 || (pattern[i][1]==4 && (pattern[i][3]==0 || pattern[i][3]==14))){//�v����
      }
      else{
        if(pattern[i][5]<j){
          pattern[11][6]=j;
          j=pattern[i][5];
        }
        else if(pattern[i][5]<pattern[11][6]){
          pattern[11][6]=pattern[i][5];
        }
      }
    }
    //z,revz�̌���
    if(i==0){
      pattern[11][n]=0;
    }
    if(i!=13){
      if(pattern[i][1]<=3 && pattern[i][1]!=0){
        if(pattern[i][n]>=1 && pattern[i][n]<=30){
          pattern[11][n]+=2;
        }
        else if(pattern[i][n]>=31 && pattern[i][n]<=60){
          pattern[11][n]+=1;
        }
        else if(pattern[i][n]>=61 && pattern[i][n]<=90){
          //pattern[11][n]+=0;
        }
        else if(pattern[i][n]>=91 && pattern[i][n]<=200){
          if(rev==0 && pattern[i][0]>=max){//��ԋ�������
            pattern[11][n]-=pattern[i][1];
          }
          else if(rev==1 && pattern[i][0]<=max){
            pattern[11][n]-=pattern[i][1];
          }
          else{
            pattern[11][n]-=1;
          }
        }
      }
      if(pattern[i][3]==14 && pattern[i][2]==0){//�W���[�J�[
        pattern[11][n]-=2;
      }
    }
  }
}

void cardprint(int own_cards[8][15]){
	int i,j;
	for(j=0;j<15;j++){
		for(i=0;i<4;i++){
			if(own_cards[i][j]>=1){
				printf(" ");
				if(own_cards[i][j]==2){
					printf("[jk]");
				}
				switch(i){
				case 0:printf("s");break;
				case 1:printf("h");break;
				case 2:printf("d");break;
				case 3:printf("c");break;
				}
				if(j>=1 && j<=8)printf("%d",j+2);
				else if(j==9)printf("J");
				else if(j==10)printf("Q");
				else if(j==11)printf("K");
				else if(j==12)printf("A");
				else if(j==13)printf("2");
				else if(j==0)printf("3-");
				else if(j==14)printf("2+");
			}
		}
	}
	if(own_cards[4][1]==2){
		printf(" jk");
	}
	printf("\n");
}

void Tableprint(int own_cards[8][15],int print){
	//print 0:�J�[�h�e�[�u���̂� 1:��Ԃ��\��
	int i,j,k;
	k=5;
	if(print==1){
		k=7;
	}
	  fprintf( stderr, "    [3-][ 3][ 4][ 5][ 6][ 7][ 8][ 9][10][ J][ Q][ K][ A][ 2][2+]\n");
      for(i=0;i<k;i++){
        if(i==0){
          fprintf( stderr, "[ S]");
        }
        if(i==1){
          fprintf( stderr, "[ H]");
        }
        if(i==2){
          fprintf( stderr, "[ D]");
        }
        if(i==3){
          fprintf( stderr, "[ C]");
        }
        if(i==4){
          fprintf( stderr, "[jk]");
        }
        if(i==5){
          fprintf( stderr, "[ba]");
        }
        if(i==6){
          fprintf( stderr, "[ps]");
        }
        for(j=0;j<=14;j++){
          if(own_cards[i][j]==0){fprintf( stderr, "    ");}
          else{
            fprintf( stderr, " %2d ",own_cards[i][j]);
          }
        }
        fprintf( stderr, "\n");
      }
}

//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//--------------------------------�V�����֐�----------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//
//----------------------------------------------------------------------------------//

//���荞��2000�s�t��

//-------------------------------------------------------------------------------------
//����邩�ǂ�����Kou�����]���l�Ŕ��f���[�h��2
#define kou_e_value_mode 2
//�c��N���C�A���g��
#define remaining_client_num 3 

//1�Łi�P�j�̃��[�h�A�Q�Łi�Q�j�̃��[�h
#define hand_num_switch 2
//�i�P�j�c���Ă�N���C�A���g�̒��ł̍ő��D
#define remaining_client_max_hand_num 6
//�i�Q�j�O�̃N���C�A���g�̎c���D�̏��
#define front_client_max_hand_num 6
//��D�������ς�(�O�̃N���C�A���g�̊K�����Ⴂ�Ƃ���D�����ɘa)�i�i�Q�j�̃��[�h�̂݁j�i�P�F�ρA�O�F��ρj)
#define hand_variable 1

//����邩�ǂ������f�E�P�i����(9=J,10=Q,11=K)
#define ba_nagare_single_lock 11
//����邩�ǂ������f�E�y�A����(6=8,7=9,8=10,9=J,10=Q,11=K)
#define ba_nagare_double_lock 8
//����邩�ǂ����̔��f���[�h�ύX�p�i�P�F��ρA�Q�F�ρj
#define ba_nagare_mode 2
//��̏󋵁A��o�����A�����̎�D�@���猻�݂̏�ɂ����Ď������ŋ��̃J�[�h�������Ă���Ƃ�
//single �ʏ�\���S�A�ɘa�\���R
//double �ʏ�\��3�ȏ�A�ɘa�\��
#define ba_nagare_max_limit_single 3
#define ba_nagare_max_limit_double 3

//�O�̃N���C�A���g���ǂ��܂Ō��邩�̃��[�h�I���i�P�F1�l�O�̂݁A�Q�F1�l�O��2�l�O�j
#define front_client_mode 1

//�O�̃N���C�A���g�̊K���������ł���Ƃ��N�����Ȃ��i�P�ŋ@�\�A�O�ł��̂܂܁j
#define front_client_rank 1

//��̎���o�������̂������̎����Ă���Œ჉���N����
//2���ڈȏ�̃����N�������ꍇ->2
//1���ڂ��傫���@���@2���ڂ�菬���������N�������ꍇ->1
//1���ڈȏ�̏ꍇ->3
#define front_client_log_st_off 3

//�ȏ��헪�𔭓����Ȃ����[�h�i�P�F�p�X���Ȃ��A�O�F�p�X����j
#define st_no_log_mode 0
//-------------------------------------------------------------------------------------

//�O�̃N���C�A���g�̗����i���̏�̎����̃J�[�h�o�����A�p�X�����j�����𗬂������Ɏ����������Ă���J�[�h���ア�J�[�h���o���邩�̊֐�����
//	��������ۂ��@�\�Ƃ��đO�̃N���C�A���g���ꂪ��̎��ɏo�����J�[�h�̋����ɂ���ăp�X���邩�ǂ�����ς���
//	�@�Q�[�����ƂɑO�̃N���C�A���g���ꂪ��̏�Ԃŏo�����������i�[����(1���A2�����ꂼ��ۑ�)
//	����̏�Ԃ̎��ɏo���������N�������̎����Ă���Œ჉���N�{�Ȃ�ڂ��@�Ȃǋ@�\�����̒�`����I�i�����]���l�g���̂���H�j
//    ��̏�Ԃ̎��ɏo���������N�������̎�D�ŕK�v�Ȃ����̂�����o�������ɂȂ��Ȃ�p�X���Ȃ�
//
//	��̎���o�������̂������̎����Ă���Œ჉���N����2���ڂ��傫���̃����N�������ꍇ���p�X���Ȃ�

//�O�̃N���C�A���g�̎�D�����̂ݍl��
//	->�O�̃N���C�A���g�i������D�i�����₷���g�j�����Ă�j�ɑ΂��Ă̂ݔ����ł���
//		->��x���̊m����������
//	->�O�̃N���C�A���g�ɑ΂��Ă̂ݔ�������̂ő��̃N���C�A���g�̎�D�����͖���
//		->�n���E��n���̊m����������i�S���̃N���C�A���g�̎�D����������ƕ����ɂ����Ȃ�H�j

//1000*10�̎����񐔂ł͈��̃��O�����Ȃ�
// ->����ŗ����������f�[�^�ɂȂ邩����

//�p�X����ق������тɃo���c�L�����遨�����������Ǝォ�������̍������遨���ƂȂ錴����������
//������T��̂ɂǂ�ȃf�[�^���K�v�H�������𗧂Ăā�����

//�o��������
//
//�����P�F�����D����������ꍇ�i���̏�Ŏ������o���Ȃ��j
//�����Q�F���肪�オ�낤�Əo���ɍs���^�C�~���O�ƁA�O�̃N���C�A���g�̎c���D������}�b�`���ĂȂ�(front_client_max_hamd_num)
//�@�@�@�@����̊K���������قǏ�������͑����ݒ肷�ׂ��H�i��D�����Əオ���D�ɂȂ�^�C�~���O���������j���ݒ�ς݁i�^�C�~���O�͖����؁j
//
//

//�ȏ���p(kou_follow_group�ɉ�����Ă���̂ŕ��������o���������֗^���ĂȂ�)
int seat_tactics(int own_cards[8][15], int my_player_num, int seat_order[5], int last_playnum, int used_cards[8][15], int game_count)
{
	////////////////////////////////////////////////////////
	//�ȏ���p�𔭓�����Ƃ��Ԃ�l�P�A�������Ȃ����Ԃ�l�O//
	////////////////////////////////////////////////////////

	int i, j;


	if (remaining_client() <= remaining_client_num) {
		//�N���C�A���gremaining_client_num�ȉ��̎��\�\�N�����Ȃ�
		seat_tactics_check_off++;
		return 0;
	}
	else if (remaining_client_max_hand(my_player_num) <= remaining_client_max_hand_num && hand_num_switch==1) {
		//�c���Ă�N���C�A���g�̎�D�����̍ő�l��remainig_client_max_hand_num�ȉ��̎��\�\�N�����Ȃ�
		//�v����ɂ��ꂩ�オ�肻���ȂƂ��͂�߂Ƃ�
		seat_tactics_check_off++;
		return 0;
	}
	else if (front_client_hand_num_check(own_cards, my_player_num, seat_order, last_playnum) <= front_client_max_hand_num && hand_num_switch == 2) {
		//�O�̃N���C�A���g�̎�D������front_client_max_hand_num�ȉ��̎��\�\�N�����Ȃ�
		seat_tactics_check_off++;
		return 0;
	}
	else if (seat_order_check(own_cards, my_player_num, seat_order, last_playnum) == 1) {
		if (kou_e_value_mode == 1) {
			//�����̗��ꂻ���֐�
			if (ba_nagare(own_cards, seat_order, used_cards) == 1) {
			//�ꂪ���ꂻ���Ȃ�
				if (front_client_log_st(own_cards,my_player_num,seat_order,last_playnum,used_cards,game_count) == 0) {
				//�O�̃N���C�A���g�̒�o���O������v�����ȂƂ�
					seat_tactics_check_on++;
					seat_tactics_check[game_count][0]++;
					seat_tactics_flag = 1;
					ba_nagare_last_playernum = last_playnum;

					if (st_no_log_mode == 1) {
						return 0;
					}
					else {
						return 1;
					}
				}
				else if (front_client_log_st(own_cards, my_player_num, seat_order, last_playnum, used_cards, game_count) == 1){
					client_log_st_pass_off_count++;
				}
			}
		}
		else if (kou_e_value_mode == 2) {
			//kou�̋����]���l
			if (kou_e_value(own_cards, seat_order, used_cards) >= 86) {
				if (front_client_log_st(own_cards, my_player_num, seat_order, last_playnum, used_cards, game_count) == 0) {
				//�O�̃N���C�A���g�̒�o���O������v�����ȂƂ�
					seat_tactics_check_on++;
					seat_tactics_check[game_count][0]++;
					seat_tactics_flag = 1;
					ba_nagare_last_playernum = last_playnum;

					if (st_no_log_mode == 1) {
						return 0;
					}
					else {
						return 1;
					}
				}
				else if (front_client_log_st(own_cards, my_player_num, seat_order, last_playnum, used_cards, game_count) == 1) {
					client_log_st_pass_off_count++;
				}
			}
		}

		seat_tactics_check_off++;
		seat_tactics_flag = 0;
		return 0;
	}


	return 0;
}

//�O�̃N���C�A���g����̏�ɒ�o�������̃��O��p���āAST�𑱍s���邩���肷��
int front_client_log_st(int own_cards[8][15], int my_player_num, int seat_order[5], int last_playnum, int used_cards[8][15], int game_count)
{
	int i, j, count = 0, seat_order_remainig[5] = { 5,5,5,5,5 };

	for (i = 0; i < 5; i++)
	{
		//�オ���ĂȂ��N���C�A���g�݂̂��i�[���Ă���
		if (state.player_qty[seat_order[i]] != 0) {
			seat_order_remainig[count] = seat_order[i];
			count++;
		}
	}

	int own_cards_limit_1 = 0, own_cards_limit_2 = 0, own_cards_limit_flag = 1, front_client_most_weak = 0, count_own_qty = 0, count_enemy_qty = 0;
	int continue_falg = 0;

	
	if (state.rev == 0) {
		//qyt==1
		for (j = 1; j <= 13; j++) {
			for (i = 0; i < 4; i++) {
				if (own_cards[i][j] == 1 && own_cards_limit_flag == 1) {
					own_cards_limit_1 = j;
					own_cards_limit_flag++;
					continue_falg++;

				}

				if (client_log[0][i][j][seat_order_remainig[count - 1]] == 1) {
					front_client_most_weak = j;
				}

				if (continue_falg == 1) {
					continue_falg--;
					continue;
				}

				if (own_cards[i][j] == 1 && own_cards_limit_flag == 2) {
					own_cards_limit_2 = j;
					own_cards_limit_flag++;
				}
			}
			
			if (own_cards_limit_flag >= 3) {
				break;
			}
		}

		if (front_client_log_st_off == 2) {
			if (own_cards_limit_2 < front_client_most_weak) {
				return 1;
			}
		}
		else if (front_client_log_st_off == 1) {
			if (own_cards_limit_2 > front_client_most_weak && own_cards_limit_1 < front_client_most_weak) {
				return 1;
			}
		}
		else if (front_client_log_st_off == 3) {
			if (own_cards_limit_1 <= front_client_most_weak) {
				if (own_cards_limit_1 != 0 && own_cards_limit_2 != 0 && front_client_most_weak != 0) {
					/*printf("*------------------*\n");
					printf("own_cards_limit_1 : %d\n", own_cards_limit_1);
					printf("own_cards_limit_2 : %d\n", own_cards_limit_2);
					printf("front_client_most_weak : %d\n", front_client_most_weak);
					printf("*------------------*\n\n");*/

					return 1;
				}
			}
		}

		continue_falg = 0;
		own_cards_limit_1 = 0;
		own_cards_limit_2 = 0;
		own_cards_limit_flag = 1;
		front_client_most_weak = 0;
		//qty=2
		for (j = 1; j <= 13; j++) {
			for (i = 0; i < 4; i++) {
				if (own_cards[i][j] == 1) {
					count_own_qty++;
				}
				if (client_log[1][i][j][seat_order_remainig[count - 1]] == 1) {
					count_enemy_qty++;
				}
			}

			if (count_own_qty >= 2 && own_cards_limit_flag == 1) {
				own_cards_limit_1 = j;
				own_cards_limit_flag++;
				continue_falg++;
			}

			if (count_enemy_qty >= 2) {
				front_client_most_weak = j;
			}

			if (continue_falg == 1) {
				continue_falg--;
				continue;
			}

			if (count_own_qty >= 2 && own_cards_limit_flag == 2) {
				own_cards_limit_2 = j;
				own_cards_limit_flag++;
			}

			if (own_cards_limit_flag >= 3) {
				break;
			}

			count_own_qty = 0;
			count_enemy_qty = 0;
		}
	}
	else if (state.rev == 1) {
		//qty=1
		for (j = 13; j >= 1; j--) {
			for (i = 0; i < 4; i++) {
				if (own_cards[i][j] == 1 && own_cards_limit_flag == 1) {
					own_cards_limit_1 = j;
					own_cards_limit_flag++;
					continue_falg++;
				}

				if (client_log[0][i][j][seat_order_remainig[count - 1]] == 1) {
					front_client_most_weak = j;
				}
				
				if (continue_falg == 1) {
					continue_falg--;
					continue;
				}
				
				if (own_cards[i][j] == 1 && own_cards_limit_flag == 2) {
					own_cards_limit_2 = j;
					own_cards_limit_flag++;
				}
			}
				
			if (own_cards_limit_flag >= 3) {
				break;
			}
		}

		if (front_client_log_st_off == 2) {
			if (own_cards_limit_2 < front_client_most_weak) {
				return 1;
			}
		}
		else if (front_client_log_st_off == 1) {
			if (own_cards_limit_2 > front_client_most_weak && own_cards_limit_1 < front_client_most_weak) {
				return 1;
			}
		}
		else if (front_client_log_st_off == 3) {
			if (own_cards_limit_1 <= front_client_most_weak) {
				if (own_cards_limit_1 != 0 && own_cards_limit_2 != 0 && front_client_most_weak != 0) {
					/*printf("*------------------*\n");
					printf("own_cards_limit_1 : %d\n", own_cards_limit_1);
					printf("own_cards_limit_2 : %d\n", own_cards_limit_2);
					printf("front_client_most_weak : %d\n", front_client_most_weak);
					printf("*------------------*\n\n");*/

					return 1;
				}
			}
		}

		continue_falg = 0;
		own_cards_limit_1 = 0;
		own_cards_limit_2 = 0;
		own_cards_limit_flag = 1;
		front_client_most_weak = 0;
		//qty=2
		for (j = 13; j >= 1; j--) {
			for (i = 0; i < 4; i++) {
				if (own_cards[i][j] == 1) {
					count_own_qty++;
				}
				if (client_log[1][i][j][seat_order_remainig[count - 1]] == 1) {
					count_enemy_qty++;
				}
			}

			if (count_own_qty >= 2 && own_cards_limit_flag == 1) {
				own_cards_limit_1 = j;
				own_cards_limit_flag++;
			}

			if (count_enemy_qty >= 2) {
				front_client_most_weak = j;
			}

			if (continue_falg == 1) {
				continue_falg--;
				continue;
			}

			if (count_own_qty >= 2 && own_cards_limit_flag == 2) {
				own_cards_limit_2 = j;
				own_cards_limit_flag++;
			}

			if (own_cards_limit_flag >= 3) {
				break;
			}

			count_own_qty = 0;
			count_enemy_qty = 0;
		}
	}
	

	if (front_client_log_st_off == 2) {
		if (own_cards_limit_2 < front_client_most_weak) {
			return 1;
		}
	}
	else if (front_client_log_st_off == 1) {
		if (own_cards_limit_2 > front_client_most_weak && own_cards_limit_1 < front_client_most_weak) {
			return 1;
		}
	}
	else if (front_client_log_st_off == 3) {
		if (own_cards_limit_1 <= front_client_most_weak) {
			if (own_cards_limit_1 != 0 && own_cards_limit_2 != 0 && front_client_most_weak != 0) {
				/*printf("*------------------*\n");
				printf("own_cards_limit_1 : %d\n", own_cards_limit_1);
				printf("own_cards_limit_2 : %d\n", own_cards_limit_2);
				printf("front_client_most_weak : %d\n", front_client_most_weak);
				printf("*------------------*\n\n");*/

				return 1;
			}
		}
	}
	
	/*printf("own_cards_limit_1 : %d\n", own_cards_limit_1);
	printf("own_cards_limit_2 : %d\n", own_cards_limit_2);
	printf("front_client_most_weak : %d\n\n", front_client_most_weak);*/

	//return 0�ł��̂܂܃p�X�A1�Ńp�X���Ȃ�
	return 0;
}

//�����̑O�̐Ȃ̃N���C�A���g����ɍŌ�ɏo�����Ƃ��A�Ԃ�l�P
int seat_order_check(int own_cards[8][15], int my_player_num, int seat_order[5], int last_playnum)
{
	//�����Ă鏇�Ԃ��i�[����z��[->�������Z���Z���Z���Z-]
	int i, count = 0, seat_order_remainig[5] = {5,5,5,5,5};

	for (i = 0; i < 5; i++)
	{
		//�オ���ĂȂ��N���C�A���g�݂̂��i�[���Ă���
		if (state.player_qty[seat_order[i]] != 0) {
			seat_order_remainig[count] = seat_order[i];
			count++;
		}
	}

	if (front_client_rank == 1) {
		//�O�̃N���C�A���g�̊K���������̎��N�����Ȃ�
		if (seat_order_remainig[count - 1] == last_playnum && (state.player_rank[seat_order_remainig[count - 1]] == 2)) {
			return 0;
		}
	}

	if (front_client_mode == 1) {
		//1�l�O�̂�
		if (seat_order_remainig[count-1] == last_playnum) {
			return 1;
		}
	}else if (front_client_mode == 2) {
		//1�l�O��2�l�O
		if (seat_order_remainig[count - 1] == last_playnum || seat_order_remainig[count - 2] == last_playnum) {
			return 1;
		}
	}

	return 0;
}

//��̏�Ԃ���p�X������𓾂Ȃ��Ƃ��Ԃ�l�P
int pass_check(int own_cards[8][15], int ba_cards[8][15])
{
	if (state.qty == 1) {
		//�P�i
		if (pass_check_solo(own_cards, ba_cards) == 1) {
			return 0;
		}
	}
	else if (state.qty >= 2 && state.sequence == 0) {
		//�������i�K�i�łȂ��j
		if (pass_check_group(own_cards, ba_cards) == 1) {
			return 0;
		}
	}
	else if (state.qty >= 3 && state.sequence == 1) {
		//�K�i
		if (pass_check_kaidan(own_cards, ba_cards) == 1) {
			return 0;
		}
	}

	return 1;
}

//�p�X�`�F�b�N�Q�P�i�i�o���J�[�h�����ĂȂ���ΕԂ�l�O�j�A�i�W���[�J�[���l���j
int pass_check_solo(int own_cards[8][15], int ba_cards[8][15])
{
	int i, j, lock_flag=5, rev_flag=0;

	//���΂�Ȃ�A�X�[�g���i�[
	if (state.lock == 1) {
		for (i = 0; i < 5; i++) {
			if (state.suit[i] == 1) {
				lock_flag = i;
			}
		}
	}
	//�v���Ȃ�P
	if (state.rev == 1) {
		rev_flag = 1;
	}

	if (rev_flag == 0) {
		for (j = state.ord + 1; j <= 13; j++)
		{
			for (i = 0; i < 4; i++)
			{
				//���΂�Ŏ����Ă�
				if (lock_flag != 5) {
					if (own_cards[lock_flag][j] == 1) {
						return 1;
					}
					else {
						break;
					}
				}

				//���ʂɎ����Ă�
				if (own_cards[i][j] == 1) {
					return 1;
				}
			}
		}
	}
	else if (rev_flag == 1) {
		for (j = state.ord - 1; j >= 1; j--)
		{
			for (i = 0; i < 5; i++)
			{
				//���΂�Ŏ����Ă�
				if (lock_flag != 5) {
					if (own_cards[lock_flag][j] == 1) {
						return 1;
					}
					else {
						break;
					}
				}

				//���ʂɎ����Ă�
				if (own_cards[i][j] == 1) {
					return 1;
				}
			}
		}
	}

	return 0;
}

//�p�X�`�F�b�N�Q�����i�K�i�łȂ��j
int pass_check_group(int own_cards[8][15], int ba_cards[8][15])
{
	int i, j, k, lock_flag[4] = {5,5,5,5} , lock_flag_count=0, rev_flag = 0;
	int group_count = 0;

	//���΂�Ȃ�A�X�[�g���i�[
	if (state.lock == 1) {
		for (i = 0; i < 4; i++) {
			if (state.suit[i] == 1) {
				lock_flag[lock_flag_count] = i;
				lock_flag_count++;
			}
		}
	}
	//�v���Ȃ�P
	if (state.rev == 1) {
		rev_flag = 1;
	}

	if (rev_flag == 0) {
		for (j = state.ord + 1; j <= 13; j++)
		{
			for (i = 0; i < 4; i++)
			{
				//���΂�Ŏ����Ă�
				if (state.lock == 1) {
					for (k = 0; k < lock_flag_count; k++) {
						if (lock_flag[k] = i) {
							if (own_cards[i][j] == 1) {
								group_count++;
								continue;
							}
						}
					}

					continue;
				}

				//�����Ă�
				if (own_cards[i][j] == 1) {
					group_count++;
				}
			}

			if (group_count == state.qty) {
				return 1;
			}
			else {
				group_count = 0;
			}
		}
	}
	else if (rev_flag == 1) {
		for (j = state.ord - 1; j >= 1; j--)
		{
			for (i = 0; i < 4; i++)
			{
				//���΂�Ŏ����Ă�
				if (state.lock == 1) {
					for (k = 0; k < lock_flag_count; k++) {
						if (lock_flag[k] = i) {
							if (own_cards[i][j] == 1) {
								group_count++;
								continue;
							}
						}
					}

					continue;
				}

				//�����Ă�
				if (own_cards[i][j] == 1) {
					group_count++;
				}
			}

			if (group_count == state.qty) {
				return 1;
			}
			else {
				group_count = 0;
			}
		}
	}

	return 0;
}

//�p�X�`�F�b�N�Q�K�i
int pass_check_kaidan(int own_cards[8][15], int ba_cards[8][15])
{
	int i, j, k, rev_flag = 0, qty_count = 0;
	int min_kaidan = 0, max_kaidan = 0;

	//�v���Ȃ�P
	if (state.rev == 1) {
		rev_flag = 1;
	}

	//�K�i�̍ŏ������N�ƍő僉���N���i�[
	for (i = 0; i < 4; i++)
	{
		for (j = 1; j <= 13; j++)
		{
			if (ba_cards[i][j] == 1 || ba_cards[i][j] == 2) {
				if (qty_count == 0) {
					//�ŏ������N
					min_kaidan = j;
				}
				else if (qty_count == (state.qty - 1)) {
					//�ő僉���N
					max_kaidan = j;
				}

				qty_count++;
			}
		}
	}

	qty_count = 0;

	if (rev_flag == 0) {
		for (i = 0; i < 4; i++)
		{
			for (j = max_kaidan + 1; j <= 13; j++)
			{
				if (own_cards[i][j] == 1) {
					qty_count++;
				}
			}

			if (qty_count > state.qty) {
				return 1;
			}
			else {
				qty_count = 0;
			}
		}
	}
	else if (rev_flag == 1) {
		for (i = 0; i < 4; i++)
		{
			for (j = min_kaidan - 1; j >= 1; j--)
			{
				if (own_cards[i][j] == 1) {
					qty_count++;
				}
			}

			if (qty_count > state.qty) {
				return 1;
			}
			else {
				qty_count = 0;
			}
		}
	}

	return 0;
}

//�c��N���C�A���g�̐l����Ԃ�(�����܂�)
int remaining_client(void)
{
	int i, count = 0;

	for (i = 0; i < 5; i++)
	{
		//��D�Ȃ��l�J�E���g
		if (state.player_qty[i] == 0)
		{
			count++;
		}
	}

	if (status_print == 1) {
		printf("----------remaining_client:%d-----------\n", (5-count) );
	}

	return (5 - count);
}

//�c���Ă���N���C�A���g�̒��ōő�ƂȂ��D������Ԃ�(�����܂߂�)
int remaining_client_max_hand(int my_player_num)
{ 
	int i , client_max_hand = 0 ;

	for (i = 0; i < 5 ; i++)
	{
		if (i == my_player_num) {
			continue;
		}
		else {
			//�ő�Ȃ�X�V
			if (state.player_qty[i] > client_max_hand) {
				client_max_hand = state.player_qty[i];
			}
		}

	}

	if (status_print == 1) {
		printf("----------remaining_client_max_hand:%d-----------\n",client_max_hand);
	}

	return client_max_hand;
}

//�O�̃N���C�A���g�̎�D������Ԃ�
int front_client_hand_num_check(int own_cards[8][15], int my_player_num, int seat_order[5], int last_playnum)
{
	//�����Ă鏇�Ԃ��i�[����z��[->�������Z���Z���Z���Z-]
	int i, count = 0, seat_order_remainig[5] = { 5,5,5,5,5 };

	for (i = 0; i < 5; i++)
	{
		//�オ���ĂȂ��N���C�A���g�݂̂��i�[���Ă���
		if (state.player_qty[seat_order[i]] != 0) {
			seat_order_remainig[count] = seat_order[i];
			count++;
		}
	}

	if (hand_variable == 0) {
		return (state.player_qty[seat_order_remainig[count - 1]]);
	}
	else if(hand_variable == 1){
		if (front_client_rank == 0) {
			//�O����������ST�N�����Ȃ��\��OFF
			if (state.player_rank[seat_order_remainig[count - 1]] == 2) {
				//�����O�̃N���C�����������������D����������1���ɘa
				return (state.player_qty[seat_order_remainig[count - 1]] + 1);
			}
			else if (state.player_rank[seat_order_remainig[count - 1]] == 3 || state.player_rank[seat_order_remainig[count - 1]] == 4) {
				//�����O�̃N���C�����n��or��n�����������D�����������Q���ɘa			
				return (state.player_qty[seat_order_remainig[count - 1]] + 2);
			}
		}
		else if (front_client_rank == 1) {
			//�O�̃N���C������������ST�N�����Ȃ���ON�̂Ƃ�
			if (state.player_rank[seat_order_remainig[count - 1]] == 3) {
				//�����O�̃N���C�����n�����������D����������1���ɘa
				return (state.player_qty[seat_order_remainig[count - 1]] + 1);
			}
			else if (state.player_rank[seat_order_remainig[count - 1]] == 4) {
				//�����O�̃N���C������n�����������D�����������Q���ɘa			
				return (state.player_qty[seat_order_remainig[count - 1]] + 2);
			}
		}
	}

	return (state.player_qty[seat_order_remainig[count - 1]]);
}

//�c���Ă���N���C�A���g�̒��ōŏ��ƂȂ��D������Ԃ�(�����܂߂�)
int remaining_client_min_hand(int my_player_num)
{
	int i, client_min_hand = 0,flag=0;

	for (i = 0; i < 5; i++)
	{
		if (i == my_player_num) {
			continue;
		}
		else {
			//�ő�Ȃ�X�V
			if (flag==0) {
				flag = 1;
				client_min_hand = state.player_qty[i];
			}

			if (state.player_qty[i] < client_min_hand) {
				client_min_hand = state.player_qty[i];
			}
		}

	}

	if (status_print == 1) {
		printf("----------remaining_client_max_hand:%d-----------\n", client_min_hand);
	}

	return client_min_hand;
}

//�ꂪ���ꂻ���Ȃ�P�A���ꂻ���ɂȂ��Ȃ�O
int ba_nagare(int own_cards[8][15], int seat_order[5], int used_cards[8][15])
{
	int i, j;

	//�����̑O�̐Ȃ̋@�\�����

	if (ba_nagare_max(own_cards, used_cards) >= 1) {
		//��������ɂ�����ŋ��̃J�[�h�����Ă�
		if (status_print == 1) {
			printf("----------ba_nagare_max <ON>-----------\n");
		}
		ba_nagare_max_count++;
		return 1;
	}

	if (ba_nagare_mode == 1)
	{
		if (state.rev == 0) {
			//�v���łȂ�
			if (state.ord >= ba_nagare_single_lock && state.qty == 1 && state.lock == 1) {
				//�P�̂��΂�ŗ��ꂻ�������f
				if (status_print == 1) {
					printf("----------ba_nagare_single_lock <ON>-----------\n");
				}
				ba_nagare_single++;
				return 1;
			}
			else if (state.ord >= ba_nagare_double_lock && state.qty == 2 && state.lock == 1) {
				//�y�A���΂�ŗ��ꂻ�������f
				if (status_print == 1) {
					printf("----------ba_nagare_double_lock <ON>-----------\n");
				}
				ba_nagare_double++;
				return 1;
			}
			else if (state.qty >= 3) {
				//3�J�[�h�ȏ�̑g�i3�K�[�h�E�K�i�Ȃǁj�͗��ꂻ��
				if (status_print == 1) {
					printf("----------ba_nagare_three_card_or_kaidan <ON>-----------\n");
				}
				ba_nagare_three_cards++;
				return 1;
			}
		}
		else if (state.rev == 1) {
			//�v����
			if (state.ord <= (ba_nagare_single_lock - 8) && state.qty == 1 && state.lock == 1) {
				//�P�̂��΂�ŗ��ꂻ�������f
				if (status_print == 1) {
					printf("----------ba_nagare_single_lock <ON>-----------\n");
				}
				ba_nagare_single++;
				return 1;
			}
			else if (state.ord <= (ba_nagare_double_lock - 4) && state.qty == 2 && state.lock == 1) {
				//�y�A���΂�ŗ��ꂻ�������f
				if (status_print == 1) {
					printf("----------ba_nagare_double_lock <ON>-----------\n");
				}
				ba_nagare_double++;
				return 1;
			}
			else if (state.qty >= 3) {
				//3�J�[�h�ȏ�̑g�i3�K�[�h�E�K�i�Ȃǁj�͗��ꂻ��
				if (status_print == 1) {
					printf("----------ba_nagare_three_card_or_kaidan <ON>-----------\n");
				}
				ba_nagare_three_cards++;
				return 1;
			}
		}
	}
	else if (ba_nagare_mode == 2)
	{
		//�ꂪ�����σ��[�h�i�������͊֐����Ŋv���̕�����s���Ă���j

		if (state.qty == 1 && state.lock == 1) {
			if (ba_nagare_single_variable(own_cards, seat_order, used_cards) == 1) {
				if (status_print == 1) {
					printf("----------ba_nagare_single_lock <ON>-----------\n");
				}
				ba_nagare_single++;
				return 1;
			}
		}
		else if (state.qty == 2 && state.lock == 1) {
			if (ba_nagare_double_variable(own_cards, seat_order, used_cards) == 1) {
				if (status_print == 1) {
					printf("----------ba_nagare_double_lock <ON>-----------\n");
				}
				ba_nagare_double++;
				return 1;
			}
		}
		else if (state.qty >= 3) {
			//3�J�[�h�ȏ�̑g�i3�K�[�h�E�K�i�Ȃǁj�͗��ꂻ��
			if (status_print == 1) {
				printf("----------ba_nagare_three_card_or_kaidan <ON>-----------\n");
			}
			ba_nagare_three_cards++;
			return 1;
		}
	}

	if (status_print == 1) {
		printf("----------ba_nagare <OFF>-----------\n");
	}
	return 0;
}

//����邩�ǂ����̔��f�̊�idefine�j���ςɁi�P�i�j
int ba_nagare_single_variable(int own_cards[8][15], int seat_order[5], int used_cards[8][15])
{
	int ba_nagare_single_lock_variable = ba_nagare_single_lock;
	float a, b;

	a = now_cards_ord_used(used_cards,own_cards);  //���ݏ�ɏo�Ă���J�[�h��苭�������N�̃J�[�h�̖���o�����O(�g��ꂽ��Ǝ�����)
	b = now_cards_ord(used_cards);  //���ݏ�ɏo�Ă���J�[�h��苭�������N�̃J�[�h�̑���

	//a / b�͏��߂͂��Ȃ菬���������A�㔼�قǑ傫���Ȃ�(�l��:0~1)

	if (b != 0)
	{
		if (state.rev == 0) {
			if (a / b < 0.25) {
				if (state.ord >= ba_nagare_single_lock_variable) {
					return 1;
				}
			}
			else if (a / b < 0.33) {
				if (state.ord >= ba_nagare_single_lock_variable - 1) {
					return 1;
				}
			}
			else if (a / b < 0.5) {
				if (state.ord >= ba_nagare_single_lock_variable - 2) {
					return 1;
				}
			}
			else {
				if (state.ord >= ba_nagare_single_lock_variable - 3) {
					return 1;
				}
			}
		}
		else if (state.rev == 1) {
			if (a / b < 0.25) {
				if (state.ord <= ba_nagare_single_lock_variable) {
					return 1;
				}
			}
			else if (a / b < 0.33) {
				if (state.ord <= ba_nagare_single_lock_variable - 1) {
					return 1;
				}
			}
			else if (a / b < 0.5) {
				if (state.ord <= ba_nagare_single_lock_variable - 2) {
					return 1;
				}
			}
			else {
				if (state.ord <= ba_nagare_single_lock_variable - 3) {
					return 1;
				}
			}
		}
	}

	return 0;
}

//����邩�ǂ����̔��f�̊�idefine�j���ςɁi�y�A�j
int ba_nagare_double_variable(int own_cards[8][15], int seat_order[5], int used_cards[8][15])
{
	int ba_nagare_double_lock_variable = ba_nagare_double_lock;
	int a, b;

	a = now_cards_ord_used(used_cards, own_cards);
	b = now_cards_ord(used_cards);

	if (b != 0)
	{
		if (state.rev == 0) {
			if (a / b < 0.25) {
				if (state.ord >= ba_nagare_double_lock_variable) {
					return 1;
				}
			}
			else if (a / b < 0.33) {
				if (state.ord >= ba_nagare_double_lock_variable - 1) {
					return 1;
				}
			}
			else if (a / b < 0.5) {
				if (state.ord >= ba_nagare_double_lock_variable - 2) {
					return 1;
				}
			}
			else {
				if (state.ord >= ba_nagare_double_lock_variable - 3) {
					return 1;
				}
			}
		}
		else if (state.rev == 0) {
			if (a / b < 0.25) {
				if (state.ord <= ba_nagare_double_lock_variable) {
					return 1;
				}
			}
			else if (a / b < 0.33) {
				if (state.ord <= ba_nagare_double_lock_variable - 1) {
					return 1;
				}
			}
			else if (a / b < 0.5) {
				if (state.ord <= ba_nagare_double_lock_variable - 2) {
					return 1;
				}
			}
			else {
				if (state.ord <= ba_nagare_double_lock_variable - 3) {
					return 1;
				}
			}
		}
	}

	return 0;
}

//���ݏ�ɏo�Ă���J�[�h��苭�������N�̃J�[�h�̓��C����o�J�[�h�̍��v
int now_cards_ord_used(int used_cards[8][15], int own_cards[8][15])
{
	int i, j, count = 0;

	if (state.rev == 0)
	{
		for (i = 0; i < 4; i++)
		{
			for (j = state.ord + 1; j <= 13; j++)
			{
				if (used_cards[i][j] != 1 && own_cards[i][j] != 1) {
					count++;
				}
			}
		}
	}
	else if (state.rev == 1)
	{
		for (i = 0; i < 4; i++)
		{
			for (j = state.ord - 1; j >= 1; j--)
			{
				if (used_cards[i][j] != 1 && own_cards[i][j] != 1) {
					count++;
				}
			}
		}
	}

	return count;
}

//���ݏ�ɏo�Ă���J�[�h��苭�������N�̃J�[�h�̑�����Ԃ�
int now_cards_ord(int used_cards[8][15])
{
	int i, j, count = 0;

	if (state.rev == 0)
	{
		for (i = 0; i < 4; i++)
		{
			for (j = state.ord + 1; j <= 13; j++)
			{
				count++;
			}
		}
	}
	else if (state.rev == 1)
	{
		for (i = 0; i < 4; i++)
		{
			for (j = state.ord - 1; j >= 1; j--)
			{
				count++;
			}
		}
	}

	return count;
}

//��̏󋵁A��o�����A�����̎�D�@���猻�݂̏�ɂ����Ď������ŋ��̃J�[�h�������Ă���Ƃ�
int ba_nagare_max(int own_cards[8][15], int used_cards[8][15])
{
	int ba_nagare_max_single_buf = 0, ba_nagare_max_double_buf = 0;

	if (state.qty == 1) {
		ba_nagare_max_single_buf = ba_nagare_max_single(own_cards, used_cards);
		if (ba_nagare_max_single_buf >= 1) {
			//�ꂪ�ꖇ�ōŋ������Ă�
			ba_nagare_max_count_single++;
			return ba_nagare_max_single_buf;
		}
	}
	else if (state.qty == 2) {
		ba_nagare_max_double_buf = ba_nagare_max_double(own_cards, used_cards);
		if (ba_nagare_max_double_buf >= 1) {
			//�ꂪ�񖇂ōŋ������Ă�
			ba_nagare_max_count_double++;
			return ba_nagare_max_double_buf;
		}
	}

	return 0;
}

//�ꂪ�ꖇ�i�Ԃ�l�̓����N�m1�`13�n�j
int ba_nagare_max_single(int own_cards[8][15], int used_cards[8][15])
{
	int i, j;
	int rank_count_used_cards = 0, rank_count_own_cards = 0, ba_suit;
	int flag = 0;

	if (state.rev == 0) {
		//�v���łȂ�
		if (state.lock == 0) {
			//����Ȃ�

			//�ォ��(���̎��ɁA�ł������z����)
			for (j = 13; j > state.ord; j--) {
				//�����N�̃��[�v
				for (i = 0; i < 4; i++) {
					//�X�[�g�̃��[�v
					if (used_cards[i][j] == 1) {
						//���̃����N�̒�o�ςݖ����𒲂ׂ�
						rank_count_used_cards++;
					}

					if (own_cards[i][j] == 1) {
						//���̃����N�̎����̎����Ă閇���𒲂ׂ�	
						rank_count_own_cards++;
					}
				}

				if (rank_count_used_cards == 4) {
					//�S�Ē�o�ς݂̃����N�Ȃ玟��
					rank_count_used_cards = 0;
					rank_count_own_cards = 0;
					continue;
				}
				else if (rank_count_used_cards + rank_count_own_cards >= ba_nagare_max_limit_single) {
					//�����݂̂��A���̃����N�̃J�[�h�������Ă���Ƃ�(�ォ�珇�ԂɌ��Ă���)
					flag = j;
					rank_count_used_cards = 0;
					rank_count_own_cards = 0;
					break;
				}
				else {
					return 0;
				}
			}

			//������(���̎��ɁA���������Ƃ�����)
			for (j = state.ord + 1; j <= flag; j++) {
				//�����N�̃��[�v
				for (i = 0; i < 4; i++) {
					//�X�[�g�̃��[�v
					if (used_cards[i][j] == 1) {
						//���̃����N�̒�o�ςݖ����𒲂ׂ�	
						rank_count_used_cards++;
					}

					if (own_cards[i][j] == 1) {
						//���̃����N�̎����̎����Ă閇���𒲂ׂ�	
						rank_count_own_cards++;
					}
				}

				if (rank_count_used_cards == 4) {
					//�S�Ē�o�ς݂̃����N�Ȃ玟��
					rank_count_used_cards = 0;
					rank_count_own_cards = 0;
					continue;
				}
				else if (rank_count_used_cards + rank_count_own_cards >= ba_nagare_max_limit_single) {
					//�����݂̂��A���̃����N�̃J�[�h�������Ă���Ƃ�(�����珇�ԂɌ��Ă���)
					if (j == flag) {
						ba_nagare_max_count_single_nomal++;
						return j;
					}
					else {
						rank_count_used_cards = 0;
						rank_count_own_cards = 0;
						continue;
					}
				}
				else {
					return 0;
				}
			}

			return 0;
		}
		else {
			//���肠��

			//��̃X�[�g�𒲂ׂ�
			for (i = 0; i < 4; i++) {
				if (state.suit[i] == 1) {
					//��̃X�[�g��ba_suit�Ɋi�[
					ba_suit = i;
				}
			}

			//�ォ��
			for (j = 13; j > state.ord; j--) {
				//�����N�̃��[�v
				if (used_cards[ba_suit][j] == 0 && own_cards[ba_suit][j] == 1) {
					//��o�ς݂łȂ��@���@�����������Ă�Ƃ�
					flag = j;
				}
				else if (used_cards[ba_suit][j] == 0 && own_cards[ba_suit][j] == 0) {
					//��o�ς݂łȂ��@���@�����������ĂȂ�
					return 0;
				}
			}

			//������
			for (j = state.ord + 1; j <= flag; j++) {
				//�����N�̃��[�v
				if (used_cards[ba_suit][j] == 0 && own_cards[ba_suit][j] == 1) {
					//��o�ς݂łȂ��@���@�����������Ă�Ƃ�
					if (j == flag) {
						ba_nagare_max_count_single_sibari++;
						return j;
					}
					else {
						continue;
					}
				}
				else if (used_cards[ba_suit][j] == 0 && own_cards[ba_suit][j] == 0) {
					//��o�ς݂łȂ��@���@�����������ĂȂ�
					return 0;
				}
			}

			return 0;
		}
	}
	else {
		//�v����
		if (state.lock == 0) {
			//����Ȃ�

			//�ォ��(���̎��ɁA�ł������z����)
			for (j = 1; j < state.ord; j++) {
				//�����N�̃��[�v
				for (i = 0; i < 4; i++) {
					//�X�[�g�̃��[�v
					if (used_cards[i][j] == 1) {
						//���̃����N�̒�o�ςݖ����𒲂ׂ�	
						rank_count_used_cards++;
					}

					if (own_cards[i][j] == 1) {
						//���̃����N�̎����̎����Ă閇���𒲂ׂ�	
						rank_count_own_cards++;
					}
				}

				if (rank_count_used_cards == 4) {
					//�S�Ē�o�ς݂̃����N�Ȃ玟��
					rank_count_used_cards = 0;
					rank_count_own_cards = 0;
					continue;
				}
				else if (rank_count_used_cards + rank_count_own_cards >= ba_nagare_max_limit_single) {
					//�����݂̂��A���̃����N�̃J�[�h�������Ă���Ƃ�(�ォ�珇�ԂɌ��Ă���)
					flag = j;
					rank_count_used_cards = 0;
					rank_count_own_cards = 0;
					break;
				}
				else {
					return 0;
				}
			}

			//������(���̎��ɁA���������Ƃ�����)
			for (j = state.ord - 1; j >= flag; j--) {
				//�����N�̃��[�v
				for (i = 0; i < 4; i++) {
					//�X�[�g�̃��[�v
					if (used_cards[i][j] == 1) {
						//���̃����N�̒�o�ςݖ����𒲂ׂ�	
						rank_count_used_cards++;
					}

					if (own_cards[i][j] == 1) {
						//���̃����N�̎����̎����Ă閇���𒲂ׂ�	
						rank_count_own_cards++;
					}
				}

				if (rank_count_used_cards == 4) {
					//�S�Ē�o�ς݂̃����N�Ȃ玟��
					rank_count_used_cards = 0;
					rank_count_own_cards = 0;
					continue;
				}
				else if (rank_count_used_cards + rank_count_own_cards >= ba_nagare_max_limit_single) {
					//�����݂̂��A���̃����N�̃J�[�h�������Ă���Ƃ�(�����珇�ԂɌ��Ă���)
					if (j == flag) {
						ba_nagare_max_count_single_nomal_rev++;
						return j;
					}
					else {
						rank_count_used_cards = 0;
						rank_count_own_cards = 0;
						continue;
					}
				}
				else {
					return 0;
				}
			}

			return 0;
		}
		else {
			//���肠��

			//��̃X�[�g�𒲂ׂ�
			for (i = 0; i < 4; i++) {
				if (state.suit[i] == 1) {
					//��̃X�[�g��ba_suit�Ɋi�[
					ba_suit = i;
				}
			}

			//�ォ��
			for (j = 1; j < state.ord; j++) {
				//�����N�̃��[�v
				if (used_cards[ba_suit][j] == 0 && own_cards[ba_suit][j] == 1) {
					//��o�ς݂łȂ��@���@�����������Ă�Ƃ�
					flag = j;
				}
				else if (used_cards[ba_suit][j] == 0 && own_cards[ba_suit][j] == 0) {
					//��o�ς݂łȂ��@���@�����������ĂȂ�
					return 0;
				}
			}

			//������
			for (j = state.ord - 1; j >= flag; j--) {
				//�����N�̃��[�v
				if (used_cards[ba_suit][j] == 0 && own_cards[ba_suit][j] == 1) {
					//��o�ς݂łȂ��@���@�����������Ă�Ƃ�
					if (j == flag) {
						ba_nagare_max_count_single_sibari_rev++;
						return j;
					}
					else {
						continue;
					}
				}
				else if (used_cards[ba_suit][j] == 0 && own_cards[ba_suit][j] == 0) {
					//��o�ς݂łȂ��@���@�����������ĂȂ�
					return 0;
				}
			}

			return 0;
		}
	}

	return 0;
}

//�ꂪ�񖇁i�Ԃ�l�̓����N�m1�`13�n�j
int ba_nagare_max_double(int own_cards[8][15], int used_cards[8][15])
{
	int i, j;
	int rank_count_used_cards = 0, rank_count_own_cards = 0, ba_suit[2];
	int flag = 0, suit_count = 0;

	if (state.rev == 0) {
		//�v���łȂ�
		if (state.lock == 0) {
			//����Ȃ�

			//�ォ��(���̎��ɁA�ł������z����)
			for (j = 13; j > state.ord; j--) {
				//�����N�̃��[�v
				for (i = 0; i < 4; i++) {
					//�X�[�g�̃��[�v
					if (used_cards[i][j] == 1) {
						//���̃����N�̒�o�ςݖ����𒲂ׂ�	
						rank_count_used_cards++;
					}

					if (own_cards[i][j] == 1) {
						//���̃����N�̎����̎����Ă閇���𒲂ׂ�
						rank_count_own_cards++;
					}
				}

				if (rank_count_used_cards >= 3) {
					//��o�ς݃J�[�h��3���ȏ�i�R���S�j�̎��i���̃����N�͒N���o���Ȃ��j
					rank_count_used_cards = 0;
					rank_count_own_cards = 0;
					continue;
				}
				else if (rank_count_used_cards + rank_count_own_cards >= ba_nagare_max_limit_double) {
					//�����݂̂��A���̃����N�̃J�[�h�������Ă���Ƃ�(�ォ�珇�ԂɌ��Ă���)
					flag = j;
					rank_count_used_cards = 0;
					rank_count_own_cards = 0;
					break;
				}
				else {
					//���肪�����Ă�
					return 0;
				}
			}

			//������(���̎��ɁA���������Ƃ�����)
			for (j = state.ord + 1; j <= flag; j++) {
				//�����N�̃��[�v
				for (i = 0; i < 4; i++) {
					//�X�[�g�̃��[�v
					if (used_cards[i][j] == 1) {
						//���̃����N�̒�o�ςݖ����𒲂ׂ�	
						rank_count_used_cards++;
					}

					if (own_cards[i][j] == 1) {
						//���̃����N�̎����̎����Ă閇���𒲂ׂ�	
						rank_count_own_cards++;
					}
				}

				if (rank_count_used_cards >= 3) {
					//��o�ς݃J�[�h��3���ȏ�i�R���S�j�̎�
					rank_count_used_cards = 0;
					rank_count_own_cards = 0;
					continue;
				}
				else if (rank_count_used_cards + rank_count_own_cards >= ba_nagare_max_limit_double) {
					//�����݂̂��A���̃����N�̃J�[�h�������Ă���Ƃ�(�����珇�ԂɌ��Ă���)
					if (j == flag) {
						ba_nagare_max_count_double_nomal++;
						return j;
					}
					else {
						rank_count_used_cards = 0;
						rank_count_own_cards = 0;
						continue;
					}
				}
				else {
					return 0;
				}
			}

			return 0;
		}
		else {
			//���肠��

			//��̃X�[�g�𒲂ׂ�
			for (i = 0; i < 4; i++) {
				if (state.suit[i] == 1) {
					//��̃X�[�g��ba_suit�Ɋi�[
					ba_suit[suit_count] = i;
					suit_count++;
				}
			}

			//�ォ��
			for (j = 13; j > state.ord; j--) {
				//�����N�̃��[�v
				if (used_cards[(ba_suit[0])][j] == 0 && used_cards[ba_suit[1]][j] == 0) {
					//�y�A�̃X�[�g�̗������o�ĂȂ��Ƃ�
					if (own_cards[ba_suit[0]][j] == 0 && own_cards[ba_suit[1]][j] == 0) {
						//���@�y�A�̃X�[�g�̗����������Ă�Ƃ�
						flag = j;
					}
					else if (own_cards[ba_suit[0]][j] == 1 || own_cards[ba_suit[1]][j] == 1) {
						//���@�y�A�̃X�[�g������ł������ĂȂ��Ƃ�
						return 0;
					}
				}
			}

			//������
			for (j = state.ord + 1; j <= flag; j++) {
				//�����N�̃��[�v
				if (used_cards[ba_suit[0]][j] == 0 && used_cards[ba_suit[1]][j] == 0) {
					//�y�A�̃X�[�g�̗������o�ĂȂ��Ƃ�
					if (own_cards[ba_suit[0]][j] == 0 && own_cards[ba_suit[1]][j] == 0) {
						//���@�y�A�̃X�[�g�̗����������Ă�Ƃ�
						if (j == flag) {
							ba_nagare_max_count_double_sibari++;
							return j;
						}
						else {
							continue;
						}
					}
					else if (own_cards[ba_suit[0]][j] == 1 || own_cards[ba_suit[1]][j] == 1) {
						//���@�y�A�̃X�[�g������ł������ĂȂ��Ƃ�
						return 0;
					}
				}
			}

			return 0;
		}
	}
	else {
		//�v����
		if (state.lock == 0) {
			//����Ȃ�

			//�ォ��(���̎��ɁA�ł������z����)
			for (j = 1; j < state.ord; j++) {
				//�����N�̃��[�v
				for (i = 0; i < 4; i++) {
					//�X�[�g�̃��[�v
					if (used_cards[i][j] == 1) {
						//���̃����N�̒�o�ςݖ����𒲂ׂ�	
						rank_count_used_cards++;
					}

					if (own_cards[i][j] == 1) {
						//���̃����N�̎����̎����Ă閇���𒲂ׂ�	
						rank_count_own_cards++;
					}
				}

				if (rank_count_used_cards >= 3) {
					//��o�ς݃J�[�h��3���ȏ�i�R���S�j�̎�
					rank_count_used_cards = 0;
					rank_count_own_cards = 0;
					continue;
				}
				else if (rank_count_used_cards + rank_count_own_cards >= ba_nagare_max_limit_double) {
					//�����݂̂��A���̃����N�̃J�[�h�������Ă���Ƃ�(�ォ�珇�ԂɌ��Ă���)
					flag = j;
					rank_count_used_cards = 0;
					rank_count_own_cards = 0;
					break;
				}
				else {
					return 0;
				}
			}

			//������(���̎��ɁA���������Ƃ�����)
			for (j = state.ord - 1; j >= flag; j--) {
				//�����N�̃��[�v
				for (i = 0; i < 4; i++) {
					//�X�[�g�̃��[�v
					if (used_cards[i][j] == 1) {
						//���̃����N�̒�o�ςݖ����𒲂ׂ�	
						rank_count_used_cards++;
					}

					if (own_cards[i][j] == 1) {
						//���̃����N�̎����̎����Ă閇���𒲂ׂ�	
						rank_count_own_cards++;
					}
				}

				if (rank_count_used_cards >= 3) {
					//��o�ς݃J�[�h��3���ȏ�i�R���S�j�̎�
					rank_count_used_cards = 0;
					rank_count_own_cards = 0;
					continue;
				}
				else if (rank_count_used_cards + rank_count_own_cards >= ba_nagare_max_limit_double) {
					//�����݂̂��A���̃����N�̃J�[�h�������Ă���Ƃ�(�����珇�ԂɌ��Ă���)
					if (j == flag) {
						ba_nagare_max_count_double_nomal_rev++;
						return j;
					}
					else {
						rank_count_used_cards = 0;
						rank_count_own_cards = 0;
						continue;
					}
				}
				else {
					return 0;
				}
			}

			return 0;
		}
		else {
			//���肠��

			//��̃X�[�g�𒲂ׂ�
			for (i = 0; i < 4; i++) {
				if (state.suit[i] == 1) {
					//��̃X�[�g��ba_suit�Ɋi�[
					ba_suit[suit_count] = i;
					suit_count++;
				}
			}

			//�ォ��
			for (j = 1; j < state.ord; j++) {
				//�����N�̃��[�v
				if (used_cards[ba_suit[0]][j] == 0 && used_cards[ba_suit[1]][j] == 0) {
					//�y�A�̃X�[�g�̗������o�ĂȂ��Ƃ�
					if (own_cards[ba_suit[0]][j] == 0 && own_cards[ba_suit[1]][j] == 0) {
						//���@�y�A�̃X�[�g�̗����������Ă�Ƃ�
						flag = j;
					}
					else if (own_cards[ba_suit[0]][j] == 1 || own_cards[ba_suit[1]][j] == 1) {
						//���@�y�A�̃X�[�g������ł������ĂȂ��Ƃ�
						return 0;
					}
				}
			}

			//������
			for (j = state.ord - 1; j >= flag; j--) {
				//�����N�̃��[�v
				if (used_cards[ba_suit[0]][j] == 0 && used_cards[ba_suit[1]][j] == 0) {
					//�y�A�̃X�[�g�̗������o�ĂȂ��Ƃ�
					if (own_cards[ba_suit[0]][j] == 0 && own_cards[ba_suit[1]][j] == 0) {
						//���@�y�A�̃X�[�g�̗����������Ă�Ƃ�
						if (j == flag) {
							ba_nagare_max_count_double_sibari_rev++;
							return j;
						}
						else {
							continue;
						}
					}
					else if (own_cards[ba_suit[0]][j] == 1 || own_cards[ba_suit[1]][j] == 1) {
						//���@�y�A�̃X�[�g������ł������ĂȂ��Ƃ�
						return 0;
					}
				}
			}

			return 0;
		}
	}

	return 0;
}

//kou�̋����]���l�v�Z
int kou_e_value(int own_cards[8][15], int seat_order[5], int used_cards[8][15])
{
	int i, j, count = 0;
	//qty=1
	int n = 0, c=0;
	//qty>=2
	int r[15] = { 0 }, r_count = 0, fn[15] = {15}, player=0 , e = 100;

	if (state.qty == 1) {
		//n�̒l�v�Z
		if (state.rev == 0)
		{
			for (j = state.ord + 1; j <= 13; j++)
			{
				for (i = 0; i < 4; i++)
				{
					if (own_cards[i][j] != 1 && used_cards[i][j] != 1) {
						n++;
						break;
					}
				}
			}
		}
		else if (state.rev == 1)
		{
			for (j = state.ord - 1; j >= 1; j--)
			{
				for (i = 0; i < 4; i++)
				{
					if (own_cards[i][j] != 1 && used_cards[i][j] != 1) {
						n++;
						break;
					}
				}
			}
		}

		//���̒l�v�Z�i�W���[�J�[���܂��o�ĂȂ��@���@�����������Ă��Ȃ��ꍇ�P�@�A�@�����łȂ��ꍇ�O�j
		if (used_cards[4][1] != 2 && state.joker == 0) {
			c = 1;
		}
		else {
			c = 0;
		}

		//�����]���lE����
		return (100 - (n * 30) - c);
	}
	else if (state.qty >= 2 && state.sequence == 0) {
		//n_i�̌v�Z����
		if (state.rev == 0) {
			for (j = state.ord + 1; j <= 13; j++)
			{
				for (i = 0; i < 4; i++)
				{
					if (own_cards[i][j] != 1 && used_cards[i][j] != 1) {
						count++;
					}
				}

				if (count >= state.qty) {
					r[r_count] = count;
					r_count++;
				}
				count = 0;
			}
		}
		else if (state.rev == 1) {
			for (j = state.ord - 1; j >= 1; j--)
			{
				for (i = 0; i < 4; i++)
				{
					if (own_cards[i][j] != 1 && used_cards[i][j] != 1) {
						count++;
					}
				}

				if (count >= state.qty) {
					r[r_count] = count;
					r_count++;
				}
				count = 0;
			}
		}

		//�c��v���C����
		for (i = 0; i < 5; i++) {
			if (state.player_qty > 0) {
				player++;
			}
		}

		//f(n_i)�v�Z
		for (i = 0; i < r_count; i++) {
			if ((r[i] - state.qty - player + 5) == 0) {
				fn[i] = 4;
			}
			else if ((r[i] - state.qty - player + 5) == 1) {
				fn[i] = 9;
			}
			else if ((r[i] - state.qty - player + 5) == 2) {
				fn[i] = 15;
			}
			else if ((r[i] - state.qty - player + 5) >= 3) {
				fn[i] = 24;
			}
		}

		//������
		for (i = 0; i < r_count; i++) {
			e = e - fn[i];
		}

		//�����]���lE����
		return e;
	}

	return 0;
}


/*struct state_type
{
    int ord;            // ���ݏ�ɏo�Ă���J�[�h�̋���(�J�[�h�̂R���P�A�J�[�h�̂Q���P�R)
    int sequence;       // ��ɏo�Ă���J�[�h���K�i�Ȃ�1�A�����g�Ȃ�0
    int qty;            // ��ɏo�Ă���J�[�h�̖���
    int rev;            // �v���Ȃ�1�A�����łȂ��Ȃ�0
    int b11;            // 11�o�b�N�Ȃ�1�A�����łȂ��Ȃ�0�i���g�p�j
    int lock;           // ���΂�̂Ƃ�1�A�����łȂ��Ƃ�0
    int onset;          // ��ɉ����o�Ă��Ȃ��Ƃ�1�A�����łȂ��Ƃ�0
    int suit[5];        // ��ɏo�Ă���J�[�h�̃}�[�N�Bsuit[i]��1�̂Ƃ��A
                        // �}�[�N��i�̃J�[�h���o�Ă���B
    int player_qty[5];  // �e�v���C���̎c�薇��
    int player_rank[5]; // �e�v���C���̃����N
    int seat[5];        // �e�Ȃɒ����Ă���v���C���̔ԍ�
 
    int joker;          // ������Joker�������Ă���Ƃ�1�A�����łȂ��Ƃ�0�B
}*/