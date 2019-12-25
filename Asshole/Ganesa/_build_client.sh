gfortran -O3 -c blas/sgemm.f -o blas/sgemm.o
gfortran -O3 -c blas/xerbla.f -o blas/xerbla.o
gfortran -O3 -c blas/lsame.f -o blas/lsame.o
gcc -O3 -DADD_ -c blas/cblas_sgemm.c -o blas/cblas_sgemm.o
gcc -O3 -DADD_ -c blas/cblas_globals.c -o blas/cblas_globals.o
gcc -O3 -DADD_ -c blas/xerbla.c -o blas/xerbla_c.o
gcc -O3 -DADD_ -c blas/cblas_xerbla.c -o blas/cblas_xerbla.o
g++ \
client.cpp \
snowl/connection.cpp \
snowl/handGenerator.cpp \
snowl/bitCard.cpp \
snowl/checkInfo.cpp \
snowl/cardChange.cpp \
snowl/cardSelect.cpp \
snowl/mt19937ar.cpp \
Player.cpp \
logger.cpp \
HashTable.cpp \
Common.cpp \
blas/cblas_globals.o \
blas/xerbla_c.o \
blas/lsame.o \
blas/cblas_xerbla.o \
blas/sgemm.o \
blas/cblas_sgemm.o \
-std=c++11 -Ofast -Iblas -DNDEBUG -DUSE_CBLAS -lgfortran -o client
