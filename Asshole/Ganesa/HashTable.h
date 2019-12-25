#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#include "snowl/bitCard.h"
#include "snowl/mydef.h"

class HashTable {
	int64 mycardHash[53];
	int64 oppcardHash[53];
	int64 onsetHash;
	int64 qtyHash[12];
	int64 suitHash[1 << 4];
	int64 ordHash[15];
	int64 seqHash;
	int64 lockHash;
	int64 revHash;
	int64 passHash[1 << 5];

	struct Node {
		int64 hash;
		double value;
		Node* next;
	};

	constexpr static int listSize = 1 << 20;
	constexpr static int entrySize = 300000;
	Node entry[entrySize];
	Node* list[listSize];
	int size;

  public:
	HashTable();
	~HashTable();

	int Size() {
		return size;
	}
	void Init();
	int64 GetHash(int64 myCard, int64 oppCard, const fieldInfo* finfo);
	void Add(int64 hash, float val);
	bool Find(int64 hash, float& val);
};

#endif
