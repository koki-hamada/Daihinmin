#include "HashTable.h"

#include "def.h"
#include "logger.h"
#include <random>
#include <stdio.h>
#include <stdlib.h>

HashTable::HashTable()
	: size(0) {
	std::mt19937_64 mt(12345);
	for(int i = 0; i < 53; i++) {
		mycardHash[i] = mt();
	}
	for(int i = 0; i < 53; i++) {
		oppcardHash[i] = mt();
	}
	onsetHash = mt();
	for(int i = 0; i < 12; i++)
		qtyHash[i] = mt();

	for(int i = 0; i < 1 << 4; i++) {
		suitHash[i] = mt();
	}
	for(int i = 0; i < 15; i++) {
		ordHash[i] = mt();
	}
	seqHash = mt();
	lockHash = mt();
	revHash = mt();
	for(int i = 0; i < 1 << 5; i++) {
		passHash[i] = mt();
	}
	Init();
}

HashTable::~HashTable() {
}

void HashTable::Init() {
	for(int i = 0; i < listSize; i++) {
		list[i] = nullptr;
	}
	size = 0;
}

int64 HashTable::GetHash(int64 myCard, int64 oppCard, const fieldInfo* finfo) {
	int64 ret = 0;
	if(myCard) {
		for(int i = 0; i < 53; i++) {
			if(myCard >> i & 1) {
				ret ^= mycardHash[i];
			}
		}
	}
	if(oppCard) {
		for(int i = 0; i < 53; i++) {
			if(oppCard >> i & 1) {
				ret ^= oppcardHash[i];
			}
		}
	}
	if(finfo->onset) {
		ret ^= onsetHash;
	} else {
		ret ^= qtyHash[finfo->qty];
		ret ^= suitHash[finfo->suit];
		ret ^= ordHash[finfo->ord];
		if(finfo->seq) {
			ret ^= seqHash;
		}
		if(finfo->lock) {
			ret ^= lockHash;
		}
		if(finfo->rev) {
			ret ^= revHash;
		}
	}
	ret ^= passHash[finfo->pass];
	return ret;
}

// エントリーに登録
void HashTable::Add(int64 hash, float val) {
	// 最大サイズに達していたら何もしない
	if(size >= entrySize)
		return;

	int idx = hash % listSize;

	Node* n = list[idx];
	while(n != nullptr) {
		// 登録済み
		if(n->hash == hash)
			return;
		n = n->next;
	}
	entry[size].hash = hash;
	entry[size].value = val;
	entry[size].next = list[idx];
	list[idx] = &entry[size];
	size++;
}

// エントリーに登録されているか調べる
bool HashTable::Find(int64 hash, float& val) {
	int idx = hash % listSize;
	Node* n = list[idx];
	while(n != nullptr) {
		if(n->hash == hash) {
			val = n->value;
			return true;
		}
		n = n->next;
	}
	return false;
}
