/*myDefine*/ 
#ifndef __fumiya__myDefine__
#define __fumiya__myDefine__

// 色んな配列のサイズ
#define MAX_ARRAY_SIZE 128
// 必勝手探索で見るノード数の上限
#define MAX_SEARCH_NODE 1000000
// モンテカルロでのシミュレーション回数
//#define SIMULATION_COUNT 2193
#define SIMULATION_COUNT 4887

// モンテカルロ中の相手の手札生成時の確率を偏らせるか
//#define USE_ESTIMATE_HAND

// 必勝手探索で見るノードの数に制限をかけるか
#define USE_MAX_SEARCH_NODE

// 評価関数値の計算に64ビットマシン向けの高速化を使うか
//#define USE_BIT64

typedef unsigned long long int int64;

// 取り得る手の情報を保持する構造体
typedef struct{
	int64 hands;  // 使うカード集合
	unsigned char qty;   // カード枚数
	unsigned char seq;   // 階段かどうか
	unsigned char ord;   // 強さ
	unsigned char suit;  // マーク
} bitValidHand;

// 取り得る手を配列として保持する構造体
typedef struct{
	bitValidHand hands[MAX_ARRAY_SIZE];
	int size; // 取り得る手の数
} bitValidHandsArray;

// フィールドの情報を保持する構造体
typedef struct{
	int onset; // 場にカードが出ているか
	int qty;   // 場に出ているカードの枚数
	int suit;  // 場に出ているカードのマーク
	int ord;   // 場に出ているカードの強さ(役の中で一番弱いカードの強さ)
	int seq;   // 場に出ているカードが階段か
	int lock;  // しばりが発生しているか
	int rev;   // 革命が発生しているか
	int pass;  // 現在パスしているプレイヤーの集合
	int goal;  // 既に上がったプレイヤーの集合

	int mypos;   // 自プレイヤーの席
	int seat[5]; // seat[i] : プレイヤーiが座っている席
	int lest[5]; // lest[i] : 席iに座っているプレイヤーの残りカード枚数
	int rank[5]; // rank[i] : 席iに座っているプレイヤーの階級
} fieldInfo;

typedef struct{
	int64 chgCards;  // 交換に出したカード
	int64 notCards;  // 交換相手が持ち得ないカード
	int firstPlayer; // 最初に行動したプレイヤー(ダイヤの3を所持)
} changeInfo;

#endif
