#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "connection.h"
#include "cardChange.h"
#include "bitCard.h"
#include "cardSelect.h"
#include "mt19937ar.h"
#include "checkInfo.h"
#include "mydef.h"

const int g_logging=0;                     //���O�������邩�ۂ��𔻒肷�邽�߂̕ϐ�

int main(int argc,char* argv[]){

  int i, j;

  int my_playernum;            //�v���C���[�ԍ����L������
  int whole_gameend_flag=0;	 //�S�Q�[�����I���������ۂ��𔻕ʂ���ϐ�
  int one_gameend_flag=0;	     //1�Q�[�����I��������ۂ��𔻕ʂ���ϐ�
  int accept_flag=0;           //��o�����J�[�h���󗝂��ꂽ���𔻕ʂ���ϐ�
  int game_count=0;		     //�Q�[���̉񐔂��L������
  int game_start_flag = 0;
  int is_my_turn = 0;
  int change_qty = 0;

  int own_cards[8][15];    //��D�̃J�[�h�e�[�u���������߂�ϐ�
  int ba_cards[8][15];     //��ɏo���J�[�h�e�[�u����[�߂�

  int64 myCards, oppCards;
  int64 befCards, aftCards; // , getCards;
  changeInfo cinfo;
  fieldInfo  finfo;
  const int64 allCards = (1LL << 53) - 1;

  // �����̏�����
  //init_genrand(25837341);
  init_genrand(141592653);
  
  //�����̃`�F�b�N �����ɏ]���ăT�[�o�A�h���X�A�ڑ��|�[�g�A�N���C�A���g����ύX
  checkArg(argc,argv);

  //�Q�[���ɎQ��
  my_playernum=entryToGame();
  
  while(whole_gameend_flag==0){
    one_gameend_flag=0;                 //1�Q�[�����I��������������t���O��������
    game_start_flag = 1;

    game_count=startGame(own_cards);//���E���h���n�߂� �ŏ��̃J�[�h���󂯎��B

    ///�J�[�h����
    if(own_cards[5][0]== 0){ //�J�[�h�������t���O���`�F�b�N ==1�Ő���
      printf("not card-change turn?\n");
      exit (1);
    }
    else{ //�e�[�u���ɖ�肪������Ύ��ۂɌ�����
      // �����O�̎�D���r�b�g�ŕۑ����Ă���
      befCards = setBit(own_cards);

      if(own_cards[5][1] > 0 && own_cards[5][1]<100){
	change_qty = own_cards[5][1];          //�J�[�h�̌�������
	int select_cards[8][15] = {{0}};       //�I�񂾃J�[�h���i�[
	
        if(change_qty == 2){
	  //��������x���ł���Εs�v�ȃJ�[�h��I�яo��	
	  checkCards(select_cards, own_cards, change_qty);
        }else{
          //�������x���̂Ƃ��͍Ŏ���o��
	  checkCards2(select_cards, own_cards, change_qty);
	  //printf("Im fugo\n");
        }
    
	//�I�񂾃J�[�h�𑗐M
	sendChangingCards(select_cards);
      }
      else{
	change_qty = 0;
	//�����������ȉ��Ȃ�A��������K�v�͂Ȃ�
      }
    } //�J�[�h���������܂�

    while(one_gameend_flag == 0){     //1�Q�[�����I���܂ł̌J��Ԃ�
      int select_cards[8][15]={{0}};      //��o�p�̃e�[�u��

      is_my_turn = receiveCards(own_cards);

      //getStates();
      checkState(&finfo, own_cards);

      // �Q�[���J�n���̏���
      if(game_start_flag == 1){
	// �����̎�D
	myCards = setBit(own_cards);
	// �����ȊO�����J�[�h
	oppCards = allCards^myCards;
	aftCards = myCards;
	// �����ɏo�����J�[�h
	cinfo.chgCards = befCards^(befCards&aftCards);

	cinfo.firstPlayer = -1;

	// ������t���O������
	finfo.goal = 0;
	// �ȏ��E�K���E�J�[�h�̏����������擾
	for(i=0;i<5;i++) finfo.seat[own_cards[6][i+10]] = i;
	for(i=0;i<5;i++){
	  finfo.lest[finfo.seat[i]] = own_cards[6][i  ];
	  finfo.rank[finfo.seat[i]] = own_cards[6][i+5];
	  if(own_cards[6][i+10] == my_playernum) finfo.mypos = i;
	}


	// �������x���ȏ�̏ꍇ�C�J�[�h�������肪�������Ȃ��J�[�h�̏W�������߂�
	cinfo.notCards = 0LL;
	int cnt = 0;
	if(finfo.rank[my_playernum] < 2){
	  int ni=13, nj=0;
	  for(i=13;i>=0;i--){
	    for(j=0;j<4;j++){
	      if(((befCards>>(13*j+i))&1)==1&&cnt<change_qty){
		ni = i, nj = j;
		cnt++;
	      }
	    }
	  }
	  for(i=0;i<13;i++){
	    for(j=3;j>=0;j--){
	      if(ni<i||(ni==i&&nj>j)){
		cinfo.notCards |= (1LL << (13*j+i));
	      }
	    }
	  }
	}

#ifdef USE_ESTIMATE_HAND
	initEnemysProb();
#endif

	game_start_flag = 0;
      }

      // �����̃^�[���̂Ƃ�
      if(is_my_turn== 1){  
	// �����̎�D���r�b�g�ɕϊ�
	myCards = setBit(own_cards);
	// �o���J�[�h�����肷��
	monteCarloSearch(select_cards, myCards, oppCards, &cinfo, &finfo);

	// �I�񂾃J�[�h���o
	accept_flag=sendCards(select_cards);
      }
      else{
	// �����̃^�[���ł͂Ȃ���
	// �Ӗ����Ȃ�������i�߂Ă݂�
	//genrand_int32();
      }

      //���̃^�[���ɒ�o���ꂽ���ʂ̃e�[�u���󂯎��,��ɏo���J�[�h�̏�����͂���
      lookField(ba_cards);

      // �ŏ��ɍs�������v���C�����o���Ă���
      if(cinfo.firstPlayer == -1){
	cinfo.firstPlayer = finfo.seat[ba_cards[5][3]];
      }

      // �^�[���̌��ʂ����̏����X�V
      checkField(&finfo, &cinfo, ba_cards, oppCards);

      // ���肪���������D�̏����X�V
      oppCards ^= (oppCards&setBit(ba_cards));
      
      //���̃Q�[�����I��������ۂ��̒ʒm���T�[�o���炤����B
      switch (beGameEnd()){
      case 0: //0�̂Ƃ��Q�[���𑱂���
	one_gameend_flag=0;
	whole_gameend_flag=0;
	break;
      case 1: //1�̂Ƃ� 1�Q�[���̏I��
	one_gameend_flag=1;
	whole_gameend_flag=0;
	if(g_logging == 1){
	  printf("game #%d was finished.\n",game_count);
	}
	break;
      default: //���̑��̏ꍇ �S�Q�[���̏I��
	one_gameend_flag=1;
	whole_gameend_flag=1;
	if(g_logging == 1){
	  printf("All game was finished(Total %d games.)\n",game_count);
	}
	break;
      }
    }//1�Q�[�����I���܂ł̌J��Ԃ������܂�
  }//�S�Q�[�����I���܂ł̌J��Ԃ������܂�
  //�\�P�b�g����ďI��
  if(closeSocket()!=0){
    printf("failed to close socket\n");
    exit(1);
  }
  exit(0);
}
