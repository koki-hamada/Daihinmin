/* cardChange.c : �J�[�h�����ɏo���J�[�h�����肷�� */ 
/* Author       : Kengo Matsuta                    */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "cardChange.h"

//�J�[�h�̒��g���ǂ�ȑg�ݍ��킹�ɂȂ��Ă��邩���ׂ�
void checkCards(int select_cards[8][15], int my_cards[8][15], int select_qty){
  int kaidan_buf[8][15];
  int pair_buf[8][15];
  int my_cards_buf[8][15];
  int temp_cards[8][15];

  int i,j,k;
  int count, noJcount;

  for(i=0;i<8;i++){
    for(j=0;j<15;j++){
      kaidan_buf[i][j] = 0;
      pair_buf[i][j] = 0;
      select_cards[i][j] = 0;
      temp_cards[i][j] = 0;
      my_cards_buf[i][j] = my_cards[i][j];
    }
  }


  for(i=0;i<4;i++){            //�e�X�[�g���ɑ�����
    count=1;
    noJcount=0; 
    for(j=13;j>=0;j--){        //���Ԃɂ݂�
      if(my_cards_buf[i][j]==1){   //�J�[�h������Ƃ�
	count++;               //2�̃J�E���^��i�߂�
	noJcount++;
      }
      else{                    //�J�[�h���Ȃ��Ƃ�
	count=noJcount+1;      //�W���[�J�[����̊K�i�̖����ɃW���[�J�[���𑫂� 
	noJcount=0;            //�W���[�J�[�Ȃ��̊K�i�̖��������Z�b�g����
      }
      if((my_cards_buf[4][1]==2 ? count : noJcount) > 2){              //3���ȏ�̂Ƃ�
	kaidan_buf[i][j]=(my_cards_buf[4][1]==2 ? count : noJcount);  //���̖������e�[�u���Ɋi�[
      }
      else{
	kaidan_buf[i][j]=0;      //���̑���0�ɂ���
      }
    }
  }

  int kaidan, joker_flag;

  //�ア�ق��̎�D��D�悵�ĊK�i�ŏ���邽�߂�j->i�̏��Ń��[�v(�W���[�J�[���l�����Ƃ�)
  for(j=0;j<15;j++){
    for(i=0;i<5;i++){
      if(kaidan_buf[i][j]!=0){
	joker_flag=0;
	kaidan = kaidan_buf[i][j];
	for(k=j;k<j+kaidan;k++){
	  if(my_cards_buf[i][k]==0){
	    if(my_cards_buf[4][1]==2 && joker_flag==0){
	      //�W���[�J�[�g�p
	      joker_flag=1;
	    }else{
	      //�W���[�J�[�g�p�ς݂̂Ƃ��͊K�i�t���O���Ȃ��������Ƃɂ���
	      kaidan_buf[i][j] = 0;
	    }
	  }
	}
	if(kaidan_buf[i][j]!=0){
	  if(joker_flag){	//�W���[�J�[�g�p
	    for(k=j;k<j+kaidan;k++){
	      my_cards_buf[i][k]=0;
	      kaidan_buf[i][j]=1;
	    }
	    my_cards_buf[4][1]=0;
	  }else{	//�W���[�J�[�����ŊK�i������
	    for(k=j;k<j+kaidan;k++){
	      my_cards_buf[i][k]=0;
	      kaidan_buf[i][k]=1;
	    }
	  }
	}
      }
    }
  }

  //��������y�A
  for(i=1;i<=13;i++){  //���ꂻ��̋����̃J�[�h�̖����𐔂��C�W���[�J�[������΂��̕���������
    count=my_cards_buf[0][i]+my_cards_buf[1][i]+my_cards_buf[2][i]+my_cards_buf[3][i]+(my_cards_buf[4][1]==2);
    if(count>1){      //������2���ȏ�̂Ƃ�
      for(j=0;j<4;j++){
	if(my_cards_buf[j][i]==1){    //�J�[�h�������Ă��镔����
	  pair_buf[j][i]=count; //���̖������i�[
	}
      }
    }
  }

  int pair;
  //�ア�ق��̎�D��D�悵�ăy�A�ŏ���邽�߂�j->i�̏��Ń��[�v(�W���[�J�[���l�����Ƃ�)
  for(j=0;j<15;j++){
    for(i=0;i<5;i++){
      if(pair_buf[i][j]!=0){
	pair = pair_buf[i][j];
	if(my_cards_buf[0][j]+my_cards_buf[1][j]+my_cards_buf[2][j]+my_cards_buf[3][j]==(pair-1) && my_cards_buf[4][1]==2 && pair>2){
	  for(k=0;k<4;k++){
	    if(my_cards_buf[k][j]!=0){
	      my_cards_buf[k][j]=0;
	      pair_buf[k][j]=1;
	    }
	  }
	  my_cards_buf[4][1]=0;
	}else if(my_cards_buf[0][j]+my_cards_buf[1][j]+my_cards_buf[2][j]+my_cards_buf[3][j]==pair){
	  for(k=0;k<4;k++){
	    if(my_cards_buf[k][j]!=0){
	      my_cards_buf[k][j]=0;
	      pair_buf[k][j]=1;
	    }
	  }
	}else{
	  pair_buf[i][j]=0;
	}
      }
    }
  }

  for(i=0;i<8;i++){
    for(j=0;j<15;j++){
      temp_cards[i][j] = my_cards_buf[i][j];
    }
  }

  for(i=0;i<4;i++){
    if(my_cards_buf[i][6]==1){
      //			printf("solo 8 card\n");
      my_cards_buf[i][6]=0;
    }
  }

  //�����ɓ���\���͂ƂĂ��Ⴂ��
  if(my_cards_buf[4][1]==2){
    //		printf("solo joker\n");
    my_cards_buf[4][1]=0;
  }

  //�v���΍�
  for(i=0;i<4;i++){
    if(my_cards_buf[i][1]==1){
      //			printf("solo 3 card\n");
      my_cards_buf[i][1]=0;
    }
  }

  for(i=0;i<4;i++){
    for(j=9;j<15;j++){
      if(my_cards_buf[i][j]==1){
	//				printf("solo %d card\n", j+2);
	my_cards_buf[i][j]=0;
      }
    }
  }

  int remains = 0;
  count = 0;

  //�c��̖����`�F�b�N
  for(i=0;i<5;i++)
    for(j=0;j<15;j++)
      remains += (my_cards_buf[i][j] > 0);

  if(remains==select_qty){			//�����Ȃ炻�̂܂܃R�s�[
    for(i=0;i<8;i++){
      for(j=0;j<15;j++){
	select_cards[i][j] = my_cards_buf[i][j];
      }
    }
  }else if(remains > select_qty){		//�c��̖����̂ق�������������葽����Ώ������J�[�h����e�[�u���ɒǉ�
    for(j=0;j<15;j++){
      if(count == select_qty) break;
      for(i=0;i<4;i++){
	if(count==select_qty)break;
	if(my_cards_buf[i][j]!=0){
	  select_cards[i][j]=1;
	  count++;
	}
      }
    }
  }else if(remains < select_qty){		//���������ɒB���Ă��Ȃ���΁A�����������Ă���
    for(i=0;i<8;i++){
      for(j=0;j<15;j++){
	select_cards[i][j] = my_cards_buf[i][j];
	if(my_cards_buf[i][j]==1) temp_cards[i][j] = 0;
      }
    }
    count = remains;
    for(i=0;i<4;i++){				//�ŏ��͒P�i��3���o��
      if(temp_cards[i][1]!=0){
	select_cards[i][1]=1;
	temp_cards[i][1]=0;
	count++;
      }
      if(count == select_qty) break;
    }
    if(count != select_qty){
      for(i=0;i<4;i++){		//���͒P�i��8���o��
	if(temp_cards[i][6]!=0){
	  select_cards[i][6]=1;
	  temp_cards[i][6]=0;
	  count++;
	}
	if(count == select_qty) break;
      }
    }
    if(count != select_qty){	//���͒P�i��11�`12���o��
      for(j=9;j<11;j++){
	if(temp_cards[i][j]!=0){
	  select_cards[i][j]=1;
	  temp_cards[i][j]=0;
	  count++;
	}
	if(count == select_qty) break;
      }
    }
    if(count != select_qty){	//���̓y�A�����
      for(i=0;i<8;i++){
	for(j=0;j<15;j++){
	  if(pair_buf[i][j]>0) temp_cards[i][j] = 1;
	}
      }
      for(j=0;j<15;j++){
	for(i=0;i<4;i++){
	  if(temp_cards[i][j] != 0){
	    for(k=0;k<4;k++){
	      if(temp_cards[k][j]!=0){
		select_cards[k][j]=1;
		temp_cards[k][j]=0;
		count++;
	      }
	      if(count == select_qty) break;
	    }
	  }
	  if(count == select_qty) break;
	}
	if(count == select_qty) break;
      }
    }
    if(count != select_qty){	//�Ō�ɊK�i�����
      for(i=0;i<8;i++){
	for(j=0;j<15;j++){
	  if(pair_buf[i][j]>0) temp_cards[i][j] = 1;
	}
      }
      for(j=0;j<15;j++){
	for(i=0;i<4;i++){
	  if(temp_cards[i][j]!=0){
	    select_cards[i][j]=1;
	    temp_cards[i][j]=0;
	    count++;
	  }
	  if(count == select_qty) break;
	}
	if(count == select_qty) break;
      }
    }
  }
}


void checkCards2(int select_cards[8][15], int my_cards[8][15], int select_qty){
  int s, r;
  for(r=1; r<14; r++){
    for(s=0;s<4;s++){			
      if(my_cards[s][r]!=0){
	select_cards[s][r]=1;
        goto out;
      }
    }
  }
 out:
  return;
}
