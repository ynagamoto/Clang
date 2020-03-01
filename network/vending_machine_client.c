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

  // ソケットアドレス構造体を初期化
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

  // 通信
  for (;;) {
    int i, nmenu;
    
    memset(res, 0, sizeof(res));
    while ((nbytes = read(sockfd, res, sizeof(res))) > 0) {
      printf("%s", res);

      int break_flag = 0;
      for (i = 0; i < strlen(res); i++) {
        if (res[i] == '.') break_flag = 1;
      }
      if (break_flag == 1) break;
      else memset(res, 0, sizeof(res));
    } 
    
    // 文字列の入力
    scanf("%s", message);

    // 'x'が入力された時は通信を終了する
    if (strlen(message) == 1 && message[0] == 'x') break;

    // 文字列の送信
    write(sockfd, message, strlen(message)); 
  }

  // ソケットを閉じて終了
  printf("通信を終了します\n");
  if (shutdown(sockfd, SHUT_RDWR) == -1) {
    perror("shutdown");
  } 
  close(sockfd);
  return 0;
}
