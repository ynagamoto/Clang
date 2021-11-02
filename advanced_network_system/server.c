// クライアントから "s" を受信したらスライムのaaかはぐれメタルのaaをランダムで送信するプログラム 

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/select.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/uio.h>
#include<unistd.h>
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

struct monster {
  char name[CHARN];
  int  hp;
  char path[CHARN];
  char commands[CHARN][COMN];
  int  damages[COMN];
  char status[2*CHARN];
  char aa[IMGSIZ];
  int  size;
  char size_s[IMGSIZLEN];
};

// プロトタイプ宣言
void send_mes (FILE *fp, char *mes);
int  Load_Monster(char *fpath, struct monster monsters[]);

void okey (FILE *fp);
void sorry (FILE *fp);
void delete_client (int client);
void close_conn (int client);

// グローバル変数
int  count_access = 0;
int  nclients = 0;
int  clients[MAXNCLIENTS];
FILE *fp_arr[MAXNCLIENTS];
int  nmons;
struct monster monsters[MONSMAX];

char Bad_Request[] = "Bad Request.";
char Inp_Request[] = "Please enter s to display the slime. Enter x when close connection.";
char Lucky[] =  "ラッキー！";
char Normal[] = "普通です！";

int main (int argc, char *argv[]) {
  int listenfd, connfd, nbytes, client, port, i;
  struct sockaddr_in servaddr;
  char buf[256];
  
  // コマンドライン引数をチェック
  if (argc != 2) {
    printf("コマンドライン引数でポートが指定されていないため 8080 ポートでサーバを起動します\n");
    port = 8080;
  } else {
    port = atoi(argv[1]);
  }

  char fpath[] = "./AA/monsters.txt"; 
  nmons = Load_Monster(fpath, monsters);
  srand((unsigned int)time(NULL));

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
    
    for (i = 0; i < nclients; i++) {
      FD_SET(clients[i], &readfds);

      // maxfd の更新
      if (clients[i] > maxfd) {
        maxfd = clients[i];
      }
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
          delete_client(client);
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
        char *mes = "Sorry, it's full.\nPlease try again later.";
        send_mes(fp, mes);
        fclose(fp);
        close_conn(client);
        printf("But refused a new connection.\n");
      } else {
        // まだ空きがあった
        clients[nclients] = client;
        printf("Accepted a connection on descriptor %d.\n", client);
        fp_arr[nclients] = fp;
        nclients++;
        count_access++;
        char *mes = "Welcome!";
        send_mes(fp, mes);
        send_mes(fp, Inp_Request);
      }
    }

    // クライアントとの入出力
    for (i = 0; i < nclients; i++) {
      int client = clients[i];
      if (FD_ISSET(client, &readfds)) {
        // 返信を受け取ってチェック
        FILE *fp = fp_arr[i];
        char res[BUFSIZ];
        memset(&res, 0, sizeof(res));
        fgets(res, BUFSIZ, fp);
        printf("receive: %c.(client: %d)\n", res[0], client);
        // if (res[0] == 'x' | res[0] == EOF) { // 接続を終了する
        if (res[0] == 'x') { // 接続を終了する
          delete_client(client);
          continue;
        } else if (res[0] == 's') { // ランダムで aa を送信
          struct monster *monster;
          char *result;
          switch (rand()%9) {
            case 0: case 1:
              printf("send: metal.(client: %d)\n", client);
              result = Lucky;
              monster = &monsters[1]; break;
            default: 
              printf("send: slime.(client: %d)\n", client);
              result = Normal;
              monster = &monsters[0];
          }
          send_mes(fp, monster->name);
          send_mes(fp, monster->size_s);
          send_mes(fp, monster->aa);
          send_mes(fp, result);
        } else { // 入力エラー
          send_mes(fp, Bad_Request); 
        }
        send_mes(fp, Inp_Request);
      }
    }
  }
}

void send_mes (FILE *fp, char *mes) {
  char mes_ln[IMGSIZ+1];
  snprintf(mes_ln, sizeof(mes_ln), "%s\n", mes);
  if (fwrite(mes_ln, sizeof(unsigned char), strlen(mes_ln), fp) == -1) {
    perror("fwrite");
    exit(1);
  }
}

void delete_client (int client) {
  for (int i = 0; i < nclients; i++) {
    if (clients[i] == client) {
      clients[i] = clients[nclients - 1];
      fclose(fp_arr[i]);
      fp_arr[i] = fp_arr[nclients - 1];
      nclients--;
      break;
    }
  }
  
  printf("Connection closed on descriptor %d.\n", client);
  close_conn(client);
}

void close_conn (int client) {
  shutdown(client, SHUT_RDWR); 
  close(client);
}

int Load_Monster(char *fpath, struct monster monsters[]) {
  FILE *fp;
  int nmons = 0;
  if ((fp = fopen(fpath, "r")) == NULL) {
    perror("fopen");
    exit(1);
  }
  
  char line[BUFSIZ], *prof;
  for (int i = 0; fgets(line, BUFSIZ, fp) != NULL; i++, nmons++) {
    char *name = strtok(line, ","); // name
    for (int j = 0; name[j] != '\0'; j++) {
      monsters[i].name[j] = name[j];
      monsters[i].name[j+1] = '\0';
    }
    monsters[i].hp = atoi(strtok(NULL, ",")); // hp
    char *path = strtok(NULL, ","); // path
    for (int j = 0; path[j] != '\0'; j++) {
      monsters[i].path[j] = path[j];
      monsters[i].path[j+1] = '\0';
    }
    for (int j = 0; j < COMN; j++) { // commands
      char *command = strtok(NULL, ",");
      for (int k = 0; command[k] != '\0'; k++) {
        monsters[i].commands[j][k] = command[k];
        monsters[i].commands[j][k+1] = '\0';
      }
    }
    for (int j = 0; j < COMN; j++) { // damages
      monsters[i].damages[j] = atoi(strtok(NULL, ","));
    }
    snprintf(monsters[i].status, SNSIZ, "%s,%d", monsters[i].name, monsters[i].hp);

    FILE *aafp;
    int file_size;
    if ((aafp = fopen(monsters[i].path, "r")) == NULL) {
      perror("aa fopen");
      exit(1);
    }

    struct stat stbuf;
    if (fstat(aafp->_fileno, &stbuf) == -1) {
      perror("aa fstat");
      exit(1);
    }

    file_size = stbuf.st_size;
    if (file_size >= IMGSIZ) {
      perror("fread");
      exit(1);
    }
    fread(monsters[i].aa, sizeof(unsigned char), file_size, aafp);
    monsters[i].size = file_size;
    snprintf(monsters[i].size_s, IMGSIZLEN, "%d", file_size);

    fclose(aafp);
  }

  fclose(fp);
  return nmons;
}
