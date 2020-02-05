#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int main(int argc, char *argv[]) {
  int sockfd, nbytes;
  struct sockaddr_in servaddr;
  char message[256], res[BUFSIZ];

  // 入力値の処理
  if (argc < 3) {
    printf("IPアドレスとポート番号を引数で指定してください\n");
    exit(1);
  }

  // socket の作成
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }

  // 念のためソケットアドレス構造体を初期化
  memset(&servaddr, 0, sizeof(servaddr));
  // 通信相手の設定
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(atoi(argv[2]));  // 通信相手のport番号
  if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) < 0) {  // IPアドレスの設定，ローカルホスト
    perror("inet_pton");
    exit(1);
  }

  // 接続の要求
  if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("connect");
    exit(1);
  } 

  // 文字列の入力
  printf("送信したい文字列を入力して下さい（256文字以内）:");
  scanf("%s", message);

  // 文字列の送信
  write(sockfd, message, strlen(message));

  // 文字列を受け取って表示
  while ((nbytes = read(sockfd, res, sizeof(res))) > 0) {
    printf("\nres message = %s\n", res);
  }

  // ソケットを閉じて終了
  if (shutdown(sockfd, SHUT_RDWR) == -1) {
    perror("shutdown");
  } 
  close(sockfd);
  return 0;
}
