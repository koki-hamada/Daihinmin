/*daihinmin*/ 

struct state_type
{
  int ord;
  int sequence;
  int qty;
  int rev;
  int b11;
  int lock;
  int onset;
  int suit[5];

  int player_qty[5];
  int player_rank[5];
  int seat[5];

  int joker;
}state;

struct seatsort_state//�ȏ�(������0�Ƃ���)�Ƀv���C���̏�Ԃ��i�[
{
  int seat[5];//�e�Ȃ̃v���C���ԍ�
  int score[5];//���v�X�R�A
  int old_rank[5];//�O��Q�[���̊K��
  int hand_qty[5];//��D����
  int pass[5];//�p�X�ł����1,��D����0�ł����3
}ss;

typedef struct Pattern
{
  int num;
  int suits;
  int count;
  int joker;
  int jnum;
  int jsuits;
  int x;
}pattern;


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
void kou_lead(int select_cards[][15],int own_cards[8][15],int max,int used_cards[8][15],int tes[3][4][3]);
void kou_followgroup(int select_cards[][15],int own_cards[8][15],int max,int used_cards[8][15],int tes[3][4][3]);
void kou_followsequence(int select_cards[][15],int own_cards[8][15],int max,int used_cards[8][15],int tes[3][4][3]);
void kou_change(int out_cards[8][15],int my_cards[8][15],int num_of_change,int tes[3][4][3]);

void pat_make(int pattern[][7],int own_cards[8][15],int max,int used_cards[8][15],int joker_flag,int tes[3][4][3]);
void value_strong(int pattern[][7],int own_cards[8][15],int used_cards[8][15],int rev,int n,int tes[3][4][3]);

void cardprint(int own_cards[8][15]);
void Tableprint(int own_cards[8][15],int print);

void set(pattern *pat,int own_cards_ad[8][15]);
int conv2to10(int i);
int conv10to2(int i);
void printPattern(pattern *pat);
int make3to2(int suits, int j, int c, pattern *pat);
int make2to1(int suits, int j, int c, pattern *pat);
void Cselect(pattern *pat, struct state_type state);
int conv2tosuits(int *suits);
void convsuitsto2(int suit[5],int x);
void cardstoPat(int kakumei,int out_cards[5][15],pattern pat);

int NNselect(pattern *pat, double OUT[54],int kakumei,int count);
