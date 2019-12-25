/*記号定数の定義*/
#define INPUTNO 100  //入力層のセル数
#define HIDDENNO 150  //中間層のセル数
#define OUTPUTNO 53  //出力層のセル数

/*関数*/
void NN(double IN[INPUTNO],double OUT[OUTPUTNO],double wh[HIDDENNO][INPUTNO+1],double wo[OUTPUTNO][HIDDENNO+1]);//NN入出力
double forward(double wh[HIDDENNO][INPUTNO+1],double [HIDDENNO+1],double hi[],double e[100]);//順方向の計算
void printweight(double wh[HIDDENNO][INPUTNO+1],double wo[OUTPUTNO][HIDDENNO+1]);//結果の出力
double s(double u);//シグモイド関数
