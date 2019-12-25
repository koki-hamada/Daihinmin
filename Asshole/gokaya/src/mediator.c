#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>

#include <pthread.h>
#include "connection.h"
#include "include.h"
#include "server/tn_protocol.hpp"

int main(int argc, char* argv[]) {

  int recv_table[8][15];
  int send_table[8][15];
  int i,j;
  for (i=0;i<8;i++){
    for (j=0;j<15;j++){
      recv_table[i][j] = send_table[i][j] = 0;
    }
  }

  int sockfd, client_sockfd[N_PLAYERS]={0};
  int players_card[5][8][15]={0}; // players_card
  int work_card[8][15]={0};       // i.e. submitted card etc...
  //int port_number=42485;
  int port_number=42400;
  int protocol_version=20070;
  struct sockaddr_in wait_addr; // waiting port
  struct sockaddr_in client_addr[N_PLAYERS]; // port for each clients
  socklen_t client_len[N_PLAYERS]; // waiting port
  fd_set target_fds;
  fd_set org_target_fds;
  struct timeval waitval;

  // entry to server
  int playerid = entryToGame();
  #define N_PLAYERS 1
  if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    perror("client: socket");
    exit(1);
  }
  memset((char *) &wait_addr, 0, sizeof(wait_addr));
  wait_addr.sin_family = PF_INET;
  wait_addr.sin_addr.s_addr = htons(INADDR_ANY);
  wait_addr.sin_port = htons(port_number);
  i = 1;
  j = sizeof(i);
  if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&i, j) < 0){
    perror("setsockopt");
  }
  if(::bind(sockfd, (struct sockaddr *)&wait_addr, sizeof(wait_addr)) < 0){
    perror("reader: bind");
    exit(1);
  }
  if(listen(sockfd, 1) < 0) {
    perror("reader: listen");
    close(sockfd);
    exit(1);
  }

  // waiting clients
  for(i=0;i<N_PLAYERS;i++){
    printf("now waiting %i \n", i);
    client_len[i]=sizeof(client_addr[i]);
    if((client_sockfd[i]=accept(sockfd,(struct sockaddr *)&client_addr[i],&client_len[i])) < 0 ){
      perror("reader: accept");
      exit(1);
    };
    FD_ZERO(&org_target_fds);
    FD_SET(client_sockfd[i], &org_target_fds);
    memcpy(&target_fds, &org_target_fds, sizeof(org_target_fds));
    waitval.tv_sec  = 2;
    waitval.tv_usec = 500;
    int res = select(50,&target_fds,NULL,NULL,&waitval);
    printf("result of select() : %d\n",res);
    if (res == 1) {
      tn_card_read(client_sockfd[i], work_card , protocol_version);
      printf("version:%d\n",protocol_version);
    }
    tn_int_write(client_sockfd[i], i , protocol_version);
       printf("accepted from %s \n",inet_ntoa(client_addr[i].sin_addr));
  }

  // client
  while (1) {
    startGame(recv_table);
  }

  if(closeSocket() != 0){
    printf("failed to close socket\n");
    exit(1);
  } // ソケットを閉じて終了

  while (1) {
    players_card[0][5][0] = 1;
    players_card[0][5][0] = 2;
    for(int p = 0; p < N_PLAYERS; ++p){
      tn_card_write(client_sockfd[0],players_card[0],protocol_version); // tuuchi
    }
  }

  return 0;
}
