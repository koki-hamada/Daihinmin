#ifndef PLAYER_H_
#define PLAYER_H_

#include "HashTable.h"
//#include "LR_Predicter.h"
#include "NeuralNet/Net.hpp"
#include "def.h"
#include "snowl/mydef.h"
#include <strings.h>
#include <vector>
class Player {
  public:
	Player();
	~Player();

	constexpr static float INF = 1e8;

	int clientNum;		// 1つのプログラムで動かすクライアント数(標準では1, 最大5)
	int channel;		// 入力画像のチャネル数
	int height;			// 入力画像の高さ
	int width;			// 入力画像の幅
	int inputSize;		// channel * height * width
	History history[5]; // 履歴
	SearchData sd[5];   // 探索データ
	int extra = -1;

	virtual void Init(std::string config);
	void Select(int sendCard[8][15], const State* state);
	virtual float Evaluate(const State* state, const History* history);

	int ClientNum() {
		return clientNum;
	}

  protected:
	Net* net;
	std::vector<Net*> nets;

	float randomEps; // ランダム行動をする確率(標準では0)

	HashTable hashtable; // 探索で利用するハッシュテーブル
	//LR_Predicter predicter;

	virtual void SwitchNet(int id);
	void AddInput(float* input, const State* state);
	void AddInput(float* input, const State* state, const History* history, int size);
	void AddCardImage(float* input, int64 card, int& idx);
	void AddStateImage(float* input, const State* state, int& idx);
	void AddFieldImage(float* input, const fieldInfo* finfo, const int baCards[8][15], int& idx);
	void AddPlayerInfoImage(float* input, const State* state, int& idx);
	void AddPlayerSeatImage(float* input, const State* state, int& idx);
	float SearchHand(const int depth, const State* state, const float visitProb, const bool continuous);
};

#endif /* PLAYER_H_ */
