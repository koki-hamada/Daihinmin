/*handGenerator*/ 
#ifndef __fumiya__handGenerator__
#define __fumiya__handGenerator__

#include "mydef.h"

// vha���g���č��@����v�Z���Cres�Ɍ��ʂ��i�[����
void getAllValidHands(bitValidHandsArray *res, const bitValidHandsArray*, const bitValidHandsArray*, const fieldInfo *info, int64 myCards);

// ��o�J�[�h�̏�񂩂�C���@����̏W�����X�V����
void removeHands(bitValidHandsArray*, bitValidHandsArray*, int64 submit, int64 myCards);

// myCards���̍��@���S�Đ�������
// - joker���܂ގ��joker�P�̏o����sequence�݂̂𐶐�
void generateAllHands(bitValidHandsArray*, bitValidHandsArray*, int64 myCards);

void generateGroup(bitValidHandsArray *vha, int64 myCards);

int checkAllValidHands(const fieldInfo *info, int64 myCards);

#endif
