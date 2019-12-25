#g++ \
g++-9 \
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
-std=c++11 -Ofast -DNDEBUG -lpthread -o client
# -Iblas -lopenblas -std=c++11 -Ofast -DNDEBUG -DUSE_CBLAS -lpthread -o client
# -lsocket -lnsl -Iblas -lopenblas -std=c++11 -Ofast -DNDEBUG -DUSE_CBLAS -lpthread -o client
