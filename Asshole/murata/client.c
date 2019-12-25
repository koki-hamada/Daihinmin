#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "daihinmin.h"
#include "connection.h"

const int g_logging = 0;                     //���O�������邩�ۂ��𔻒肷�邽�߂̕ϐ�

int main(int argc, char* argv[]) {
	int my_playernum;              //�v���C���[�ԍ����L������
	int whole_gameend_flag = 0;		 //�S�Q�[�����I���������ۂ��𔻕ʂ���ϐ�
	int one_gameend_flag = 0;		 //1�Q�[�����I��������ۂ��𔻕ʂ���ϐ�
	int accept_flag = 0;             //��o�����J�[�h���󗝂��ꂽ���𔻕ʂ���ϐ�
	int game_count = 0;				 //�Q�[���̉񐔂��L������
	int own_cards_buf[8][15];      //��D�̃J�[�h�e�[�u���������߂�ϐ�
	int own_cards[8][15];	         //����p�̎�D�̃e�[�u��
	int ba_cards_buf[8][15];       //��ɏo���J�[�h�e�[�u����[�߂�
	int ba_cards[8][15];           //����p�̏�̎D�̃e�[�u��

	int last_playernum = 0;          //�Ō�ɒ�o�����v���C���[
	int used_cards[8][15] = { {0} };   //��o��̃J�[�h
	int i, j, k;
	int search[8][15] = { {0} };//�����p�e�[�u��
	int max = 0;//��ԋ����J�[�h���L�^

	int pattern[13][7] = { {0} };
	//[0]�`[10] �g�̏��F[0]����[1]����[2]�X�[�g[3]joker���g��������[4]�����]���l(�ȍ~x)[5]�v�����̋����]���l(revx)[6]�D��]���l(y)
	//     [11] ��D�̏��F[0]�g��[1]�v���\���̗L��[2]x�̍ŏ��l(1min)[3]x��2�Ԗڂɏ������l(2min)[4]�S�̕]���l(z)[5]�v������z(revz)[6]�v������2min(rev2m)
	//     [12] ��o��̏󋵁F[0]�I�������g�̋����]���l(x')[1]�I�������g��o���̑S�̕]���l(z')[2]�W���[�J�[�̎g�p(0:������,1:1������,2:�K�i�̈ꕔ(patmake),3:�g�p)
	//     lead�ł�[12]�ɂ͒�o����g�̏�񂪓���

	int seat_num = 0;//�^�[���v���C���̐Ȕԍ�
	int score[5] = { 0 };//���v�X�R�A ���Ԃ̓v���C���ԍ���
	int ranks[5] = { 0 };//1�Q�[���̊e�v���C���̏���(��x��0...��n��4,������Ȃ������ꍇ4) ���Ԃ̓v���C���ԍ���
	int jk1card = 0;//�W���[�J�[��1���ŏo���Ƃ�1
	int win_flag = 0;//�N����������������1
	int ba_cut = 0;//�ꂪ�����Ƃ�1

	int rank_buf = 0;//�����̃����N���i�[�i�ȏ��헪�m�F�p�j


	//�ȏ��`�F�b�N�̃t���O-----------------------
	int seat_order_flag = 0;

	//�����Ă鏇�Ԃ��i�[����z��[->�������Z���Z���Z���Z-]
	int seat_order[5] = { 0 };
	//-------------------------------------------
	//�ȏ��헪�Ńp�X�����������Ɨ���Ă��邩
	int pass_check_lag[2][100000] = { 0 };
	//-------------------------------------------
	//�ꂪ��̎��̃N���C�A���g���Ƃ̒�o����
	int onset_flag = 0;
	//-------------------------------------------

	//�����̃`�F�b�N �����ɏ]���ăT�[�o�A�h���X�A�ڑ��|�[�g�A�N���C�A���g����ύX
	checkArg(argc, argv);


	//1000�����Ȃǂ̃Q�[���̃��[�v�E�Q�[���ɎQ��
	my_playernum = entryToGame();
	while (whole_gameend_flag == 0) {
		one_gameend_flag = 0;                 //1�Q�[�����I��������������t���O��������
		game_count = startGame(own_cards_buf);//���E���h���n�߂� �ŏ��̃J�[�h���󂯎��B
		copyTable(own_cards, own_cards_buf); //��������e�[�u���𑀍삷�邽�߂̃e�[�u���ɃR�s�[

		//100�Q�[�����Ƀf�[�^���쓙�̏���
		if (game_count % 100 == 1) {//�K�������Z�b�g����Ƃ��ɁC�O��̊K���𕽖��ɂ���
			for (i = 0; i < 5; i++) {
				ranks[i] = 2;
			}
			if (game_count != 1) {
				if ((PRINT_SCORE == 1 && my_playernum == 0) || PRINT_SCORE == 2) {
					fprintf(stderr, "%5d game score ", game_count - 1);//�X�R�A�\��
					for (i = 0; i < 5; i++) {
						fprintf(stderr, "%5d", score[i]);
						if (i == 4) {
							fprintf(stderr, "\n");
						}
						else {
							fprintf(stderr, " / ");
						}
					}
				}
			}
		}

		//���������Ƃ̐ȏ��`�F�b�N�̃t���O�̏�����
		seat_order_flag = 0;

		//�Q�[���J�n���Ƀf�[�^�i�[�Ə�����
		for (i = 0; i < 5; i++) {//�ȏ��𒲂ׂ�
			if (own_cards_buf[6][10 + i] == my_playernum) {
				for (j = 0; j < 5; j++) {
					if ((i + j) < 5) {
						ss.seat[j] = own_cards_buf[6][10 + i + j];
					}
					else {
						ss.seat[j] = own_cards_buf[6][5 + i + j];
					}
				}
			}
		}
		for (i = 0; i < 5; i++) {
			ss.score[i] = score[ss.seat[i]];//���v�X�R�A�Z�b�g
			ss.old_rank[i] = ranks[ss.seat[i]];//�O�Q�[���̊K���Z�b�g
			ss.hand_qty[i] = own_cards_buf[6][ss.seat[i]];//�����̎�D�����Z�b�g
			ss.pass[i] = 0;//�p�X/������󋵃��Z�b�g
		}
		for (i = 0; i < 5; i++) {
			ranks[i] = 4;//����4�Ƀ��Z�b�g
		}
		clearTable(used_cards);
		clearTable(search);
		jk1card = 0;
		win_flag = 0;
		ba_cut = 1;

		///�J�[�h����
		if (own_cards[5][0] == 0) { //�J�[�h�������t���O���`�F�b�N ==1�Ő���
			printf("not card-change turn?\n");
			exit(1);
		}
		else { //�e�[�u���ɖ�肪������Ύ��ۂɌ�����
			if (own_cards[5][1] > 0 && own_cards[5][1] < 100) {
				int change_qty = own_cards[5][1];          //�J�[�h�̌�������
				int select_cards[8][15] = { {0} };           //�I�񂾃J�[�h���i�[

				//�������x���A��x���ł���Εs�v�ȃJ�[�h��I�яo��
				/////////////////////////////////////////////////////////////
				//�J�[�h�����̃A���S���Y���͂����ɏ���
				/////////////////////////////////////////////////////////////
				kou_change(select_cards, own_cards, change_qty);
				/////////////////////////////////////////////////////////////
				//�J�[�h�����̃A���S���Y�� �����܂�
				/////////////////////////////////////////////////////////////

				//�I�񂾃J�[�h�𑗐M
				sendChangingCards(select_cards);
			}
			else {
				//�����������ȉ��Ȃ�A��������K�v�͂Ȃ�
			}
		} //�J�[�h���������܂�


		//�ȏ��헪�L�����̊m�F�p---------------------
		if (game_count >= 1) {
			for (i = 0; i < 5; i++)
			{
				if (i == my_playernum) {
					rank_buf = state.player_rank[i];
					break;
				}
			}

			//�������Ȃ����K�����i�[
			seat_tactics_check[(game_count - 1)][1] = rank_buf;
		}
		//-------------------------------------------

		//1�Q�[���ł̃��[�v----------------------------------------------------------//
		while (one_gameend_flag == 0) {     //1�Q�[�����I���܂ł̌J��Ԃ�

		    //��̃J�[�h�\��
			if ((PRINT_STATE == 1 && my_playernum == 0) || PRINT_STATE == 2) {
				fprintf(stderr, "bacard:");
				if (ba_cut == 1 || state.onset == 1) {
					fprintf(stderr, " no card\n");
				}
				else {
					fprintf(stderr, "P%d:", last_playernum);
					if (jk1card == 1) {
						fprintf(stderr, " jk\n");
					}
					else {
						cardprint(ba_cards);
					}
				}
			}
			int select_cards[8][15] = { {0} };      //��o�p�̃e�[�u��
			if (receiveCards(own_cards_buf) == 1) {  //�J�[�h��own_cards_buf�Ɏ󂯎��
												  //�����Ԃ̓ǂݏo��
											  //�����̃^�[���ł��邩���m�F����
			//�����̃^�[���ł���΂��̃u���b�N�����s�����B
				clearCards(select_cards);             //�I�񂾃J�[�h�̃N���A
				copyTable(own_cards, own_cards_buf);   //�J�[�h�e�[�u�����R�s�[
			  /////////////////////////////////////////////////////////////
			  //�A���S���Y����������
			  //�ǂ̃J�[�h���o�����͂����ɂ���
			  /////////////////////////////////////////////////////////////

				//---------------------------------------------------------------
				//�����̎�D���p�X��������𓾂Ȃ��󋵂��m�F
				/*if (pass_check(own_cards, ba_cards) == 1) {
					printf("\n[must_pass]\n");
				}else if (pass_check(own_cards, ba_cards) == 0) {
					printf("\n[must_not_pass]\n");
				}*/
				//---------------------------------------------------------------

				//��̋����]���l
				/*
				if (kou_e_value(own_cards, seat_order, used_cards) >= 86) {
					printf("E value=%d\n", kou_e_value(own_cards, seat_order, used_cards));
				}
				*/



				//�����Ƒ���̐Ȃ̊֌W���i�[����--------------------
				if (seat_order_flag == 0) {
					for (i = 0; i < 5; i++) {
						if (own_cards[6][10 + i] == my_playernum) {
							break;
						}
					}

					for (j = 0; j < 5; j++) {
						seat_order[j] = own_cards[6][10 + (i + j) % 5];
					}

					seat_order_flag = 1;

					//�O�ȃN���C�A���g�̊K���i�[
					if (game_count >= 1) {
						seat_tactics_check[game_count][2] = state.player_rank[seat_order[4]];
					}
				}
				//--------------------------------------------------

				max = 0;//�����ȊO�̃J�[�h�̋����̍ő�
				if (state.rev == 0) {
					for (i = 13; i > 0 && max == 0; i--) {
						if (own_cards[0][i] + own_cards[1][i] + own_cards[2][i] + own_cards[3][i] + used_cards[4][i] != 4) {//���������̃J�[�h�����̎�D�ɂ����
							max = i;//���̋������L�^
						}
					}
				}
				else {
					for (i = 1; i < 14 && max == 0; i++) {
						if (own_cards[0][i] + own_cards[1][i] + own_cards[2][i] + own_cards[3][i] + used_cards[4][i] != 4) {//���������̃J�[�h�����̎�D�ɂ����
							max = i;//���̋������L�^
						}
					}
				}
				if (max == 0) {
					if (state.rev == 0) {
						max = 1;
					}
					else {
						max = 13;
					}
				}

				for (i = 0; i < 13; i++) {
					for (j = 0; j < 6; j++) {
						pattern[i][j] = 0;
					}
				}

				//��D�̑g���ƕ]���l�̌���
				pat_make(pattern, own_cards, max, used_cards, state.joker);

				//�󋵕\��
				if ((PRINT_STATE == 1 && my_playernum == 0) || PRINT_STATE == 2) {
					for (i = 0; i < 5; i++) {
						fprintf(stderr, "P%d:score%4d rank%d hand%2d ", ss.seat[i], ss.score[i], ss.old_rank[i], ss.hand_qty[i]);
						if (ss.pass[i] == 1) {
							fprintf(stderr, "Pass");
						}
						if (ss.pass[i] == 3) {
							fprintf(stderr, "Win");
						}
						fprintf(stderr, "\n");
					}
					fprintf(stderr, "my_cards:");
					cardprint(own_cards);
					fprintf(stderr, "used_cards_table\n");
					fprintf(stderr, "\n");
					Tableprint(used_cards, 0);
				}
				//��o�O�̑g�ƕ]���l�\��
				if ((PRINT_PAT == 1 && my_playernum == 0) || PRINT_PAT == 2) {
					for (i = 0; i < 11; i++) {
						if (pattern[i][1] == 0) {
							i = 11;
						}
						else {
							fprintf(stderr, "[ord%2d][qty%d][sui%2d][Jok%2d][x%3d][rx%3d][y%2d]\n", pattern[i][0], pattern[i][1], pattern[i][2], pattern[i][3], pattern[i][4], pattern[i][5], pattern[i][6]);
						}
					}
					fprintf(stderr, "[set%2d][rev%d][1min%3d][2min%3d][z%3d][rz%3d][r2m%3d]\n", pattern[11][0], pattern[11][1], pattern[11][2], pattern[11][3], pattern[11][4], pattern[11][5], pattern[11][6]);
					fprintf(stderr, "\n");
				}

				//��������ǂ̃J�[�h���o��������-----------------------------------------------------------------------

				if (state.onset == 1) { //��ɃJ�[�h�������Ƃ�
					copyTable(own_cards, own_cards_buf);
					kou_lead(select_cards, own_cards, max, used_cards);//�V�����J�[�h���o���Ƃ��̔���
				}
				else {//���łɏ�ɃJ�[�h������Ƃ�
				  //  jkcard:�W���[�J�[��1���o���Ƃ��P  own_cards:����p�e�[�u���A[0][1]�̓X��3
					if (jk1card == 1 && own_cards[0][1] > 0) {//�W���[�J�[���ł��@���@�X�y�[�h3�����Ă�//////////////////////////////
						k = pattern[11][0];//pattern[11][0]:��D�̑g�����L�^
						own_cards[0][1] = 0;//[0][1]=0:�X��3�����ĂȂ�
						pat_make(pattern, own_cards, max, used_cards, state.joker);//pat_make��D�̑g���ƕ]���l�̌���

						//�g���������Ȃ���Ώo��,serch[][]:�����p�e�[�u��
						if ((last_playernum != own_cards[5][3] && k >= pattern[11][0]) || (last_playernum == own_cards[5][3] && search[4][1] == 1)) {
							select_cards[0][1] = 1;//select_cards:��o�p�̃e�[�u��
						}
						copyTable(own_cards, own_cards_buf);//�����own_cards�����ɖ߂�
					}
					else if (state.qty == own_cards[6][my_playernum])//��D�����Ə�̖����������Ƃ��́Adefault����I��(�o����Ƃ��͏o��)
					{
						if (state.rev == 0) {
							follow(select_cards, own_cards);    //�ʏ펞�̒�o�p 
						}
						else {
							followRev(select_cards, own_cards); //�v�����̒�o�p
						}
					}
					else {////////////////////////////////////////////////////////////////////////////////////////////////////////
						i = 0;
						//�N����������������@���@�K�i����Ȃ���,��̑g����𗬂���P�́E�y�A�̏ꍇ�͒�o���Ȃ�
						if (win_flag == 1 && state.sequence == 0) {
							i = 1;
							if (state.rev == 0) {
								for (j = state.ord + 1; j <= max; j++) {
									if (4 - (own_cards[0][j] + own_cards[1][j] + own_cards[2][j] + own_cards[3][j] + used_cards[4][j]) >= state.qty) {
										//i=0�Ō��݂̒P�ior�������̏�̃J�[�h��苭���̎����Ă�
										i = 0;
										j = 14;
									}
								}
							}
							else if (state.rev == 1) {
								for (j = state.ord - 1; j >= max; j--) {
									if (4 - (own_cards[0][j] + own_cards[1][j] + own_cards[2][j] + own_cards[3][j] + used_cards[4][j]) >= state.qty) {
										i = 0;
										j = 0;
									}
								}
							}
						}
						if (state.sequence == 1) {//�K�i�g�̑I��
							clearTable(select_cards);
							kou_followsequence(select_cards, own_cards, max, used_cards);//�K�i�ŏo���Ƃ��̎v�l
						}
						else if (i == 0 && state.qty != 5) {//�����g�̑I��(5���̏ꍇ�͏o���Ȃ��̂œ���Ȃ�)
							clearTable(select_cards);
							kou_followgroup(select_cards, own_cards, max, used_cards, my_playernum, seat_order, last_playernum, ba_cards, game_count);//�o���J�[�h�̔���
						}
					}
					//�Ō�ɃJ�[�h���o�����v���C���[�������̏ꍇ/////////////////////////////////////////////////////////////////
					if (beEmptyCards(select_cards) == 0 && last_playernum == own_cards[5][3]) {//�����ȊO�p�X
						j = pattern[11][3];//2min���L�^
						k = pattern[11][0];//�g�����L�^
						cardsDiff(own_cards, select_cards);//��o�\��̑g��own_cards���甲��
						pat_make(pattern, own_cards, max, used_cards, state.joker);//��o��̃p�^�[�������
						//�W���[�J�[���肪�����܂��̉\������
						if (pattern[11][3] < 100) {//��o��2min<100�̏ꍇ(2min>=100�̏ꍇ�͒�o)
							if (j > 70 && j > pattern[11][3]) {//��o��<��o�O����o�O>70
								clearTable(select_cards);
							}
							//�W���[�J�[�P�̂͏o���Ȃ�
							if (select_cards[0][0] == 2)
								select_cards[0][0] = 0;
							//�����J�[�h�̖����g�͏o���Ȃ�
							if (state.sequence == 0 && state.rev == 0) {
								for (i = max; i <= 13; i++) {
									select_cards[0][i] = 0;
									select_cards[1][i] = 0;
									select_cards[2][i] = 0;
									select_cards[3][i] = 0;
								}
							}
							if (state.sequence == 0 && state.rev == 1) {
								for (i = max; i >= 1; i--) {
									select_cards[0][i] = 0;
									select_cards[1][i] = 0;
									select_cards[2][i] = 0;
									select_cards[3][i] = 0;
								}
							}
							//�g�̐�������Ȃ��ꍇ�͏o���Ȃ�
							if (k <= pattern[11][0]) {
								clearTable(select_cards);
							}
						}
					}
				}//���łɏ�ɃJ�[�h������Ƃ��̔��肱���܂�

			/////////////////////////////////////////////////////////////
			//�A���S���Y���͂����܂�
			/////////////////////////////////////////////////////////////
				accept_flag = sendCards(select_cards);//cards���o
			}
			else {
				//�����̃^�[���ł͂Ȃ���
				//�K�v�Ȃ炱���ɏ������L�q����
				if (state.onset == 1) {
					onset_flag = 1;
				}
			}
			//���̃^�[���ɒ�o���ꂽ���ʂ̃e�[�u���󂯎��,��ɏo���J�[�h�̏�����͂���
			lookField(ba_cards_buf);
			copyTable(ba_cards, ba_cards_buf);

			///////////////////////////////////////////////////////////////
			//�J�[�h���o���ꂽ���� �N�����J�[�h���o���O�̏����͂����ɏ���
			///////////////////////////////////////////////////////////////
			last_playernum = getLastPlayerNum(ba_cards);

			//-------------------------------------------------------------
			//client_log[2][4][14][4]�ɒ�o�J�[�h�i�[
			if (onset_flag == 1) {
				if (state.qty == 1) {
					for (i = 0; i <= 4; i++) {
						for (j = 0; j <= 14; j++) {
							if (state.ord==j && state.suit[i]==1) {
								if (state.rev == 0) {
									client_log[0][i][j][last_playernum] = 1;
								}else if (state.rev == 1) {
									client_log[0][i][j][last_playernum] = 2;
								}
							}
						}
					}
				}
				else if (state.qty == 2) {
					for (i = 0; i <= 4; i++) {
						for (j = 0; j <= 14; j++) {
							if (state.ord == j && state.suit[i] == 1) {
								if (state.rev == 0) {
									client_log[1][i][j][last_playernum] = 1;
								}
								else if (state.rev == 1) {
									client_log[1][i][j][last_playernum] = 2;
								}
							}
						}
					}
				}

				onset_flag = 0;
			}
			//-------------------------------------------------------------

			//��o��̃J�[�h�e�[�u���ɏ�̃J�[�h���L�^
			ba_cut = 0;
			for (i = 0; i <= 4; i++) {
				for (j = 0; j <= 14; j++) {
					if (ba_cards[i][j] == 1) {
						used_cards[i][j] = 1;
						if (j == 6) {//8�؂�
							ba_cut = 1;
						}
						else if (i == 0 && j == 1 && jk1card == 1) {//�W���[�J�[�ւ̃X�y�[�h3
							ba_cut = 1;
						}
					}
					if (ba_cards[i][j] == 2) {
						used_cards[4][14] = 1;
						if (j == 6) {//8�؂�
							ba_cut = 1;
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

			used_cards[5][1] = 0;//��ɏo���J�[�h�̍��v�������i�[
			for (j = 1; j <= 13; j++) {
				used_cards[4][j] = used_cards[0][j] + used_cards[1][j] + used_cards[2][j] + used_cards[3][j];
				used_cards[5][1] += used_cards[4][j];
			}
			used_cards[5][1] += used_cards[4][14];

			//�ꂪ�W���[�J�[1���̏ꍇ�̂݃t���O��1�ɕۂ�
			if ((state.ord == 0 || state.ord == 14) && state.qty == 1) {
				jk1card = 1;
			}
			for (i = 0; i < 5; i++) {//�^�[���v���C���̐Ȕԍ����i�[
				if (ss.seat[i] == own_cards_buf[5][3]) {
					seat_num = i;
				}
			}

			//used_cards[5][0]=���O�̃^�[���܂łɏ�ɏo���J�[�h�̍��v����
			if (used_cards[5][0] == used_cards[5][1]) {//�^�[���̑O��ŃJ�[�h���o�������������Ƃ�(�p�X)
				ss.pass[seat_num] = 1;
				if ((PRINT_STATE == 1 && my_playernum == 0) || PRINT_STATE == 2) {
					fprintf(stderr, " turn :P%d:Pass\n", ba_cards[5][3]);
				}
			}
			else {//�����łȂ��ꍇ��o�����v���C���̎�D���������炷
				ss.hand_qty[seat_num] -= (used_cards[5][1] - used_cards[5][0]);
				if ((PRINT_STATE == 1 && my_playernum == 0) || PRINT_STATE == 2) {
					fprintf(stderr, " turn :P%d:use:", ba_cards[5][3]);
					if (jk1card == 1) {
						fprintf(stderr, " jk\n");
					}
					else {
						cardprint(ba_cards);
					}
				}
			}
			//������0�ɂȂ����ꍇ(������)
			if (ss.hand_qty[seat_num] == 0 && ss.pass[seat_num] != 3) {
				ranks[ss.seat[seat_num]] = 0;
				for (i = 0; i < 5; i++) {
					if (ss.pass[i] == 3) {//�������Ă��鐔�J�E���g
						ranks[ss.seat[seat_num]]++;//���ʋL�^
					}
				}
				ss.pass[seat_num] = 3;//�������ԂƂ���
				win_flag = 1;
			}
			else {
				win_flag = 0;
			}
			used_cards[5][0] = used_cards[5][1];//��ɏo���J�[�h�̍��v�����X�V

			for (i = 0; i < 5; i++) {
				if (ss.pass[i] == 0) {
					break;
				}
			}
			if (i == 5) {//�S���p�X
				ba_cut = 1;

				//----------------------------------------------------------------
				if (ba_nagare_last_playernum != 10) {
					//�ȏ��헪�Ńp�X�������̍Ō�ɏo�����N���C�A���g�i�O�̃N���C�A���g�j���ŏI�I�ɏ�𗬂��Ă��邩
					if (last_playernum == ba_nagare_last_playernum) {
						//�\��ʂ藬�ꂽ
						pass_check_lag[0][game_count]++;
						ba_nagare_last_playernum = 10;
					}
					else {
						//�\��ʂ藬��Ȃ�����
						pass_check_lag[1][game_count]++;
						ba_nagare_last_playernum = 10;
					}
				}
				//----------------------------------------------------------------
			}

			//�S���p�X�܂���8�؂�A�W���[�J�[�ւ̃X�y�[�h3�̏ꍇ�p�X��Ԃƃt���O���Z�b�g
			if (ba_cut == 1) {
				for (i = 0; i < 5; i++) {
					if (ss.pass[i] <= 2) {
						ss.pass[i] = 0;
					}
				}
				jk1card = 0;
				win_flag = 0;
			}

			//�v���C���̏�ԕ\��
			if ((PRINT_STATE == 1 && my_playernum == 0) || PRINT_STATE == 2) {
				fprintf(stderr, "state :");
				if (ba_cards[5][6] == 1) {
					fprintf(stderr, "rev ");
				}
				if (ba_cut == 1) {
					printf("ba_cut");
				}
				else if (ba_cards[5][7] == 1) {
					fprintf(stderr, "bind ");
				}
				printf("\n\n");

				for (i = 0; i < 5; i++) {
					fprintf(stderr, "P%d:score%4d rank%d hand%2d ", ss.seat[i], ss.score[i], ss.old_rank[i], ss.hand_qty[i]);
					if (ss.pass[i] == 1) {
						printf("Pass");
					}
					if (ss.pass[i] == 3) {
						printf("Win");
					}
					printf("\n");
				}
				printf("\n");
			}

			///////////////////////////////////////////////////////////////
			//�J�[�h���o���ꂽ���� �N�����J�[�h���o���O�̏��������܂�
			///////////////////////////////////////////////////////////////

			//���̃Q�[�����I��������ۂ��̒ʒm���T�[�o���炤����B
			switch (beGameEnd()) {
			case 0: //0�̂Ƃ��Q�[���𑱂���
				one_gameend_flag = 0;
				whole_gameend_flag = 0;
				break;
			case 1: //1�̂Ƃ� 1�Q�[���̏I��
				one_gameend_flag = 1;
				whole_gameend_flag = 0;
				if (g_logging == 1) {
					printf("game #%d was finished.\n", game_count);
				}
				break;
			default: //���̑��̏ꍇ �S�Q�[���̏I��
				one_gameend_flag = 1;
				whole_gameend_flag = 1;
				if (g_logging == 1) {
					printf("All game was finished(Total %d games.)\n", game_count);
				}
				break;
			}
		}
		//1�Q�[�����I���܂ł̌J��Ԃ������܂�---------------------------------------------------------//

		//1�Q�[�����ƂɃ��Z�b�g
		for (i = 0; i <= 4; i++) {
			for (j = 0; j <= 14; j++) {
				for (k = 0; k <= 4; k++) {
					client_log[0][i][j][k] = 0;
					client_log[1][i][j][k] = 0;
				}
			}
		}

		for (i = 0; i < 5; i++) {
			score[i] += (5 - ranks[i]);
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

	}//�S�Q�[�����I���܂ł̌J��Ԃ������܂�-----------------------------------------------------


	//�����p�Q�[���I���̏�Ԋm�F---------------------------

	//printf("\nseat_tactics_on:%d\n", seat_tactics_check_on);
	//printf("seat_tactics_off:%d\n", seat_tactics_check_off);

	//printf("\nba_nagare_single:%d\n", ba_nagare_single);
	//printf("ba_nagare_double:%d\n", ba_nagare_double);
	//printf("ba_nagare_three_cards:%d\n", ba_nagare_three_cards);
	//printf("ba_nagare_max:%d\n", ba_nagare_max_count);
	//printf("	ba_nagare_max_single:%d\n", ba_nagare_max_count_single);
	//printf("		ba_nagare_max_single_nomal:%d\n", ba_nagare_max_count_single_nomal);
	//printf("		ba_nagare_max_single_nomal_rev:%d\n", ba_nagare_max_count_single_nomal_rev);
	//printf("		ba_nagare_max_single_sibari:%d\n", ba_nagare_max_count_single_sibari);
	//printf("		ba_nagare_max_single_sibari_rev:%d\n", ba_nagare_max_count_single_sibari_rev);
	//printf("	ba_nagare_max_double:%d\n", ba_nagare_max_count_double);
	//printf("		ba_nagare_max_double_nomal:%d\n", ba_nagare_max_count_double_nomal);
	//printf("		ba_nagare_max_double_nomal_rev:%d\n", ba_nagare_max_count_double_nomal_rev);
	//printf("		ba_nagare_max_double_sibari:%d\n", ba_nagare_max_count_double_sibari);
	//printf("		ba_nagare_max_double_sibari_rev:%d\n", ba_nagare_max_count_double_sibari_rev);
	//printf("\nflag_count_normal:%d\n", flag_count_normal);
	//printf("pass_check_count:%d\n", pass_check_count);
	//printf("client_log_pass_off_count:%d\n", client_log_st_pass_off_count);

	//printf("\n\n");
	////---------------------------
	//int rank_total[5] = { 0 };//�ȏ��헪�����s���ꂽ�ۂ̊K�����i�[�A0:��x���`4:��n��
	//float rank_sum = 0, zero_sum = 0, one_sum = 0, two_sum = 0, three_sum = 0, four_sum = 0;
	//float zero_rate, one_rate, two_rate, three_rate, four_rate;

	//for (i = 0; i < 1000; i++)
	//{
	//	if (seat_tactics_check[i][0] >= 1) {
	//		//�K�����i�[
	//		if (seat_tactics_check[i][1] == 0) {
	//			rank_total[0]++;
	//		}
	//		else if (seat_tactics_check[i][1] == 1) {
	//			rank_total[1]++;
	//		}
	//		else if (seat_tactics_check[i][1] == 2) {
	//			rank_total[2]++;
	//		}
	//		else if (seat_tactics_check[i][1] == 3) {
	//			rank_total[3]++;
	//		}
	//		else if (seat_tactics_check[i][1] == 4) {
	//			rank_total[4]++;
	//		}
	//	}
	//}

	//printf("daihugou:%d\n", rank_total[0]);
	//printf("hugou:%d\n", rank_total[1]);
	//printf("heimin:%d\n", rank_total[2]);
	//printf("hinmin:%d\n", rank_total[3]);
	//printf("daihinmin:%d\n", rank_total[4]);


	////--------------------------------------------------------------------------
	//printf("\n\n<seat_tactics>\n");
	////�e�����̏�
	//for (i = 0; i < 1000; i++)
	//{
	//	if (seat_tactics_check[i][0] >= 1) {
	//		printf("%3d:%d , rank%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1]);
	//		rank_sum++;
	//		if (seat_tactics_check[i][1] == 0) {
	//			zero_sum++;
	//		}
	//		else if (seat_tactics_check[i][1] == 1) {
	//			one_sum++;
	//		}
	//		else if (seat_tactics_check[i][1] == 2) {
	//			two_sum++;
	//		}
	//		else if (seat_tactics_check[i][1] == 3) {
	//			three_sum++;
	//		}
	//		else if (seat_tactics_check[i][1] == 4) {
	//			four_sum++;
	//		}
	//	}
	//}
	//zero_rate = zero_sum / rank_sum;
	//one_rate = one_sum / rank_sum;
	//two_rate = two_sum / rank_sum;
	//three_rate = three_sum / rank_sum;
	//four_rate = four_sum / rank_sum;

	//printf("\ndaihugou_rate :%.1f\n", (zero_sum / rank_sum) * 100);
	//printf("hugou_rate    :%.1f\n", (one_sum / rank_sum) * 100);
	//printf("heimin_rate   :%.1f\n", (two_sum / rank_sum) * 100);
	//printf("hinmin_rate   :%.1f\n", (three_sum / rank_sum) * 100);
	//printf("daihinmin_rate:%.1f\n", (four_sum / rank_sum) * 100);

	//rank_sum = 0;
	//zero_sum = 0;
	//one_sum = 0;
	//two_sum = 0;
	//three_sum = 0;
	//four_sum = 0;
	//four_sum = 0;

	////--------------------------------------------------------------------------
	//printf("\n\n<seat_tactics_lot>\n");
	////�e�����̏󋵁i�ȏ��헪���������j
	//for (i = 0; i < 1000; i++)
	//{
	//	if (seat_tactics_check[i][0] >= 2) {
	//		printf("%3d:%d , rank%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1]);
	//		rank_sum++;
	//		if (seat_tactics_check[i][1] == 0) {
	//			zero_sum++;
	//		}
	//		else if (seat_tactics_check[i][1] == 1) {
	//			one_sum++;
	//		}
	//		else if (seat_tactics_check[i][1] == 2) {
	//			two_sum++;
	//		}
	//		else if (seat_tactics_check[i][1] == 3) {
	//			three_sum++;
	//		}
	//		else if (seat_tactics_check[i][1] == 4) {
	//			four_sum++;
	//		}
	//	}
	//}

	//zero_rate = zero_sum / rank_sum;
	//one_rate = one_sum / rank_sum;
	//two_rate = two_sum / rank_sum;
	//three_rate = three_sum / rank_sum;
	//four_rate = four_sum / rank_sum;

	//printf("\ndaihugou_rate :%.1f\n", (zero_sum / rank_sum) * 100);
	//printf("hugou_rate    :%.1f\n", (one_sum / rank_sum) * 100);
	//printf("heimin_rate   :%.1f\n", (two_sum / rank_sum) * 100);
	//printf("hinmin_rate   :%.1f\n", (three_sum / rank_sum) * 100);
	//printf("daihinmin_rate:%.1f\n", (four_sum / rank_sum) * 100);

	//zero_sum = 0;
	//one_sum = 0;
	//two_sum = 0;
	//three_sum = 0;
	//four_sum = 0;

	////--------------------------------------------------------------------------
	//printf("\n\n<seat_tactics_few>\n");
	////�e�����̏󋵁i�ȏ��헪�����Ȃ����j
	//for (i = 0; i < 1000; i++)
	//{
	//	if (seat_tactics_check[i][0] == 1) {
	//		printf("%3d:%d , rank%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1]);
	//	}
	//	rank_sum++;
	//	if (seat_tactics_check[i][1] == 0) {
	//		zero_sum++;
	//	}
	//	else if (seat_tactics_check[i][1] == 1) {
	//		one_sum++;
	//	}
	//	else if (seat_tactics_check[i][1] == 2) {
	//		two_sum++;
	//	}
	//	else if (seat_tactics_check[i][1] == 3) {
	//		three_sum++;
	//	}
	//	else if (seat_tactics_check[i][1] == 4) {
	//		four_sum++;
	//	}
	//}

	//zero_rate = zero_sum / rank_sum;
	//one_rate = one_sum / rank_sum;
	//two_rate = two_sum / rank_sum;
	//three_rate = three_sum / rank_sum;
	//four_rate = four_sum / rank_sum;

	//printf("\ndaihugou_rate :%.1f\n", (zero_sum / rank_sum) * 100);
	//printf("hugou_rate    :%.1f\n", (one_sum / rank_sum) * 100);
	//printf("heimin_rate   :%.1f\n", (two_sum / rank_sum) * 100);
	//printf("hinmin_rate   :%.1f\n", (three_sum / rank_sum) * 100);
	//printf("daihinmin_rate:%.1f\n", (four_sum / rank_sum) * 100);

	////-------------------------------------------------------
	////1000�����ł̔����񐔂Ə��ʂ̊֌W���o�͂���
	//int my_score;
	//int end_rank;	// 5�͍ŏI�X�R�A�P�ʁ`1�͍ŏI�X�R�A5��

	//my_score = score[my_playernum];
	//end_rank = 5;	//1�ʂƉ���

	//for (i = 0; i < 5; i++) {
	//	if (i == my_playernum) {
	//		continue;
	//	}

	//	if (my_score < score[i]) {
	//		end_rank--;
	//	}
	//}

	//FILE *fp4;

	//fp4 = fopen("st_all_log.csv", "a+");

	//if (fp4 == NULL) {
	//	printf("[st_all_log.csv]file not open\n");
	//}
	//else {
	//	fprintf(fp4, "%d,%d\n", seat_tactics_check_on, end_rank);

	//	printf("[st_all_log.csv]file write OK\n");
	//}

	//fclose(fp4);
	////-------------------------------------------------------
	////�t�@�C���Ƀ��O�����o��
	////�ʏ폑�����݃��[�h"w"�A�ǉ��������݃��[�h"a+"�i�ǉ��̂݁j
	//FILE *fp;

	//fp = fopen("st_log.csv", "a+");

	//if (fp == NULL) {
	//	printf("[st_log.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++)
	//	{
	//		fprintf(fp, "%d,%d,%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1]);
	//	}

	//	printf("[st_log.csv]file write OK\n");
	//}

	//fclose(fp);
	////----------
	//FILE *fp1;

	//fp1 = fopen("st_log_1.csv", "a+");

	//if (fp1 == NULL) {
	//	printf("[st_log_1.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++)
	//	{
	//		if (seat_tactics_check[i][0] == 1) {
	//			fprintf(fp1, "%d,%d,%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1]);
	//		}

	//	}

	//	printf("[st_log_1.csv]file write OK\n");
	//}

	//fclose(fp1);
	////----------
	//FILE *fp2;

	//fp2 = fopen("st_log_2.csv", "a+");

	//if (fp2 == NULL) {
	//	printf("[st_log_2.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++)
	//	{
	//		if (seat_tactics_check[i][0] == 2) {
	//			fprintf(fp2, "%d,%d,%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1]);
	//		}

	//	}

	//	printf("[st_log_2.csv]file write OK\n");
	//}

	//fclose(fp2);
	////----------
	//FILE *fp3;

	//fp3 = fopen("st_log_3_over.csv", "a+");

	//if (fp3 == NULL) {
	//	printf("[st_log_3.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++)
	//	{
	//		if (seat_tactics_check[i][0] >= 3) {
	//			fprintf(fp3, "%d,%d,%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1]);
	//		}

	//	}

	//	printf("[st_log_3.csv]file write OK\n");
	//}

	//fclose(fp3);

	////-------------------------------------------------------
	////�����񐔂Ǝ����̊K���A�O�̃N���C�A���g�̊K���̊֌W���o��(rank=0�A��x��)
	//FILE *fp5;

	//fp5 = fopen("st_log_rank0.csv", "a+");

	//if (fp5 == NULL) {
	//	printf("[st_log_rank0.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++)
	//	{
	//		if (seat_tactics_check[i][0] >= 1 && seat_tactics_check[i][1] == 0) {
	//			fprintf(fp5, "%d,%d,%d,%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1], seat_tactics_check[i][2]);
	//		}

	//	}

	//	printf("[st_log_ramk0.csv]file write OK\n");
	//}

	//fclose(fp5);

	////-------------------------------------------------------
	////-------------------------------------------------------
	////�����񐔂Ǝ����̊K���A�O�̃N���C�A���g�̊K���̊֌W���o��(rank=1�A�x��)
	//FILE *fp6;

	//fp6 = fopen("st_log_rank1.csv", "a+");

	//if (fp6 == NULL) {
	//	printf("[st_log_rank1.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++)
	//	{
	//		if (seat_tactics_check[i][0] >= 1 && seat_tactics_check[i][1] == 1) {
	//			fprintf(fp6, "%d,%d,%d,%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1], seat_tactics_check[i][2]);
	//		}

	//	}

	//	printf("[st_log_ramk1.csv]file write OK\n");
	//}

	//fclose(fp6);

	////-------------------------------------------------------
	////-------------------------------------------------------
	////�����񐔂Ǝ����̊K���A�O�̃N���C�A���g�̊K���̊֌W���o��(rank=2�A����)
	//FILE *fp7;

	//fp7 = fopen("st_log_rank2.csv", "a+");

	//if (fp7 == NULL) {
	//	printf("[st_log_rank2.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++)
	//	{
	//		if (seat_tactics_check[i][0] >= 1 && seat_tactics_check[i][1] == 2) {
	//			fprintf(fp7, "%d,%d,%d,%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1], seat_tactics_check[i][2]);
	//		}

	//	}

	//	printf("[st_log_ramk2.csv]file write OK\n");
	//}

	//fclose(fp7);

	////-------------------------------------------------------
	////-------------------------------------------------------
	////�����񐔂Ǝ����̊K���A�O�̃N���C�A���g�̊K���̊֌W���o��(rank=3�A�n��)
	//FILE *fp8;

	//fp8 = fopen("st_log_rank3.csv", "a+");

	//if (fp8 == NULL) {
	//	printf("[st_log_rank3.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++)
	//	{
	//		if (seat_tactics_check[i][0] >= 1 && seat_tactics_check[i][1] == 3) {
	//			fprintf(fp8, "%d,%d,%d,%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1], seat_tactics_check[i][2]);
	//		}

	//	}

	//	printf("[st_log_ramk3.csv]file write OK\n");
	//}

	//fclose(fp8);

	////-------------------------------------------------------
	////-------------------------------------------------------
	////�����񐔂Ǝ����̊K���A�O�̃N���C�A���g�̊K���̊֌W���o��(rank=4�A��n��)
	//FILE *fp9;

	//fp9 = fopen("st_log_rank4.csv", "a+");

	//if (fp9 == NULL) {
	//	printf("[st_log_rank4.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++)
	//	{
	//		if (seat_tactics_check[i][0] >= 1 && seat_tactics_check[i][1] == 4) {
	//			fprintf(fp9, "%d,%d,%d,%d\n", i, seat_tactics_check[i][0], seat_tactics_check[i][1], seat_tactics_check[i][2]);
	//		}

	//	}

	//	printf("[st_log_ramk4.csv]file write OK\n");
	//}

	//fclose(fp9);

	////-------------------------------------------------------
	////-------------------------------------------------------
	////�ȏ��헪�ŗ\�肵�Ă����ʂ�O�̃N���C�A���g�ŗ��ꂽ���ǂ����̕]��
	//FILE *fp10;

	//fp10 = fopen("nagare_kakuritu.csv", "a+");

	//if (fp10 == NULL) {
	//	printf("[nagare_kakuritu.csv]file not open\n");
	//}
	//else {
	//	for (i = 0; i < 1000; i++) {
	//		fprintf(fp10, "%d,%d,%d,%d\n", i, pass_check_lag[0][i], pass_check_lag[1][i], (pass_check_lag[0][i] + pass_check_lag[1][i]));
	//	}

	//	printf("[nagare_kakuritu.csv]file write OK\n");
	//}

	//fclose(fp10);

	//-------------------------------------------------------

	//�����p�Q�[���I���̏�Ԋm�F�I���---------------------------

	if ((PRINT_SCORE == 1 && my_playernum == 0) || PRINT_SCORE == 2) {
		fprintf(stderr, "     Final score ");//�X�R�A�\��
		for (i = 0; i < 5; i++) {
			fprintf(stderr, "%5d", score[i]);
			if (i == 4) {
				fprintf(stderr, "\n");
			}
			else {
				fprintf(stderr, " / ");
			}
		}
	}

	//�\�P�b�g����ďI��
	if (closeSocket() != 0) {
		printf("failed to close socket\n");
		exit(1);
	}
	exit(0);
}
