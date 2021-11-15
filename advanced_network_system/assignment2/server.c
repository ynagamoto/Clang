// クライアントから "s" を受信したらスライムのaaかはぐれメタルのaaをランダムで送信するプログラム 

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/select.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/uio.h>
#include<netinet/in.h>
#include<sys/stat.h>
#include<time.h>

#define SNSIZ     256
#define CHARN     64
#define COMN      3
#define IMGSIZ  1024
#define IMGSIZLEN 5
#define MONSMAX   10

enum {
  NQUEUESIZE = 5,
  MAXNCLIENTS = 10, // 接続可能なクライアントの上限
};

// プロトタイプ宣言
void send_mes (FILE *fp, char *mes);
void send_all (char *mes);

void okey (FILE *fp);
void sorry (FILE *fp);
void delete_client (int client);
void close_conn (int client);

// グローバル変数
int  count_access = 0;
int  nclients = 0, npipes = 0;
FILE *clientfps[MAXNCLIENTS];
FILE *piperfps[MAXNCLIENTS];

char Bad_Request[] = "Bad Request.\n";
char Inp_Request[] = "Please enter s to display the slime. Enter x when close connection.\n";

int main (int argc, char *argv[]) {
  int listenfd, connfd, nbytes, client, port;
  struct sockaddr_in servaddr;
  char buf[256];
  
  // コマンドライン引数をチェック
  if (argc != 2) {
    printf("コマンドライン引数でポートが指定されていないため 8080 ポートでサーバを起動します\n");
    port = 8080;
  } else {
    port = atoi(argv[1]);
  }

  // socket
  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(1);
  }
  
  // メモリをきれいにする
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

  // bind
  if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind");
    exit(1);
  }

  // listen
  if (listen(listenfd, NQUEUESIZE) < 0) {
    perror("listen");
    exit(1);
  }
  printf("Waiting for connection from clients...\nEnter x when you want to close this server.\n");

  for (;;) {
    int i, maxfd;
    fd_set readfds;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(listenfd, &readfds);
    maxfd = listenfd >= STDIN_FILENO ? listenfd : STDIN_FILENO;
    
    for (int i = 0; i < nclients; i++) {
      // 初期化した readfds に追加
      FD_SET(clientfps[i]->_fileno, &readfds);
      // maxfd の更新
      if (clientfps[i]->_fileno > maxfd) maxfd = clientfps[i]->_fileno;
    }
    for (int i = 0; i < npipes; i++) {
      // 初期化した readfds に追加
      FD_SET(piperfps[i]->_fileno, &readfds);
      // maxfd の更新
      if (piperfps[i]->_fileno > maxfd) maxfd = piperfps[i]->_fileno;
    }

    if (select(maxfd+1, &readfds, NULL, NULL, NULL) < 0) {
      perror("select");
      exit(1);
    }

    // 'x'が入力された時サーバを閉じる
    if (FD_ISSET(STDIN_FILENO, &readfds)) {
      scanf("%256s", buf);
      if (strlen(buf) == 1 && buf[0] == 'x') {
        printf("\nClose server.\n");
        printf("This server has received accesses from %d people.\n", count_access);
        for (i = 0; i < nclients; i++) {
          delete_client(clientfps[i]->_fileno);
        }
        exit(0);
      }
    }

    // 新規接続
    if (FD_ISSET(listenfd, &readfds)) {
      int client;
      if ((client = accept(listenfd, (struct sockaddr *)NULL, NULL)) < 0) {
        perror("accept");
        continue;
      } else {
        printf("New connection!\n");
      } 

      FILE *fp;
      if ((fp = fdopen(client, "r+")) == NULL) {
        perror("fdopen");
        exit(1);
      }
      setlinebuf(fp);

      // 受信可能クライアント数の上限を達しているか確認
      if (nclients >= MAXNCLIENTS) {
        // 上限に達しているためシャットダウンしてclose
        char *mes = "Sorry, it's full.\nPlease try again later.\n";
        send_mes(fp, mes);
        fclose(fp);
        close_conn(fp->_fileno);
        printf("But refused a new connection.\n");
      } else {
        // まだ空きがあった
        printf("Accepted a connection on descriptor %d.\n", client);
        count_access++;
        clientfps[nclients] = fp;
        nclients++;
        char *mes = "Welcome!\nIf you enter x, the communication will be terminated.\n";
        send_mes(fp, mes);
      }
    }

    // クライアントの入力待ち
    for (int i = 0; i < nclients; i++) {
      FILE *clientfp = clientfps[i];
      if (FD_ISSET(clientfp->_fileno, &readfds)) {
        // 返信を受け取ってチェック
        char res[BUFSIZ-2];
        memset(&res, 0, sizeof(res));
        if (fgets(res, sizeof(res), clientfp) == NULL) {
          perror("fgets");
          exit(1);
        }
        if ((strlen(res) == 3) && (res[0] == 'x')) { // 接続を終了する
          delete_client(clientfp->_fileno);
          continue;
        } else { // fork して子プロセスに処理をさせる
          int ptoc[2], ctop[2];
          pipe(ptoc);
          pipe(ctop);

          pid_t pid; 
          if ((pid = fork()) == 0) {
            FILE *readFp, *writeFp;
            char req[BUFSIZ];

            // 必要ないパイプを閉じる
            close(ptoc[1]);
            close(ctop[0]);
            if ((readFp = fdopen(ptoc[0], "r")) == NULL) {
              perror("readFp fdopen");
              exit(1);
            }
            setlinebuf(readFp);
            if ((writeFp = fdopen(ctop[1], "w")) == NULL) {
              perror("writeFp fdopen");
              exit(1);
            }
            setlinebuf(writeFp);

            // 文字列を受け取る
            if (fgets(req, sizeof(req), readFp) == NULL) {
              perror("child fgets");
              exit(1);
            }
            printf("log: child fgets -> %s", req);
            fclose(readFp);

            // 標準出力をパイプ1に変更
            dup2(writeFp->_fileno, STDOUT_FILENO);
            fclose(writeFp);

            // exec low to high
            execlp("./low2high.sh", "./low2high.sh", req, NULL);
            perror("execlp");
            exit(-1);
          } else {
            FILE *readFp, *writeFp;
            char send[BUFSIZ];

            // 必要ないパイプを閉じる
            close(ptoc[0]);
            close(ctop[1]);
            if ((readFp = fdopen(ctop[0], "r")) == NULL) {
              perror("readFp fdopen");
              exit(1);
            }
            setlinebuf(readFp);
            if ((writeFp = fdopen(ptoc[1], "w")) == NULL) {
              perror("writeFp fdopen");
              exit(1);
            }
            setlinebuf(writeFp);

            snprintf(send, sizeof(send), "%s\n", res);
            if (fwrite(res, sizeof(unsigned char), strlen(res), writeFp) == -1) {
              perror("fwrite");
              exit(1);
            }
            fclose(writeFp);
            printf("log: snprintf -> %s", res);

            // パイプの readFp を追加
            piperfps[npipes] = readFp;
            npipes++;
          } 
        }
      }
    }

    // パイプの出力待ち
    for (int i = 0; i < npipes; i++) {
      FILE *piperfp = piperfps[i];
      if (FD_ISSET(piperfp->_fileno, &readfds)) {
        // パイプの出力を全クライアントに送信
        char res[BUFSIZ];
        memset(&res, 0, sizeof(res));
        if (fgets(res, BUFSIZ, piperfp) == NULL) {
          perror("fgets");
          exit(1);
        }
        for (int j = 0; j < npipes; j++) {
          if (piperfps[i]->_fileno == piperfp->_fileno) {
            piperfps[i] = piperfps[npipes-1];
            npipes--;
          }
        }
        fclose(piperfp);
        send_all(res);
      }
    }
  }
}

void send_mes (FILE *fp, char *mes) {
  char mes_ln[IMGSIZ+1];
  snprintf(mes_ln, sizeof(mes_ln), "%s", mes);
  if (fwrite(mes_ln, sizeof(unsigned char), strlen(mes_ln), fp) == -1) {
    perror("fwrite");
    exit(1);
  }
}

void send_all (char *mes) {
  printf("chat: %s", mes);
  for (int i = 0; i < nclients; i++) {
    FILE *clientfp = clientfps[i];
    send_mes(clientfp, mes); 
  }
}

void delete_client (int client) {
  for (int i = 0; i < nclients; i++) {
    if (clientfps[i]->_fileno == client) {
      shutdown(client, SHUT_RDWR); 
      fclose(clientfps[i]);
      clientfps[i] = clientfps[nclients - 1];
      nclients--;
      printf("Connection closed on descriptor %d.\n", client);
      break;
    }
  }
}

void close_conn (int client) {
  shutdown(client, SHUT_RDWR); 
  close(client);
}

