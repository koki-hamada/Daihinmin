/*
testes(2017)
���I�ɕ]���l�����߁C�_��ɕx�񂾃v���O������ڎw�����D
���g�̎��s�␬�����L�����C�����ɖ𗧂Ă�D
*/
/*
testestes(2018)
�@�B�w�K�ɂ���o��̕]����ǉ�����.
���݂̊�����̓f�[�^�Ƃ���
��o�����������悻�̃J�[�h�f�[�^�����o�̓f�[�^�Ƃ���.
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "daihinmin.h"
#include "connection.h"
#include "NN.h"
#include "init_weight.h"

const int g_logging=0;                     //���O�������邩�ۂ��𔻒肷�邽�߂̕ϐ�

int main(int argc,char* argv[]){
  int my_playernum;              //�v���C���[�ԍ����L������
  int whole_gameend_flag=0;	 //�S�Q�[�����I���������ۂ��𔻕ʂ���ϐ�
  int one_gameend_flag=0;	 //1�Q�[�����I��������ۂ��𔻕ʂ���ϐ�
  int accept_flag=0;             //��o�����J�[�h���󗝂��ꂽ���𔻕ʂ���ϐ�
  int game_count=0;		 //�Q�[���̉񐔂��L������
  int own_cards_buf[8][15];      //��D�̃J�[�h�e�[�u���������߂�ϐ�
  int own_cards[8][15];	         //����p�̎�D�̃e�[�u��
  int ba_cards_buf[8][15];       //��ɏo���J�[�h�e�[�u����[�߂�
  int ba_cards[8][15];           //����p�̏�̎D�̃e�[�u��
  int ba_cards_bef[8][15]={{0}};      //��r�p�e�[�u��

  int last_playernum=0;          //�Ō�ɒ�o�����v���C���[
  int used_cards[8][15]={{0}};         //��o��̃J�[�h
  int i,j,k;//�ėp�ϐ�
  int search[8][15]={{0}};//�����p�e�[�u��
  int max=0;//��ԋ����J�[�h���L�^
  int pass_num=0;//���g����o���Ă���̑��v���C���p�X��
  int t,y;//�O���㔼�C������i�[
  int f;//�ėp�t���O

  int pattern[13][7]={{0}};
  //[0]�`[10] �g�̏��F[0]����[1]����[2]�X�[�g[3]joker���g��������[4]�����]���l(�ȍ~x)[5]�v�����̋����]���l(revx)[6]�D��]���l(y)
  //     [11] ��D�̏��F[0]�g��[1]�v���\���̗L��[2]x�̍ŏ��l(1min)[3]x��2�Ԗڂɏ������l(2min)[4]�S�̕]���l(z)[5]�v������z(revz)[6]�v������2min(rev2m)
  //     [12] ��o��̏󋵁F[0]�I�������g�̋����]���l(x')[1]�I�������g��o���̑S�̕]���l(z')[2]�W���[�J�[�̎g�p(0:������,1:1������,2:�K�i�̈ꕔ(patmake),3:�g�p)
  //     lead�ł�[12]�ɂ͒�o����g�̏�񂪓���

  int tes[3][4][3]={{{0}}};//��d�����p�����[�^�Dt=���Ւ��ՏI�ՁCy=���(0:Joker,1:�P��,2:�y�A,3:�K�i)�C[t][y][0]=����̋����C[t][y][1]=���o���̑��v���C���p�X���C[t][y][2]=���̕]���l
  int tes_buf[3][4][3]={{{0}}};

  int seat_num=0;//�^�[���v���C���̐Ȕԍ�
  int score[5]={0};//���v�X�R�A ���Ԃ̓v���C���ԍ���
  int ranks[5]={0};//1�Q�[���̊e�v���C���̏���(��x��0...��n��4,������Ȃ������ꍇ4) ���Ԃ̓v���C���ԍ���
  int jk1card=0;//�W���[�J�[��1���ŏo���Ƃ�1
  int win_flag=0;//�N����������������1
  int ba_cut=0;//�ꂪ�����Ƃ�1

  int NNc;
  double IN[INPUTNO]={0};        //NN���̓f�[�^
  double OUT[OUTPUTNO]={0};      //NN�o�̓f�[�^
  double wh[HIDDENNO][INPUTNO+1]={{0}};//���ԑw�d��
  double wo[OUTPUTNO][HIDDENNO+1]={{0}};//�o�͑w�d��
  
  struct Pattern pat[128];

  /*�d�݂��Z�b�g*/
  for(i=0,k=0;i<HIDDENNO;i++){
    for(j=0;j<INPUTNO+1;j++){
      wh[i][j]=w[k];
      k++;
    }
  }
  for(i=0;i<OUTPUTNO;i++){
    for(j=0;j<HIDDENNO+1;j++){
      wo[i][j]=w[k];
      k++;
    }
  }

  //�����̃`�F�b�N �����ɏ]���ăT�[�o�A�h���X�A�ڑ��|�[�g�A�N���C�A���g����ύX
  checkArg(argc,argv);
  //�Q�[���ɎQ��
  my_playernum=entryToGame();
  while(whole_gameend_flag==0){
    one_gameend_flag=0;                 //1�Q�[�����I��������������t���O��������
    game_count=startGame(own_cards_buf);//���E���h���n�߂� �ŏ��̃J�[�h���󂯎��B
    copyTable(own_cards,own_cards_buf); //��������e�[�u���𑀍삷�邽�߂̃e�[�u���ɃR�s�[

    //100�Q�[�����Ƀf�[�^���쓙�̏���
    if(game_count%100==1){//�K�������Z�b�g����Ƃ��ɁC�O��̊K���𕽖��ɂ���
      for(i=0;i<5;i++){
        ranks[i]=2;
      }
      if(game_count%500==1){//500�Q�[���ڂŕ����z���Ă����ꍇ�C���I�]���l����U���Z�b�g
        for(i=0;i<5;i++){
          if(i!=my_playernum){
            if(score[my_playernum]<score[i]){
              for(j=0;j<3;j++){
                for(k=0;k<4;k++){
                  tes[j][k][2]=0;
                }
              }
              break;
            }
          }
        }
      }
      if(game_count!=1){
        if((PRINT_SCORE==1 && my_playernum==0) || PRINT_SCORE==2){
          fprintf( stderr, "%5d game score ",game_count-1);//�X�R�A�\��
          for(i=0;i<5;i++){
            fprintf( stderr, "%5d",score[i]);
            if(i==4){
              fprintf( stderr, "\n");
            }
            else{
              fprintf( stderr, " / ");
            }
          }
        }
      }
    }

    //�Q�[���J�n���Ƀf�[�^�i�[�Ə�����
    for(i=0;i<5;i++){//�ȏ��𒲂ׂ�
      if(own_cards_buf[6][10+i]==my_playernum){
        for(j=0;j<5;j++){
          if((i+j)<5){
            ss.seat[j]=own_cards_buf[6][10+i+j];
          }
          else{
            ss.seat[j]=own_cards_buf[6][5+i+j];
          }
        }
      }
    }

    for(i=0;i<5;i++){
      ss.score[i]=score[ss.seat[i]];//���v�X�R�A�Z�b�g
      ss.old_rank[i]=ranks[ss.seat[i]];//�O�Q�[���̊K���Z�b�g
      ss.hand_qty[i]=own_cards_buf[6][ss.seat[i]];//�����̎�D�����Z�b�g
      ss.pass[i]=0;//�p�X/������󋵃��Z�b�g
    }
    for(i=0;i<5;i++){
      ranks[i]=4;//����4�Ƀ��Z�b�g
    }
    clearTable(used_cards);
    clearTable(search);
    jk1card=0;
    win_flag=0;
    ba_cut=1;

    ///�J�[�h����
    if(own_cards[5][0]== 0){ //�J�[�h�������t���O���`�F�b�N ==1�Ő���
      printf("not card-change turn?\n");
      exit (1);
    }
    else{ //�e�[�u���ɖ�肪������Ύ��ۂɌ�����
      if(own_cards[5][1] > 0 && own_cards[5][1]<100){
	int change_qty=own_cards[5][1];          //�J�[�h�̌�������
      	int select_cards[8][15]={{0}};           //�I�񂾃J�[�h���i�[
	
	//�������x���A��x���ł���Εs�v�ȃJ�[�h��I�яo��
	/////////////////////////////////////////////////////////////
	//�J�[�h�����̃A���S���Y���͂����ɏ���
	/////////////////////////////////////////////////////////////
        kou_change(select_cards,own_cards,change_qty,tes);
	/////////////////////////////////////////////////////////////
	//�J�[�h�����̃A���S���Y�� �����܂�
	/////////////////////////////////////////////////////////////

	//�I�񂾃J�[�h�𑗐M
	sendChangingCards(select_cards);
      }
      else{
	//�����������ȉ��Ȃ�A��������K�v�͂Ȃ�
      }
    } //�J�[�h���������܂�
    while(one_gameend_flag == 0){     //1�Q�[�����I���܂ł̌J��Ԃ�

      //��̃J�[�h�\��
      if((PRINT_STATE==1 && my_playernum==0) || PRINT_STATE==2){
        fprintf( stderr, "bacard:");
        if(ba_cut==1 || state.onset==1){
          fprintf( stderr, " no card\n");
        }
        else{
          fprintf( stderr, "P%d:",last_playernum);
          if(jk1card==1){
            fprintf( stderr, " jk\n");
          }
          else{
            cardprint(ba_cards);
          }
        }
      }
      int select_cards[8][15]={{0}};      //��o�p�̃e�[�u��
      if(receiveCards(own_cards_buf)== 1){  //�J�[�h��own_cards_buf�Ɏ󂯎��
                                            //�����Ԃ̓ǂݏo��
	                                    //�����̃^�[���ł��邩���m�F����
	//�����̃^�[���ł���΂��̃u���b�N�����s�����B
	clearCards(select_cards);             //�I�񂾃J�[�h�̃N���A
        copyTable(own_cards,own_cards_buf);   //�J�[�h�e�[�u�����R�s�[
	/////////////////////////////////////////////////////////////
	//�A���S���Y����������
	//�ǂ̃J�[�h���o�����͂����ɂ���
	/////////////////////////////////////////////////////////////


	//��̃X�[�g��2�i���Ŋi�[
	for(i=0;i<5;i++) if(state.suit[i]==1)own_cards[5][8]|=conv2to10(i);

	set(pat, own_cards);
	Cselect(pat, state);

        for(i=0,k=0;i<4;i++){
          for(j=1;j<14;j++){
            IN[k]=(double)own_cards[i][j];
            k++;
          }
        }
        IN[k]=(double)state.joker;k++;
        for(i=0;i<5;i++,k++)IN[k]=((double)state.player_rank[i])/4;
        for(i=0;i<5;i++,k++)IN[k]=((double)state.seat[i])/4;
        for(i=0;i<5;i++,k++)IN[k]=((double)state.player_qty[i])/11;
        for(i=0;i<4;i++,k++){
          if(state.lock==1)IN[k]=((double)state.suit[i]);
	  else IN[k]=0.0;
        }
        IN[k]=(double)state.rev;k++;
	for(i=0;i<6;i++,k++){
	  if(state.qty==i)IN[k]=1.0;
	  else IN[k]=0.0;
	}
        if(state.qty>5)IN[78]=1.0;
        if(state.onset==1)IN[k]=1.0;
	else IN[k]=0.0;
        k++;
        if(state.qty==1)IN[k]=1.0;
	else IN[k]=0.0;
	k++;
	if(state.qty>1&&state.qty==0)IN[k]=1.0;
	else IN[k]=0.0;
	k++;
	if(state.sequence==1)IN[k]=1.0;
	else IN[k]=0.0;
	k++;
        for(i=0;i<13;i++,k++){
	  if(state.ord==i)IN[k]=1.0;
	  else IN[k]=0.0;
	}
	if(state.ord==14)IN[83]=1.0;
	for(i=0;i<4;i++,k++)IN[k]=(double)state.suit[i];

	NN(IN,OUT,wh,wo);


	if(pat[0].count>0){
	  NNc=NNselect(pat,OUT,state.rev,state.qty);
	  //fprintf(stderr,"NNchoice![%d]\n",i);
	}
	/*
	cardstoPat(state.rev,select_cards,pat[i]);
	if(pat[i].joker==1){
	  select_cards[4][0]=0;
	  if(pat[i].count==1)select_cards[0][0]=2;
	  }*/

        max=0;//�����ȊO�̃J�[�h�̋����̍ő�
	if(state.rev==0){
          for(i=13;i>0 && max==0;i--){
            if(own_cards[0][i]+own_cards[1][i]+own_cards[2][i]+own_cards[3][i]+used_cards[4][i]!=4){//���������̃J�[�h�����̎�D�ɂ����
              max=i;//���̋������L�^
            }
          }
        }
	else{
          for(i=1;i<14 && max==0;i++){
            if(own_cards[0][i]+own_cards[1][i]+own_cards[2][i]+own_cards[3][i]+used_cards[4][i]!=4){//���������̃J�[�h�����̎�D�ɂ����
              max=i;//���̋������L�^
            }
          }
        }
        if(max==0){
          if(state.rev==0){
            max=1;
          }
          else{
            max=13;
          }
        }

        for(i=0;i<13;i++){
          for(j=0;j<6;j++){
            pattern[i][j]=0;
          }
        }
        pat_make(pattern,own_cards,max,used_cards,state.joker,tes);

        //�󋵕\��
        if((PRINT_STATE==1 && my_playernum==0) || PRINT_STATE==2){
          for(i=0;i<5;i++){
            fprintf( stderr, "P%d:score%4d rank%d hand%2d ",ss.seat[i],ss.score[i],ss.old_rank[i],ss.hand_qty[i]);
            if(ss.pass[i]==1){
              fprintf( stderr, "Pass");
            }
            if(ss.pass[i]==3){
              fprintf( stderr, "Win");
            }
            fprintf( stderr, "\n");
          }
          fprintf( stderr, "my_cards:");
          cardprint(own_cards);
          fprintf( stderr, "used_cards_table\n");
          fprintf( stderr, "\n");
          Tableprint(used_cards,0);
        }
        //��o�O�̑g�ƕ]���l�\��
        if((PRINT_PAT==1 && my_playernum==0) || PRINT_PAT==2){
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

//��������ǂ̃J�[�h���o��������-----------------------------------------------------------------------

        if(state.onset==1){ //��ɃJ�[�h�������Ƃ�
          copyTable(own_cards,own_cards_buf);
          kou_lead(select_cards,own_cards,max,used_cards,tes);//�V�����J�[�h���o���Ƃ��̔���
	}
        else{//���łɏ�ɃJ�[�h������Ƃ�
          if(jk1card==1 && own_cards[0][1]>0){//�X�y�[�h3
            k=pattern[11][0];//�g�����L�^
            own_cards[0][1]=0;
            pat_make(pattern,own_cards,max,used_cards,state.joker,tes);
            if((last_playernum!=own_cards[5][3] && k>=pattern[11][0]) || (last_playernum==own_cards[5][3] && search[4][1]==1)){//�g���������Ȃ���Ώo��
              select_cards[0][1]=1;
            }
            copyTable(own_cards,own_cards_buf);//�����own_cards�����ɖ߂�
          }
          else if(state.qty==own_cards[6][my_playernum])//��D�����Ə�̖����������Ƃ��́Adefault����I��(�o����Ƃ��͏o��)
          {
            if(state.rev==0){
              follow(select_cards,own_cards);    //�ʏ펞�̒�o�p 
            }else{
              followRev(select_cards,own_cards); //�v�����̒�o�p
            }
          }
          else{
            i=0;
            if(win_flag==1 && state.sequence==0){//�N����������������,��̑g����𗬂���P�́E�y�A�̏ꍇ�͒�o���Ȃ�
              i=1;
              if(state.rev==0){
                for(j=state.ord+1;j<=max;j++){
                  if(4-(own_cards[0][j]+own_cards[1][j]+own_cards[2][j]+own_cards[3][j]+used_cards[4][j])>=state.qty){
                    i=0;
                    j=14;
                  }
                }
              }
              else if(state.rev==1){
                for(j=state.ord-1;j>=max;j--){
                  if(4-(own_cards[0][j]+own_cards[1][j]+own_cards[2][j]+own_cards[3][j]+used_cards[4][j])>=state.qty){
                    i=0;
                    j=0;
                  }
                }
              }
            }
            if(state.sequence==1){//�K�i�g�̑I��
              clearTable(select_cards);
              kou_followsequence(select_cards,own_cards,max,used_cards,tes);
            }
            else if(i==0 && state.qty!=5){//�����g�̑I��(5���̏ꍇ�͏o���Ȃ��̂œ���Ȃ�)
              clearTable(select_cards);
              kou_followgroup(select_cards,own_cards,max,used_cards,tes);//�o���J�[�h�̔���
            }
          }
          //�Ō�ɃJ�[�h���o�����v���C���[�������̏ꍇ
          if(beEmptyCards(select_cards)==0 && last_playernum==own_cards[5][3]){//�����ȊO�p�X
            j=pattern[11][3];//2min���L�^
            k=pattern[11][0];//�g�����L�^
            cardsDiff(own_cards,select_cards);//��o�\��̑g��own_cards���甲��
            pat_make(pattern,own_cards,max,used_cards,state.joker,tes);//��o��̃p�^�[�������
            //�W���[�J�[���肪�����܂��̉\������
            if(pattern[11][3]<100){//��o��2min<100�̏ꍇ(2min>=100�̏ꍇ�͒�o)
              if(j>70 && j>pattern[11][3]){//��o��<��o�O����o�O>70
                clearTable(select_cards);
              }
              //�W���[�J�[�P�̂͏o���Ȃ�
              if(select_cards[0][0]==2)
                select_cards[0][0]=0;
              //�����J�[�h�̖����g�͏o���Ȃ�
              if(state.sequence==0 && state.rev==0){
                for(i=max;i<=13;i++){
                  select_cards[0][i]=0;
                  select_cards[1][i]=0;
                  select_cards[2][i]=0;
                  select_cards[3][i]=0;
                }
              }
              if(state.sequence==0 && state.rev==1){
                for(i=max;i>=1;i--){
                  select_cards[0][i]=0;
                  select_cards[1][i]=0;
                  select_cards[2][i]=0;
                  select_cards[3][i]=0;
                }
              }
              //�g�̐�������Ȃ��ꍇ�͏o���Ȃ�
              if(k<=pattern[11][0]){
                clearTable(select_cards);
              }
            }
          }
        }//���łɏ�ɃJ�[�h������Ƃ��̔��肱���܂�
if(beEmptyCards(select_cards)==0)f=0;
	/////////////////////////////////////////////////////////////
	//�A���S���Y���͂����܂�
	/////////////////////////////////////////////////////////////
	accept_flag=sendCards(select_cards);//cards���o
      }
      else{
	//�����̃^�[���ł͂Ȃ���
	//�K�v�Ȃ炱���ɏ������L�q����
      }
      //���̃^�[���ɒ�o���ꂽ���ʂ̃e�[�u���󂯎��,��ɏo���J�[�h�̏�����͂���
      lookField(ba_cards_buf);
      copyTable(ba_cards,ba_cards_buf);

      ///////////////////////////////////////////////////////////////
      //�J�[�h���o���ꂽ���� �N�����J�[�h���o���O�̏����͂����ɏ���
      ///////////////////////////////////////////////////////////////
      last_playernum=getLastPlayerNum(ba_cards);

                             /*-----��������V�K�ǉ�-----*/
      /*------------------------------------------------------*/
      /*    ���I�]���t���ɕK�v�ȏ���tes[3][4][3]�Ɋi�[���Ă���  �@ */
      /*------------------------------------------------------*/
       if(f==0 || f==1){//�t���O�ŊǗ�����
         if(f==0){//���g�̎�D��o�^�[��
         //�Q�[���̑O�㔼���L�^
         j=used_cards[5][1];
         if(j>=0 && j<13)t=0;//����
         else if(j>=13 && j<32)t=1;//����
         else if(j>=32)t=2;//�I��

         copyTable(ba_cards_bef,ba_cards);//��^�[���ڂ��獷�ق��ł�̂͂܂����̂ł�����  
          if(state.rev==0){
            if(state.joker==3 && state.qty==1){//Joker�P�i�̏ꍇ
              tes[t][0][0]=14;
              y=0;
              break;
            }
            for(i=13;i>0;i--){
              if(ba_cards[0][i]+ba_cards[1][i]+ba_cards[2][i]+ba_cards[3][i]>0){//�����L��
              if(state.qty==1){//�P�̂̏ꍇ
                tes[t][1][0]=i;
                y=1;
                break;
              }
              else if(state.qty>1){//�������̏ꍇ
                if(state.sequence==1){//�K�i�̏ꍇ
                  tes[t][3][0]=i;
                  y=3;
                  break;
                }
                else{//�y�A�̏ꍇ
                  tes[t][2][0]=i;
                  y=2;
                  break;
                }
              }
            }
          }
        }
        else if(state.rev==1){//�v����
            for(i=1;i<=13;i++){
            if(state.joker==3 && state.qty==1){//Joker�P�̂̏ꍇ
              tes[t][0][0]=14;
              y=0;
              break;
            }
              if(ba_cards[0][i]+ba_cards[1][i]+ba_cards[2][i]+ba_cards[3][i]>0){//�����L��
              if(state.qty==1){//�P�̂̏ꍇ
                tes[t][1][0]=i;
                y=1;
                break;
              }
              else if(state.qty>1){//�������̏ꍇ
                if(state.sequence==1){//�K�i�̏ꍇ
                  tes[t][3][0]=i;
                  y=3;
                  break;
                }
                else{//�y�A�̏ꍇ
                  tes[t][2][0]=i;
                  y=2;
                  break;
                }
              }
            }
          }
        }
        f=1; //�t���O�𗧂ĂĂ��̃��[�v�ɓ���Ȃ��悤�ɂ���
     }//f==0�C���o�^�[����p��������I��
      pass_num++;
      f=2;
      for(i=0;i<5;i++){
       if(i == last_playernum)continue;//����������
            if(ss.pass[i]==0)f=1;//���v���C�����p�X�����Ă��Ȃ��ꍇ�C�r���Œ�o�������������Ƃ��킩��
          }
       if(cmpCards(ba_cards_bef,ba_cards)==1 || f==2){//����������or�S���p�X�ŏꂪ���ꂽ
          pass_num-=2;
         //fprintf(stderr,"pass_num=%d\n",pass_num);
          tes[t][y][1]=pass_num;//���g����o�������ő��v���C�����p�X�������񐔂��i�[
      /*------------------------------------------------------*/
      /*                      �]���l�v�Z                        */
      /*------------------------------------------------------*/
          if(state.rev==0){
            if(t==0)tes[t][y][2]+=0;//�Q�[���i�s�󋵉��_
            else if(t==1)tes[t][y][2]-=2;
            else if(t==2)tes[t][y][2]-=2;
            if(tes[t][y][0]==1)tes[t][y][2]-=3;//���l���_
            else if(tes[t][y][0]==2)tes[t][y][2]+=1;
            else if(tes[t][y][0]>=3 && tes[t][y][0]<9)tes[t][y][2]+=8-tes[t][y][0];
            else if(tes[t][y][0]>=9 && tes[t][y][0]<13)tes[t][y][2]+=9-tes[t][y][0];
            else tes[t][y][2]+=10-tes[t][y][0];
            if(tes[t][y][1]==0)tes[t][y][2]-=3;//�p�X�����_
            else if(tes[t][y][1]==1)tes[t][y][2]+=1;
            else if(tes[t][y][1]==2)tes[t][y][2]+=2;
            else tes[t][y][2]+=3;
            if(y==0)tes[t][y][2]-=0;//�����_
            else if(y==1)tes[t][y][2]-=1;
            else if(y==2)tes[t][y][2]-=2;
            else if(y==3)tes[t][y][2]-=3;
            if(state.lock==1)tes[t][y][2]-= 2;//���΂���_
          }
          else if(state.rev==1){
            if(t==0)tes[t][y][2]+=0;//�Q�[���i�s�󋵉��_
            else if(t==1)tes[t][y][2]-=2;
            else if(t==2)tes[t][y][2]-=2;
            if(tes[t][y][0]==13)tes[t][y][2]-=3;//���l���_
            if(tes[t][y][0]==12)tes[t][y][2]+=1;
            else if(tes[t][y][0]<=11 && tes[t][y][0]>5)tes[t][y][2]+=tes[t][y][0]-6;
            else if(tes[t][y][0]<=7 && tes[t][y][0]>1)tes[t][y][2]+=tes[t][y][0]-5;
            else tes[t][y][2]+=tes[t][y][0]-4;
            if(tes[t][y][1]==0)tes[t][y][2]-=3;//�p�X�����_
            else if(tes[t][y][1]==1)tes[t][y][2]+=1;
            else if(tes[t][y][1]==2)tes[t][y][2]+=2;
            else tes[t][y][2]+=3;
            if(y==0)tes[t][y][2]-=0;//�����_
            else if(y==1)tes[t][y][2]-=1;
            else if(y==2)tes[t][y][2]-=2;
            else if(y==3)tes[t][y][2]-=3;
            if(state.lock==1)tes[t][y][2]-=2;//���΂���_
          }

          //�p�����[�^��臒l�ݒ�
          for(i=0;i<3;i++){//�]���l���}�e臒l�𒴂��Ȃ��悤�ɂ���
            for(j=0;j<4;j++){
              if(i==0){
                if(tes[i][j][2]>9)tes[i][j][2]=9;
                else if(tes[i][j][2]<-9)tes[i][j][2]=-9;
              }
              else if(i==1){
                if(j==1){
                  if(tes[i][j][2]>45)tes[i][j][2]=45;
                  else if(tes[i][j][2]<-45)tes[i][j][2]=-45;
                }
                else{
                  if(tes[i][j][2]>15)tes[i][j][2]=15;
                  else if(tes[i][j][2]<-15)tes[i][j][2]=-15;
                }
              }
              else{
                if(j==1){
                  if(tes[i][j][2]>65)tes[i][j][2]=65;
                  else if(tes[i][j][2]<-65)tes[i][j][2]=-65;
                }
                else if(j==2){
                  if(tes[i][j][2]>45)tes[i][j][2]=45;
                  else if(tes[i][j][2]<-45)tes[i][j][2]=-45;
                }
                else if(j==3){
                  if(tes[i][j][2]>35)tes[i][j][2]=35;
                  else if(tes[i][j][2]<-35)tes[i][j][2]=-35;
                }
              }
            }
          }
          f=3;
          pass_num=0;
          i=5;
        }
      }
                            /*-----�����܂ŐV�K�ǉ�-----*/
      //��o��̃J�[�h�e�[�u���ɏ�̃J�[�h���L�^
      ba_cut=0;
      for(i=0;i<=4;i++){
        for(j=0;j<=14;j++){
          if(ba_cards[i][j]==1){
            used_cards[i][j]=1;
            if(j==6){//8�؂�
              ba_cut=1;
            }
            else if(i==0 && j==1 && jk1card==1){//�W���[�J�[�ւ̃X�y�[�h3
              ba_cut=1;
            }
          }
          if(ba_cards[i][j]==2){
            used_cards[4][14]=1;
            if(j==6){//8�؂�
              ba_cut=1;
            }
          }
        }
      }


      /*�v���C�����Ƃɂǂ̃J�[�h���o�������i�[�������ꍇ
      for(i=0;i<4;i++){
        for(j=1;j<=13;j++){
          if(p_used_cards[i][j]==0 && used_cards[i][j]==1){
            p_used_cards[i][j]=ba_cards[5][3]+1;
          }
        }
        if(p_used_cards[4][14]==0 && used_cards[4][14]==1){
          p_used_cards[4][14]=ba_cards[5][3]+1;
        }
      }*/

      used_cards[5][1]=0;//��ɏo���J�[�h�̍��v�������i�[
      for(j=1;j<=13;j++){
        used_cards[4][j]=used_cards[0][j]+used_cards[1][j]+used_cards[2][j]+used_cards[3][j];
        used_cards[5][1]+=used_cards[4][j];
      }
      used_cards[5][1]+=used_cards[4][14];

      //�ꂪ�W���[�J�[1���̏ꍇ�̂݃t���O��1�ɕۂ�
      if((state.ord==0 || state.ord==14) && state.qty==1){
        jk1card=1;
      }
      for(i=0;i<5;i++){//�^�[���v���C���̐Ȕԍ����i�[
        if(ss.seat[i]==own_cards_buf[5][3]){
          seat_num=i;
        }
      }

      //used_cards[5][0]=���O�̃^�[���܂łɏ�ɏo���J�[�h�̍��v����
      if(used_cards[5][0]==used_cards[5][1]){//�^�[���̑O��ŃJ�[�h���o�������������Ƃ�(�p�X)
        ss.pass[seat_num]=1;
        if((PRINT_STATE==1 && my_playernum==0) || PRINT_STATE==2){
          fprintf( stderr, " turn :P%d:Pass\n",ba_cards[5][3]);
        }
      }
      else{//�����łȂ��ꍇ��o�����v���C���̎�D���������炷
        ss.hand_qty[seat_num]-=(used_cards[5][1]-used_cards[5][0]);
        if((PRINT_STATE==1 && my_playernum==0) || PRINT_STATE==2){
          fprintf( stderr, " turn :P%d:use:",ba_cards[5][3]);
          if(jk1card==1){
            fprintf( stderr, " jk\n");
          }
          else{
            cardprint(ba_cards);
          }
        }
      }
      //������0�ɂȂ����ꍇ(������)
      if(ss.hand_qty[seat_num]==0 && ss.pass[seat_num]!=3){
        ranks[ss.seat[seat_num]]=0;
        for(i=0;i<5;i++){
          if(ss.pass[i]==3){//�������Ă��鐔�J�E���g
            ranks[ss.seat[seat_num]]++;//���ʋL�^
          }
        }
        ss.pass[seat_num]=3;//�������ԂƂ���
        win_flag=1;
      }
      else{
        win_flag=0;
      }
      used_cards[5][0]=used_cards[5][1];//��ɏo���J�[�h�̍��v�����X�V

      for(i=0;i<5;i++){
        if(ss.pass[i]==0){
          break;
        }
      }
      if(i==5){//�S���p�X
        ba_cut=1;
      }

      //�S���p�X�܂���8�؂�A�W���[�J�[�ւ̃X�y�[�h3�̏ꍇ�p�X��Ԃƃt���O���Z�b�g
      if(ba_cut==1){
        for(i=0;i<5;i++){
          if(ss.pass[i]<=2){
            ss.pass[i]=0;
          }
        }
        jk1card=0;
        win_flag=0;
      }

      //�v���C���̏�ԕ\��
      if((PRINT_STATE==1 && my_playernum==0) || PRINT_STATE==2){
        fprintf( stderr, "state :");
        if(ba_cards[5][6]==1){
          fprintf( stderr, "rev ");
        }
        if(ba_cut==1){
          printf("ba_cut");
        }
        else if(ba_cards[5][7]==1){
          fprintf( stderr, "bind ");
        }
        printf("\n\n");

        for(i=0;i<5;i++){
          fprintf( stderr, "P%d:score%4d rank%d hand%2d ",ss.seat[i],ss.score[i],ss.old_rank[i],ss.hand_qty[i]);
          if(ss.pass[i]==1){
            printf("Pass");
          }
          if(ss.pass[i]==3){
            printf("Win");
          }
          printf("\n");
        }
        printf("\n");
      }

      ///////////////////////////////////////////////////////////////
      //�����܂�
      ///////////////////////////////////////////////////////////////

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
    for(i=0;i<5;i++){
      score[i]+=(5-ranks[i]);
    }
      /*------------------------------------------------------*/
      /*             �����N�ɂ��]���l�̏�������                  */
      /*------------------------------------------------------*/
    if(ranks[my_playernum]<ss.old_rank[my_playernum]){//�O�����N�������g�̃����N�������Ȃ�����]���l�̕ω����L��
        for(i=0;i<3;i++){
          for(j=0;j<4;j++){
            for(k=0;k<3;k++){
            tes_buf[i][j][k]=tes[i][j][k];
          }
        }
      }
    }
    else{//�����N�ێ����ȑO���Ⴍ�Ȃ�����]���l�̕ω���buf�ŏ㏑��
        for(i=0;i<3;i++){
          for(j=0;j<4;j++){
            for(k=0;k<3;k++){
            tes[i][j][k]=tes_buf[i][j][k];
          }
        }
      }
    }

    //��ɏo���J�[�h�������Z�b�g

    /*�e���̎�D�̋L�^
    //�Ō�Ɏc�����v���C���������Ă����J�[�h���L�^
    for(k=0;k<5;k++){
      if(ranks[k]==4){
        for(i=0;i<4;i++){
          for(j=1;j<=13;j++){
            if(p_used_cards[i][j]==0){
              p_used_cards[i][j]=k+1;
            }
          }
          if(p_used_cards[4][14]==0){
            p_used_cards[4][14]=k+1;
          }
        }
      }
    }
    state.rev=0;
    for(k=0;k<5;k++){
      clearTable(own_cards);
      state.joker=0;
      for(i=0;i<4;i++){
        for(j=1;j<=13;j++){
          if(p_used_cards[i][j]==k+1){
            own_cards[i][j]=1;
          }
        }
      }
      if(p_used_cards[4][14]==k+1){
        own_cards[4][14]=1;
        state.joker=1;
      }
    }
    //�v���C�����Ƃ̃J�[�h���Z�b�g
    //clearTable(p_used_cards);
    */
  }//�S�Q�[�����I���܂ł̌J��Ԃ������܂�

  if((PRINT_SCORE==1 && my_playernum==0) || PRINT_SCORE==2){
    fprintf( stderr, "     Final score ");//�X�R�A�\��
    for(i=0;i<5;i++){
      fprintf( stderr, "%5d",score[i]);
      if(i==4){
        fprintf( stderr, "\n");
      }
      else{
        fprintf( stderr, " / ");
      }
    }
  }

  //�\�P�b�g����ďI��
  if(closeSocket()!=0){
    printf("failed to close socket\n");
    exit(1);
  }
  exit(0);
}
