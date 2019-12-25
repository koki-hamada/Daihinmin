/*daihinmin*/ 

struct state_type
{
  int ord;        //����
  int sequence;   //�K�i
  int qty;        //����
  int rev;        //�v��
  int b11;        //11�o�b�N
  int lock;       //����
  int onset;      //�ꂪ�V�������ǂ���
  int suit[5];    //���[��

  int player_qty[5];  //�e�v���C���[��D�̖���
  int player_rank[5]; //�e�v���C���[�����N
  int seat[5];        //�e�v���C���[���ȏ�

  int joker;      //jo-ka-
}state;

struct seatsort_state//�ȏ�(������0�Ƃ���)�Ƀv���C���̏�Ԃ��i�[
{
  int seat[5];//�e�Ȃ̃v���C���ԍ�
  int score[5];//���v�X�R�A
  int old_rank[5];//�O��Q�[���̊K��
  int hand_qty[5];//��D����
  int pass[5];//�p�X�ł����1,��D����0�ł����3
}ss;

//��𗬂���Ɣ��f����l
#define STRONG 86

//��Ԃ�\�����Ȃ��ꍇ0
//�ڑ����ŏ��̂Ƃ��̂ݕ\������ꍇ1
//�\������ꍇ2
#define PRINT_SCORE 0
#define PRINT_PAT 0
#define PRINT_STATE 0
//define�̕ύX��K�p����ɂ�c�t�@�C������x�ύX����K�v������

void showState(struct state_type *state);
void getField(int cards[8][15]);
void getState(int cards[8][15]);
void getField(int cards[8][15]);

void cardsOr(int cards1[8][15],int cards2[8][15]);
void cardsAnd(int cards1[8][15],int cards2[8][15]);
void cardsDiff(int cards1[8][15],int cards2[8][15]);
void cardsNot(int cards[8][15]);

void outputTable(int table_val[8][15]);
void copyTable(int dest_table[8][15],int org_table[8][15]);
void copyCards(int cardsTar[8][15],int cardsOrg[8][15]);
void clearCards( int cards[8][15]);
void clearTable( int cards[8][15]);
int beEmptyCards(int cards[8][15]);
int qtyOfCards(int cards[8][15]);

void makeJKaidanTable(int tgt_cards[][15], int my_cards[][15]);
void makeKaidanTable(int tgt_cards[][15], int my_cards[][15]);
void makeGroupTable(int tgt_cards[][15], int my_cards[][15]);
void makeJGroupTable(int tgt_cards[][15], int my_cards[][15]);

void lowCards(int out_cards[8][15],int my_cards[8][15],int threshold);
void highCards(int out_cards[8][15],int my_cards[8][15],int threshold);
int nCards(int n_cards[8][15],int target[8][15],int n);

void lockCards(int target_cards[8][15],int suit[5]);
void lowGroup(int out_cards[8][15],int my_cards[8][15],int group[8][15]);
void highGroup(int out_cards[8][15],int my_cards[8][15],int group[8][15]) ;
void lowSequence(int out_cards[8][15],int my_cards[8][15],int sequence[8][15]);
void highSequence (int out_cards[8][15],int my_cards[8][15],int sequence[8][15]);
void removeGroup(int out_cards[8][15],int my_cards[8][15],int group[8][15]);
void removeSequence(int out_cards[8][15],int my_cards[8][15],int sequence[8][15]);
void lowSolo(int out_cards[8][15],int my_cards[8][15],int joker_flag);
void highSolo(int out_cards[8][15],int my_cards[8][15],int joker_flag);

void change(int out_cards[8][15],int my_cards[8][15],int num_of_change);

void lead(int out_cards[8][15],int my_cards[8][15]);
void leadRev(int out_cards[8][15],int my_cards[8][15]);

void followSolo(int out_cards[8][15],int my_cards[8][15],int joker_flag);
void followGroup(int out_cards[8][15],int my_cards[8][15],int joker_flag);
void followSequence(int out_cards[8][15],int my_cards[8][15],int joker_flag);
void followSoloRev(int out_cards[8][15],int my_cards[8][15],int joker_flag);
void followGroupRev(int out_cards[8][15],int my_cards[8][15],int joker_flag);
void followSequenceRev(int out_cards[8][15],int my_cards[8][15],int joker_flag);
void follow(int out_cards[8][15],int my_cards[8][15]);
void followRev(int out_cards[8][15],int my_cards[8][15]);

int cmpCards(int cards1[8][15],int  cards2[8][15]);
int cmpState(struct state_type* state1,struct state_type* state2);

int getLastPlayerNum(int ba_cards[8][15]);


int setmake(int out_cards[][15],int my_cards[8][15],int joker_flag);
int kaidanhand(int select_cards[][15],int search[8][15],int own_cards[8][15]);
int grouphand(int select_cards[][15],int search[8][15],int n);
void kou_lead(int select_cards[][15],int own_cards[8][15],int max,int used_cards[8][15]);
void kou_followgroup(int select_cards[][15],int own_cards[8][15],int max,int used_cards[8][15],int my_player_num,int seat_order[5],int last_playnum,int ba_cards[8][15],int game_count);
void kou_followsequence(int select_cards[][15],int own_cards[8][15],int max,int used_cards[8][15]);
void kou_change(int out_cards[8][15],int my_cards[8][15],int num_of_change);

void pat_make(int pattern[][7],int own_cards[8][15],int max,int used_cards[8][15],int joker_flag);
void value_strong(int pattern[][7],int own_cards[8][15],int used_cards[8][15],int rev,int n);

void cardprint(int own_cards[8][15]);
void Tableprint(int own_cards[8][15],int print);

//----------------------------------------------------------------------------------//
//--------------------------------�V�����֐�----------------------------------------//
//----------------------------------------------------------------------------------//

//�O���[�o���ϐ�
extern int seat_tactics_check_on;
extern int seat_tactics_check_off;
extern int ba_nagare_single;
extern int ba_nagare_double;
extern int ba_nagare_three_cards;
extern int ba_nagare_max_count;
extern int ba_nagare_max_count_single;
extern int ba_nagare_max_count_single_nomal;
extern int ba_nagare_max_count_single_nomal_rev;
extern int ba_nagare_max_count_single_sibari;
extern int ba_nagare_max_count_single_sibari_rev;
extern int ba_nagare_max_count_double;
extern int ba_nagare_max_count_double_nomal;
extern int ba_nagare_max_count_double_nomal_rev;
extern int ba_nagare_max_count_double_sibari;
extern int ba_nagare_max_count_double_sibari_rev;
extern int flag_count_normal;
extern int pass_check_count;
extern int ba_nagare_last_playernum;

//[0][0]�ɃW���[�J�[��o�Ƃ���A[2]��1���g��2���g���A�J�[�h��[4][14]��2�����Ƀv���C�����Ƃ�4�����ڒǉ��I�ȃC���[�W
//0�͏ꂪ��̎��o�������ƂȂ��A�P�F��v����Ԃŏo�����A�Q�F�v����Ԃŏo����
//419�s�ɋL�q
extern int client_log[2][4][14][5];
extern int client_log_st_pass_off_count;

//�O���[�o���ϐ�
extern int seat_tactics_flag;

//�O���[�o���ϐ��i�ȏ���p�̗L�����m�F�j
//[1000]�F[0]��1�Q�[���ڂ̐ȏ��헪���s�񐔁A�P�Q�[���ڌ��ʂ̎����̊K�����i�[����
//[1000]�F[1]��2�Q�[���ڂ̐ȏ��헪���s�񐔁A�Q�Q�[���ڌ��ʂ̎����̊K�����i�[����A�P�Q�[���ڌ��ʂ̎����̑O�̃N���C�A���g�̊K�����i�[
//[0]�ɐȏ���p�̎��s�񐔁A[1]�ɊK�����i�[(�O�F��x���`�S�F��n��)�A[3]�ɑO�̐Ȃ̃N���C�A���g�̊K�����i�[
extern int seat_tactics_check[100000][3];

int seat_tactics(int own_cards[8][15], int my_player_num, int seat_order[5], int last_playnum, int used_cards[8][15], int game_count);
int seat_order_check(int own_cards[8][15], int my_player_num, int seat_order[5], int last_playnum);
int remaining_client(void);
int remaining_client_max_hand(int my_player_num);
int remaining_client_min_hand(int my_player_num);
int front_client_hand_num_check(int own_cards[8][15], int my_player_num, int seat_order[5], int last_playnum);
int ba_nagare(int own_cards[8][15], int seat_order[5], int used_cards[8][15]);
int ba_nagare_single_variable(int own_cards[8][15], int seat_order[5], int used_cards[8][15]);
int ba_nagare_double_variable(int own_cards[8][15], int seat_order[5], int used_cards[8][15]);
int now_cards_ord_used(int used_cards[8][15], int own_cards[8][15]);
int now_cards_ord(int used_cards[8][15]);
int ba_nagare_max(int own_cards[8][15], int used_cards[8][15]);
int ba_nagare_max_single(int own_cards[8][15], int used_cards[8][15]);
int ba_nagare_max_double(int own_cards[8][15], int used_cards[8][15]);

int pass_check(int own_cards[8][15], int ba_cards[8][15]);
int pass_check_solo(int own_cards[8][15], int ba_cards[8][15]);
int pass_check_group(int own_cards[8][15], int ba_cards[8][15]);
int pass_check_kaidan(int own_cards[8][15], int ba_cards[8][15]);

int kou_e_value(int own_cards[8][15], int seat_order[5], int used_cards[8][15]);

int front_client_log_st(int own_cards[8][15], int my_player_num, int seat_order[5], int last_playnum, int used_cards[8][15], int game_count);

/*
<�֐��֘A�}>
seat_tactics
	remaining_client
	remaining_client_max_hand
	front_client_hand_num_check
	seat_order_chack
	ba_nagare
		ba_nagare_max
			ba_nagare_max_single
			ba_nagare_max_double
		if(ba_nagare_mode==1)
		if(ba_nagare_mode==2)
			ba_nagare_single_variable
				now_cards_ord_used
				now_cards_ord
			ba_nagare_double_variacle
				now_cards_ord_used
				now_cards_ord

pass_check
	pass_check_solo
	pass_check_group
	pass_check_kaidan
*/