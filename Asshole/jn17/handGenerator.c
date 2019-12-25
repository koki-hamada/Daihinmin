/* handGenerator.c : �\�ȍs���̗񋓂⑶�ݔ�����s�Ȃ� */ 
/* Author          : Fumiya Suto                        */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>

#include "bitCard.h"
#include "mydef.h"

int fewBitCount(int64 t){
	int res;
	for(res=0;t;t&=t-1) res++;
	return res;
}

// �ꂪ��̏ꍇ�ɂ�����1���ȏ�̃O���[�v����Ȃ��𐶐�
void getAllValidGroup(bitValidHandsArray *res, const bitValidHandsArray *vha, const fieldInfo *info, int64 myCards){
	int i, j;
	int num[15];
	memset(num, -1, sizeof(num));
	// solo�܂���group���\��������̂��R�s�[
	for(i=0;i<vha->size;i++){
		res->hands[res->size] = vha->hands[i];
		res->size++;
		num[vha->hands[i].ord] = i;
	}
	// �e�����̃O���[�v�ɑ΂��Cjoker�����������̂𐶐�����
	if(myCards&(1LL<<52)){
		for(i=1;i<=13;i++){
			if(num[i]==-1) continue;
			res->hands[res->size] = vha->hands[num[i]];
			res->hands[res->size].hands |= (1LL<<52);
			res->hands[res->size].qty++;
			// �}�[�N���g���ĂȂ��ŏ��̏ꏊ�Ƀr�b�g�𗧂Ă�
			res->hands[res->size].suit |= ((vha->hands[num[i]].suit+1)&(~vha->hands[num[i]].suit));
			res->size++;
		}
		// joker�̒P�̏o����ǉ�
		// �����͒ʏ펞14�C�v����0����
		pushValidHands(res, (1LL<<52), 1, 0, 14*(1-info->rev), 0);
	}
}

// �ꂪ��̏ꍇ�ɂ�����K�i����Ȃ��𐶐�
void getAllValidSequence(bitValidHandsArray *res, const bitValidHandsArray *vha, const fieldInfo *info){
	int i;
	for(i=0;i<vha->size;i++){
		// �ꂪ��̏ꍇ��joker��3-2�͈̔͊O�Ŏg���������
		//  - �����Ȍ��ʂ����肪�K�����ɑ��݂���
		if(1<=vha->hands[i].ord&&vha->hands[i].ord+vha->hands[i].qty-1<=13){
			res->hands[res->size] = vha->hands[i];
			res->size++;
		}
	}
}

void getAllFollowSolo(bitValidHandsArray *res, const bitValidHandsArray *vha, const fieldInfo *info, int64 myCards){
	int i;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	for(i=vha->size-1;i>=0;i--){
		if(vha->hands[i].ord <= ord) break;
		// ���蔭�����̃}�[�N����
		if(lock == 1 && suit != vha->hands[i].suit) continue; 
		// 1������̃J�[�h��苭���J�[�h�Ȃ�ǉ�
		if(vha->hands[i].qty == 1){
			res->hands[res->size] = vha->hands[i];
			res->size++;
		}
	}
	// �W���[�J�[�T��
	if(ord < 14 && ((myCards>>52)&1) == 1){
		pushValidHands(res, (1LL<<52), 1, 0, 14, 0);
	}
	// �X�y�[�h��3�T��
	if(ord==14 && (myCards&1)==1){
		pushValidHands(res, 1LL, 1, 0, 14, 0);
	}
}

void getAllFollowSoloRev(bitValidHandsArray *res, const bitValidHandsArray *vha, const fieldInfo *info, int64 myCards){
	int i;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	for(i=0;i<vha->size;i++){
		if(vha->hands[i].ord >= ord) break;
		// ���蔭�����̃}�[�N����
		if(lock == 1 && suit != vha->hands[i].suit) continue; 
		// 1������̃J�[�h��苭���J�[�h�Ȃ�ǉ�		
		if(vha->hands[i].qty == 1){
			res->hands[res->size] = vha->hands[i];
			res->size++;
		}
	}
	// �W���[�J�[�T��
	if(ord > 0 && ((myCards>>52)&1) == 1){
		pushValidHands(res, (1LL<<52), 1, 0, 0, 0);
	}
	// �X�y�[�h��3�T��
	if(ord==0 && (myCards&1)==1){
		pushValidHands(res, 1LL, 1, 0, 0, 0);
	}
}

void getAllFollowGroup(bitValidHandsArray *res, const bitValidHandsArray *vha, const fieldInfo *info, int64 myCards){
	int i, j;
	int tmp;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	int qty  = info->qty;
	int64 validCard;
	int num[15], bsuit[15];
	memset(num, 0, sizeof(num));
	memset(bsuit, 0, sizeof(bsuit));
	// �e�����̃J�[�h�ɂ��Ė����ƃ}�[�N�W�������߂�
	for(i=0;i<vha->size;i++){
		bsuit[vha->hands[i].ord] |= vha->hands[i].suit;
		num[vha->hands[i].ord] = vha->hands[i].qty;
	}

	// ��o�ł�����T��
	for(i=0;i<vha->size;i++){
		// ��̃J�[�h��苭���J�[�h�łȂ���Ώo���Ȃ�
		if(vha->hands[i].ord <= ord) continue;
		// ��̃J�[�h�Ɠ��������̎�
		if(vha->hands[i].qty == qty){
			// ���莞�̃}�[�N����
			if(lock == 1 && vha->hands[i].suit != suit) continue;
			res->hands[res->size] = vha->hands[i];
			res->size++;
		}
		// joker���������̌���
		if((myCards&(1LL<<52)) && vha->hands[i].qty == qty-1){
			// joker����������ǉ��ς݂Ȃ�continue;
			if(num[vha->hands[i].ord] == 0) continue;
			// joker���������ɔ���肪���݂����continue;
			if((bsuit[vha->hands[i].ord]&suit) == suit) continue;
			// vha->hands[i]�̃J�[�h�ŃJ�o�[����Ȃ��}�[�N�W��
			tmp = suit^(vha->hands[i].suit&suit);
			// joker�������Ĕ��鎖���ł���
			if(fewBitCount(tmp) == 1){
				res->hands[res->size] = vha->hands[i];
				res->hands[res->size].hands |= (1LL<<52);
				res->hands[res->size].qty++;
				res->hands[res->size].suit = suit;
				res->size++;
				num[vha->hands[i].ord] = 0;
			}
			// joker�������Ȃ��Ƌ���vha->hands[i].ord�̍��@�肪�����Ƃ�
			else if(num[vha->hands[i].ord] == qty-1) {
				// �����ԂȂ獇�@������Ȃ�
				if(lock == 1) continue;
				res->hands[res->size] = vha->hands[i];
				res->hands[res->size].hands |= (1LL<<52);
				res->hands[res->size].qty++;
				// �K���ȂƂ���Ƀ}�[�N��ݒ�
				for(j=0; ;j++)
					if(!(res->hands[res->size].suit&(1<<j))){
						res->hands[res->size].suit |= (1<<j);
						break;
					}
				res->size++;
				num[vha->hands[i].ord] = 0;
			}
		}
	}
}

void getAllFollowGroupRev(bitValidHandsArray *res, const bitValidHandsArray *vha, const fieldInfo *info, int64 myCards){
	int i, j;
	int tmp;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	int qty  = info->qty;
	int64 validCard;
	int num[15], bsuit[15];
	memset(num, 0, sizeof(num));
	memset(bsuit, 0, sizeof(bsuit));
	// �e�����̃J�[�h�ɂ��Ė����ƃ}�[�N�W�������߂�
	for(i=0;i<vha->size;i++){
		bsuit[vha->hands[i].ord] |= vha->hands[i].suit;
		num[vha->hands[i].ord] = vha->hands[i].qty;
	}

	// ��o�ł�����T��
	for(i=0;i<vha->size;i++){
		// ��̃J�[�h���ア�J�[�h�łȂ���Ώo���Ȃ�
		//  - (To do)�ʏ펞�p�̊֐��Ƃ��̍s�������Ȃ��̂Ō�œ�������
		if(vha->hands[i].ord >= ord) continue;
		// ��̃J�[�h�Ɠ��������̎�
		if(vha->hands[i].qty == qty){
			// ���莞�̃}�[�N����
			if(lock == 1 && vha->hands[i].suit != suit) continue;
			res->hands[res->size] = vha->hands[i];
			res->size++;
		}
		// joker���������̌���
		if((myCards&(1LL<<52)) && vha->hands[i].qty == qty-1){
			// joker����������ǉ��ς݂Ȃ�continue;
			if(num[vha->hands[i].ord] == 0) continue;
			// joker���������ɔ���肪���݂����continue;
			if((bsuit[vha->hands[i].ord]&suit) == suit) continue;
			// vha->hands[i]�̃J�[�h�ŃJ�o�[����Ȃ��}�[�N�W��
			tmp = suit^(vha->hands[i].suit&suit);
			// joker�������Ĕ��鎖���ł���
			if(fewBitCount(tmp) == 1){
				res->hands[res->size] = vha->hands[i];
				res->hands[res->size].hands |= (1LL<<52);
				res->hands[res->size].qty++;
				res->hands[res->size].suit = suit;
				res->size++;
				num[vha->hands[i].ord] = 0;
			}
			// joker�������Ȃ��Ƌ���vha->hands[i].ord�̍��@�肪�����Ƃ�
			else if(num[vha->hands[i].ord] == qty-1) {
				// �����ԂȂ獇�@������Ȃ�
				if(lock == 1) continue;
				res->hands[res->size] = vha->hands[i];
				res->hands[res->size].hands |= (1LL<<52);
				res->hands[res->size].qty++;
				// �K���ȂƂ���Ƀ}�[�N��ݒ�
				for(j=0; ;j++)
					if(!(res->hands[res->size].suit&(1<<j))){
						res->hands[res->size].suit |= (1<<j);
						break;
					}
				res->size++;
				num[vha->hands[i].ord] = 0;
			}
		}
	}
}

void getAllFollowSequence(bitValidHandsArray *res, const bitValidHandsArray *vha, const fieldInfo *info, int64 myCards){
	int i;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	int qty  = info->qty;
	int64 mask = 0x0000008004002001ULL;
	for(i=0;i<vha->size;i++){
		// ���蔭�����̓}�[�N���������
		if(lock==1 && vha->hands[i].suit != suit) continue;
		// �����`�F�b�N
		if(vha->hands[i].qty != qty) continue;
		// �����`�F�b�N ord+qty�ȏ�ł���K�v������
		if(vha->hands[i].ord < ord+qty) continue;
		// �擪�̃J�[�h��joker�Ȃ�g��Ȃ�
		if(!((mask << (vha->hands[i].ord-1))&myCards)) continue;
		// �J�[�h��ǉ�
		res->hands[res->size] = vha->hands[i];
		res->size++;
	}
}

void getAllFollowSequenceRev(bitValidHandsArray *res, const bitValidHandsArray *vha, const fieldInfo *info, int64 myCards){
	int i;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	int qty  = info->qty;
	int64 mask = 0x0000008004002001ULL;
	for(i=0;i<vha->size;i++){
		// ���蔭�����̓}�[�N���������
		if(lock==1 && vha->hands[i].suit != suit) continue;
		// �����`�F�b�N
		if(vha->hands[i].qty != qty) continue;
		// �����`�F�b�N ord-qty�ȉ��ł���K�v������
		//  - �ʏ펞�p�Ƃ����������Ȃ��̂�(ry
		if(vha->hands[i].ord > ord-qty) continue;
		// �Ō�̃J�[�h��joker�Ȃ�g��Ȃ�
		if(!((mask << (vha->hands[i].ord+qty-2))&myCards)) continue;
		// �J�[�h��ǉ�
		res->hands[res->size] = vha->hands[i];
		res->size++;
	}
}

void getAllValidHands(bitValidHandsArray *res, const bitValidHandsArray *group, const bitValidHandsArray *seq, const fieldInfo *info, int64 myCards){
	int i;
	res->size = 0;
	if(info->onset == 1){
		getAllValidGroup(res, group, info, myCards);
		getAllValidSequence(res, seq, info);
	} else {
		if(info->rev == 0){
			if(info->qty == 1){
				getAllFollowSolo(res, group, info, myCards);
			}
			else {
				if(info->seq == 0){
					getAllFollowGroup(res, group, info, myCards);
				} else {
					getAllFollowSequence(res, seq, info, myCards);
				}
			}
		} else {
			if(info->qty == 1){
				getAllFollowSoloRev(res, group, info, myCards);
			}
			else {
				if(info->seq == 0){
					getAllFollowGroupRev(res, group, info, myCards);
				} else {
					getAllFollowSequenceRev(res, seq, info, myCards);
				}
			}
		}
	}
}

// ��o�J�[�h�̏�񂩂�C���@��̏W�����X�V����
void removeHands(bitValidHandsArray *group, bitValidHandsArray *seq, int64 submit, int64 myCards){
	int i;
	int nsize = 0;
	int64 mv;
	// ��D�W�������o�J�[�h�������C�W���[�J�[���c�邩���ׂ�
	int joker = (int)((myCards^(myCards&submit)) >> 52);
	for(i=0;i<group->size;i++){
		// i�Ԗڂ̎�ɒ�o�J�[�h�̉e��������
		if((group->hands[i].hands&submit) == 0LL){
			if(nsize != i)
				group->hands[nsize] = group->hands[i];
			nsize++;
		}
	}

	// group�̃T�C�Y���X�V
	group->size = nsize;

	nsize = 0;

	for(i=0;i<seq->size;i++){
		// i�Ԗڂ̎�ɒ�o�J�[�h�̉e��������
		if((seq->hands[i].hands&submit) == 0LL){
			if(nsize != i)
				seq->hands[nsize] = seq->hands[i];
			nsize++;
		}
		// �e��������Ƃ��c�K�i�Ȃ�joker���g���Ĉێ��ł��邩�𒲂ׂ�
		else {
			if(joker == 1){
				// ����joker���g���Ă����Ȃ�ێ��ł��Ȃ�
				if(seq->hands[i].hands&(1LL<<52)) continue;
				// ��o�J�[�h�ŏ������J�[�h��1�������Ȃ�joker�Œu�������Ĉێ�����
				mv = seq->hands[i].hands&submit;
				if(fewBitCount(mv)==1){
					seq->hands[i].hands ^= mv;
					seq->hands[i].hands |= (1LL<<52);
					seq->hands[nsize] = seq->hands[i];
					nsize++;
				}
			}
		}
	}
	// seq�̃T�C�Y���X�V
	seq->size = nsize;
}

// ��D����1���ȏ�̑g�𐶐�����
void generateGroup(bitValidHandsArray *vha, int64 myCards){
	int i;
	int maskA, maskB, maskC, maskD;
	int64 cardA, cardB, cardC, cardD;
	int cnt;
	int bitPos[4];
	// solo�����group�̐���
	for(i=1;i<=13;i++){
		cnt = 0;
		if((myCards >> (13*0+i-1))&1) bitPos[cnt++] = 0;
		if((myCards >> (13*1+i-1))&1) bitPos[cnt++] = 1;
		if((myCards >> (13*2+i-1))&1) bitPos[cnt++] = 2;
		if((myCards >> (13*3+i-1))&1) bitPos[cnt++] = 3;

		if(cnt > 0){
			maskA = (1 << bitPos[0]);
			cardA = (1LL << (13*bitPos[0]+i-1));
			pushValidHands(vha, cardA, 1, 0, i, maskA);
			if(cnt > 1){
				maskB = (1 << bitPos[1]);
				cardB = (1LL << (13*bitPos[1]+i-1));
				pushValidHands(vha,       cardB, 1, 0, i,       maskB);
				pushValidHands(vha, cardA|cardB, 2, 0, i, maskA|maskB);
				if(cnt > 2){
					maskC = (1 << bitPos[2]);
					cardC = (1LL << (13*bitPos[2]+i-1));
					pushValidHands(vha,             cardC, 1, 0, i,             maskC);
					pushValidHands(vha, cardA      |cardC, 2, 0, i, maskA      |maskC);
					pushValidHands(vha,       cardB|cardC, 2, 0, i,       maskB|maskC);
					pushValidHands(vha, cardA|cardB|cardC, 3, 0, i, maskA|maskB|maskC);
					if(cnt > 3){
						maskD = (1 << bitPos[3]);
						cardD = (1LL << (13*bitPos[3]+i-1));
						pushValidHands(vha,                   cardD, 1, 0, i,                   maskD);
						pushValidHands(vha, cardA            |cardD, 2, 0, i, maskA            |maskD);
						pushValidHands(vha,       cardB      |cardD, 2, 0, i,       maskB      |maskD);
						pushValidHands(vha, cardA|cardB      |cardD, 3, 0, i, maskA|maskB      |maskD);
						pushValidHands(vha,             cardC|cardD, 2, 0, i,             maskC|maskD);
						pushValidHands(vha, cardA      |cardC|cardD, 3, 0, i, maskA      |maskC|maskD);
						pushValidHands(vha,       cardB|cardC|cardD, 3, 0, i,       maskB|maskC|maskD);
						pushValidHands(vha, cardA|cardB|cardC|cardD, 4, 0, i, maskA|maskB|maskC|maskD);
					}
				}
			}
		}
	}
}

// ��D����joker���܂�3���ȏ�̊K�i�𐶐�����
void generateSequence(bitValidHandsArray *vha, int64 myCards){
	int i, j, k;
	int joker = ((myCards >> 52)&1);
	int64 validCard;
	int count;
	for(i=0;i<4;i++){
		int hand = 2*(((int)(myCards >> (13*i)))&((1<<13)-1));
		// hand�Ɋ܂܂��r�b�g����1�ȉ��Ȃ�
		if((hand&(hand-1)) == 0) continue;
		for(j=0;j+3-1<15;j++){
			int mask = (hand&(7<<j));
			if((mask&(mask-1))==0) continue;
			count = 0;
			validCard = 0LL;
			for(k=0;j+k<15;k++){
				if(((hand >> (j+k))&1)==1){
					validCard |= (1LL << (13*i+j+k-1));
				} else {
					validCard |= (1LL << 52);
					count++;
				}
				if(count > joker) break;
				else if(k>=2) pushValidHands(vha, validCard, k+1, 1, j, (1<<i));
			}
		}
	}
}

// myCards���̍��@���S�Đ�������
// - joker���܂ގ��joker�P�̏o����sequence�݂̂𐶐�
void generateAllHands(bitValidHandsArray *group, bitValidHandsArray *seq, int64 myCards){
	// ������
	group->size = 0;
	seq->size   = 0;
	// solo�����group�̐���
	generateGroup(group, myCards);
	// sequence�̐���
	generateSequence(seq, myCards);
}

int checkFollowValidSolo(const fieldInfo *info, int64 myCards){
	int i, j;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	for(i=0;i<4;i++){
		if(lock == 1 && ((suit>>i)&1)==0) continue; 
		for(j=ord+1;j<14;j++){
			if(((myCards >> (13*i+j-1))&1)==1){
				return 1;
			}
		}
	}
	// �W���[�J�[�T��
	if(ord < 14 && ((myCards>>52)&1) == 1){
		return 1;
	}
	// �X�y�[�h��3�T��
	if(ord==14 && (myCards&1)==1){
		return 1;
	}
	return 0;
}

int checkFollowValidGroup(const fieldInfo *info, int64 myCards){
	int i, j;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	int qty  = info->qty;
	int bitNum[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
	int joker = (int)((myCards >> 52)&1);
	for(i=ord+1;i<14;i++){
		int mask = 0;
		for(j=0;j<4;j++) mask |= (((myCards >> (13*j+i-1))&1) << j);
		// �W���[�J�[�����Ă���������Ȃ�
		if(bitNum[mask] + joker < qty) continue;
		// suit�p�^�[���Ń��[�v
		for(j=3;j<(1<<4);j++){
			// ���΂蔭����
			if(lock == 1 && j != suit) continue;
			// �r�b�g�p�^�[���������Ɉ�v���Ȃ�
			if(bitNum[j] != qty) continue;
			// mask ����o�r�b�g�p�^�[������
			if((j&mask) == j){
				return 1;
			} 
			else if (joker == 1 && bitNum[j^(j&mask)] == 1){
				if(bitNum[suit^(mask&suit)] == 1&&j!=suit) continue;
				return 1;
			}
		}
	}
	return 0;
}

int checkFollowValidSequence(const fieldInfo *info, int64 myCards){ 
	int i, j, k;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	int qty  = info->qty;
	int joker = (int)((myCards >> 52)&1);
	int count;
	// �}�[�N�Ɋւ��郋�[�v
	for(i=0;i<4;i++){
		// ���΂蔭����
		if(lock==1 && ((suit>>i)&1)==0) continue;
		// ����j����qty���̃V�[�P���X����������
		for(j=ord+qty;j+qty-1<15;j++){
			count = (int)((myCards >> (13*i+j-1))&1);
			// Joker��擪�ɂ���g�ݍ��킹������
			if(count == 0) continue;
			for(k=1;k<qty;k++){
				if(j+k-1 >= 13) continue;
				count += (int)((myCards >> (13*i+j+k-1))&1);
			}
			// �J�[�h��qty������ł���
			if(count+joker >= qty) return 1;
		}
	}
	return 0;
}

int checkFollowValidSoloRev(const fieldInfo *info, int64 myCards){
	int i, j;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	for(i=0;i<4;i++){
		if(lock == 1 && ((suit>>i)&1)==0) continue; 
		for(j=ord-1;j>0;j--){
			if(((myCards >> (13*i+j-1))&1)==1){
				return 1;
			}
		}
	}
	// �W���[�J�[�T��
	if(ord > 0 && ((myCards>>52)&1) == 1) return 1;
	// �X�y�[�h��3�T��
	if(ord==0 && (myCards&1)==1) return 1;
	return 0;
}

int checkFollowValidGroupRev(const fieldInfo *info, int64 myCards){
	int i, j;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	int qty  = info->qty;
	int bitNum[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
	int joker = (int)((myCards >> 52)&1);
	for(i=ord-1;i>0;i--){
		int mask = 0;
		for(j=0;j<4;j++) mask |= (((myCards >> (13*j+i-1))&1) << j);
		// �W���[�J�[�����Ă���������Ȃ�
		if(bitNum[mask] + joker < qty) continue;
		for(j=3;j<(1<<4);j++){
			// ���΂蔭����
			if(lock == 1 && j != suit) continue;
			// �r�b�g�p�^�[���������Ɉ�v���Ȃ�
			if(bitNum[j] != qty) continue;
			// mask ����o�r�b�g�p�^�[������
			if((j&mask) == j){
				return 1;
			} 
			else if (joker == 1 && bitNum[j^(j&mask)] == 1){
				if(bitNum[suit^(mask&suit)] == 1&&j!=suit) continue;
				return 1;
			}
		}
	}
	return 0;
}

int checkFollowValidSequenceRev(const fieldInfo *info, int64 myCards){
	int i, j, k;
	int suit = info->suit;
	int lock = info->lock;
	int ord  = info->ord;
	int qty  = info->qty;
	int joker = (int)((myCards >> 52)&1);
	int count;
	for(i=0;i<4;i++){
		if(lock==1 && ((suit>>i)&1)==0) continue;
		for(j=ord-qty;j>=0;j--){
			count = (int)((myCards >> (13*i+j+qty-1))&1);
			// 3-4-5�̃V�[�P���X��Joker��5����ɂ���ꍇ�ȊO�́C
			// Joker���Ō�ɂ���g�ݍ��킹������
			if(count == 0) continue;
			for(k=0;k<qty-1;k++){
				count += (int)((myCards >> (13*i+j+k-1))&1);
			}
			// �J�[�h��qty������ł��邩�Cjoker�𑫂���qty�����ׂ���
			if(count+joker >= qty) return 1;
		}
	}
	return 0;
}

// �^����ꂽ�󋵂�myCards����p�X�ȊO�ɂƂ��s�������邩�𔻒肷��
int checkAllValidHands(const fieldInfo *info, int64 myCards){
	if(info->onset == 1){
		return (myCards != 0LL); // �ꂪ��ŃJ�[�h���������Ă�Ή�������o����
	} else {
		if(info->rev == 0){
			if(info->qty == 1){
				return checkFollowValidSolo(info, myCards);
			}
			else {
				if(info->seq == 0){
					return checkFollowValidGroup(info, myCards);
				} else {
					return checkFollowValidSequence(info, myCards);
				}
			}
		} else {
			if(info->qty == 1){
				return checkFollowValidSoloRev(info, myCards);
			}
			else {
				if(info->seq == 0){
					return checkFollowValidGroupRev(info, myCards);
				} else {
					return checkFollowValidSequenceRev(info, myCards);
				}
			}
		}
	}
	return -1; // �����ɂ͗��Ȃ�
}
