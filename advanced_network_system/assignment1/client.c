#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define IMAGESIZ 1024

void send_mes (FILE *fp, char *mes);
void recive_mes (FILE *fp, char *mes);

int main(int argc, char *argv[]) {
  int  sockfd, nbytes, port;
  char *addr;
  char default_addr[] = "localhost";
  struct sockaddr_in servaddr;

  // 入力値の処理
  if (argc < 3) {
    printf("IPアドレスとポート番号が指定されていないため localhost:8080 で起動します．\n");
    port = 8080;
    addr = default_addr;
  } else {
    addr = argv[1];
    port = atoi(argv[2]);
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
  servaddr.sin_port = htons(port);  // 通信相手のport番号
  if (inet_pton(AF_INET, addr, &servaddr.sin_addr) < 0) {  // IPアドレスの設定，ローカルホスト
    perror("inet_pton");
    exit(1);
  }

  // 接続の要求
  if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("connect");
    exit(1);
  } 

  FILE *fp;
  if ((fp = fdopen(sockfd, "r+")) == NULL) {
    perror("fdopen");
    exit(1);
  }
  setlinebuf(fp);

  // 通信
  char res[BUFSIZ];
  char size[BUFSIZ];
  char aa[BUFSIZ];
  char req[256];
  recive_mes(fp, res); // welcom

  for (;;) {
    printf("Guide:");
    recive_mes(fp, res); // guide

    // 文字列の入力
    // 's' or 'x' が入力されるまでループ
    do {
      memset(req, 0, sizeof(req));
      printf("-> ");
      scanf("%256s", req);
      if (strlen(req) == 1 && (req[0] == 's' || req[0] == 'x')) break;
      printf("Bad Request ");
    } while (1);
    // 文字列の送信
    send_mes(fp, req);
    // 'x'が入力された時は通信を終了する
    if (req[0] == 'x') break;

    // サーバから文字列を受け取る
    printf("Name:");
    recive_mes(fp, res);  // name
    printf("Size:");
    recive_mes(fp, size); // size
    if (atoi(size) >= IMAGESIZ) {
      perror("fread");
      exit(1);
    }
    memset(aa, 0, sizeof(aa));
    fread(aa, sizeof(unsigned char), atoi(size)+1, fp); // aa
    printf("%s", aa);
    printf("Result:");
    recive_mes(fp, res); // result    
  }

  // ソケットを閉じて終了
  printf("通信を終了します\n");
  if (shutdown(sockfd, SHUT_RDWR) == -1) {
    perror("shutdown");
  } 
  close(sockfd);
  return 0;
}

void send_mes (FILE *fp, char *mes) {
  char ln = '\n';
  if (fwrite(mes, sizeof(unsigned char), strlen(mes), fp) == -1) {
    perror("fwrite");
    exit(1);
  }
  if (fwrite(&ln, sizeof(unsigned char), strlen(&ln), fp) == -1) {
    perror("fwrite");
    exit(1);
  }
}

void recive_mes (FILE *fp, char *mes) {
  memset(mes, 0, sizeof(mes));
  fgets(mes, BUFSIZ, fp);
  printf("%s", mes);
}
