/*myDefine*/ 
#ifndef __fumiya__myDefine__
#define __fumiya__myDefine__

// �F��Ȕz��̃T�C�Y
#define MAX_ARRAY_SIZE 128
// �K����T���Ō���m�[�h���̏��
#define MAX_SEARCH_NODE 1000000
// �����e�J�����ł̃V�~�����[�V������
//#define SIMULATION_COUNT 2193
#define SIMULATION_COUNT 4887

// �����e�J�������̑���̎�D�������̊m����΂点�邩
//#define USE_ESTIMATE_HAND

// �K����T���Ō���m�[�h�̐��ɐ����������邩
#define USE_MAX_SEARCH_NODE

// �]���֐��l�̌v�Z��64�r�b�g�}�V�������̍��������g����
//#define USE_BIT64

typedef unsigned long long int int64;

// ��蓾���̏���ێ�����\����
typedef struct{
	int64 hands;  // �g���J�[�h�W��
	unsigned char qty;   // �J�[�h����
	unsigned char seq;   // �K�i���ǂ���
	unsigned char ord;   // ����
	unsigned char suit;  // �}�[�N
} bitValidHand;

// ��蓾����z��Ƃ��ĕێ�����\����
typedef struct{
	bitValidHand hands[MAX_ARRAY_SIZE];
	int size; // ��蓾���̐�
} bitValidHandsArray;

// �t�B�[���h�̏���ێ�����\����
typedef struct{
	int onset; // ��ɃJ�[�h���o�Ă��邩
	int qty;   // ��ɏo�Ă���J�[�h�̖���
	int suit;  // ��ɏo�Ă���J�[�h�̃}�[�N
	int ord;   // ��ɏo�Ă���J�[�h�̋���(���̒��ň�Ԏア�J�[�h�̋���)
	int seq;   // ��ɏo�Ă���J�[�h���K�i��
	int lock;  // ���΂肪�������Ă��邩
	int rev;   // �v�����������Ă��邩
	int pass;  // ���݃p�X���Ă���v���C���[�̏W��
	int goal;  // ���ɏオ�����v���C���[�̏W��

	int mypos;   // ���v���C���[�̐�
	int seat[5]; // seat[i] : �v���C���[i�������Ă����
	int lest[5]; // lest[i] : ��i�ɍ����Ă���v���C���[�̎c��J�[�h����
	int rank[5]; // rank[i] : ��i�ɍ����Ă���v���C���[�̊K��
} fieldInfo;

typedef struct{
	int64 chgCards;  // �����ɏo�����J�[�h
	int64 notCards;  // �������肪�������Ȃ��J�[�h
	int firstPlayer; // �ŏ��ɍs�������v���C���[(�_�C����3������)
} changeInfo;

#endif
