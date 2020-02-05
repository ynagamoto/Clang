#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>

int main(int argc, char *argv[]) {
  int listenfd, connfd, nbytes, use_port;
  char *res = "success!\n";
  struct sockaddr_in servaddr;

  // port番号の受け取り
  if (argc < 2) {
    printf("ポート番号をコマンドラインで入力して下さい\n");
    exit(1);
  }
  use_port = atoi(argv[1]);

  // ソケットの作成
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }

  // メモリをきれいにする
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(use_port);
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  // ソケットに名前をつける
  if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind");
    exit(1);
  }

  // 受付開始
  if (listen(listenfd, 5) < 0) {
    perror("listen");
    exit(1);
  }

  // 起動完了のメッセージを表示する
  printf("起動完了\n-----------------------------------\n");

  for (;;) {
    if ((connfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) < 0) {
      perror("accept");
      exit(1);
    }
    char buf[BUFSIZ];
    // メモリをきれいにする
    memset(buf, 0, sizeof(buf));
       
    // 文字列を受け取って表示
    while ((nbytes = read(connfd, buf, sizeof(buf))) > 0) {
      printf("\nmessage = %s\n", buf);
      if (buf[strlen(buf)] == '\0') {
        break;
      }
    }

    // 受信完了のメッセージを送信
    write(connfd, res, strlen(res));

    if (shutdown(connfd, SHUT_RDWR) == -1) {
      perror("shutdown");
    }
    // ソケットを閉じる
    close(connfd);
  }
}
