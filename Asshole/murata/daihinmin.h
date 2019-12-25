/*daihinmin*/ 

struct state_type
{
  int ord;        //順番
  int sequence;   //階段
  int qty;        //総数
  int rev;        //革命
  int b11;        //11バック
  int lock;       //縛り
  int onset;      //場が新しいかどうか
  int suit[5];    //すーと

  int player_qty[5];  //各プレイヤー手札の枚数
  int player_rank[5]; //各プレイヤーランク
  int seat[5];        //各プレイヤー座席順

  int joker;      //jo-ka-
}state;

struct seatsort_state//席順(自分を0とする)にプレイヤの状態を格納
{
  int seat[5];//各席のプレイヤ番号
  int score[5];//合計スコア
  int old_rank[5];//前回ゲームの階級
  int hand_qty[5];//手札枚数
  int pass[5];//パスであれば1,手札枚数0であれば3
}ss;

//場を流せると判断する値
#define STRONG 86

//状態を表示しない場合0
//接続が最初のときのみ表示する場合1
//表示する場合2
#define PRINT_SCORE 0
#define PRINT_PAT 0
#define PRINT_STATE 0
//defineの変更を適用するにはcファイルも一度変更する必要がある

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
//--------------------------------新しい関数----------------------------------------//
//----------------------------------------------------------------------------------//

//グローバル変数
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

//[0][0]にジョーカー提出とする、[2]は1枚組か2枚組か、カードの[4][14]の2次元にプレイヤごとの4次元目追加的なイメージ
//0は場が空の時出したことない、１：非革命状態で出した、２：革命状態で出した
//419行に記述
extern int client_log[2][4][14][5];
extern int client_log_st_pass_off_count;

//グローバル変数
extern int seat_tactics_flag;

//グローバル変数（席順戦術の有効性確認）
//[1000]：[0]に1ゲーム目の席順戦略実行回数、１ゲーム目結果の自分の階級を格納する
//[1000]：[1]に2ゲーム目の席順戦略実行回数、２ゲーム目結果の自分の階級を格納する、１ゲーム目結果の自分の前のクライアントの階級を格納
//[0]に席順戦術の実行回数、[1]に階級を格納(０：大富豪～４：大貧民)、[3]に前の席のクライアントの階級を格納
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
<関数関連図>
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