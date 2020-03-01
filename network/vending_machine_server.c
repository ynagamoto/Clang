#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/select.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/uio.h>
#include<unistd.h>
#include<netinet/in.h>

enum {
  NQUEUESIZE = 5,
  MAXNCLIENTS = 10, // 接続可能なクライアントの上限
};

// プロトタイプ宣言
void vm_step1 (int client, int p);
void vm_step2 (int client, int p);
void write_mes (int client, char *mes);
void send_menu(int client);
void okey (int client);
void sorry (int client);
void delete_client (int client);
void close_conn (int client);

// グローバル変数
int nclients = 0;
int clients[MAXNCLIENTS];
int step[MAXNCLIENTS];
int order[MAXNCLIENTS];
int pay[MAXNCLIENTS];
int sales = 0;

// メニュー
char *menus[] = {"コーラ", "ソーダ", "お茶", "ラムネ", "コーヒー"};
int values[] = {150, 100, 150, 100, 120};
int nmenu = sizeof(menus) / sizeof(menus[0]);

int main (int argc, char *argv[]) {
  int listenfd, connfd, nbytes, client, port, i;
  struct sockaddr_in servaddr;
  char buf[256];
  
  // コマンドライン引数をチェック
  if (argc != 2) {
    printf("コマンドラインで使用するポート番号を指定して下さい\n");
    exit(0);
  }
  port = atoi(argv[1]);

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
  printf("Waiting for connection from clients...\nPress x when you want to close this server.\n");

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
      scanf("%s", buf);
      if (strlen(buf) == 1 && buf[0] == 'x') {
        printf("\nClose server.\n\n売上は %d円 です\n", sales);
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
        printf("\nNew connection!\n");
      } 

      // 受信可能クライアント数の上限を達しているか確認
      if (nclients >= MAXNCLIENTS) {
        // 上限に達しているためシャットダウンしてclose
        sorry(client);
        close_conn(client);
        printf("\nBut refused a new connection.\n");
      } else {
        // まだ空きがあった
        okey(client);
        clients[nclients] = client;
        step[nclients++] = 1;
        printf("\nAccepted a connection on descriptor %d.\n", client);

        // メニューを送る
        send_menu(client);
      }
    }

    // クライアントとの入出力
    for (i = 0; i < nclients; i++) {
      int client = clients[i]; 
      if (FD_ISSET(client, &readfds)) {
        if (step[i] == 1) {
          printf("descriptor %d step1 -> ", client);
          vm_step1(client, i);
        } else {
          printf("descriptor %d step2 -> ", client);
          vm_step2(client, i);
        }
      }
    }

  }
}

void vm_step1 (int client, int p) {
  char res[BUFSIZ];
  char mes[100];
  int temp;

  // 返信を受け取る
  memset(res, 0, sizeof(res));
  if ((temp = read(client, res, sizeof(res))) == -1) {
    perror("read, step1");
    exit(1);
  } else if (temp == 0) {
    delete_client(client);
    return;
  }
  printf("%s\n", res);

  temp = atoi(res)-1;

  // 値のチェック
  if (temp+1 > nmenu || temp < 0) {
    sprintf(mes, "メニュー表に存在するメニューを選んで下さい. : "); 
  } else {
    step[p] = 2;
    sprintf(mes, "\nc でメニューに戻る\nお金を入れて下さい.（%s : %d円）: ", menus[temp], values[temp]);
  }

  order[p] = temp;
  write_mes(client, mes);
}

void vm_step2 (int client, int p) {
  char res[BUFSIZ];
  char mes[100];
  char *menu = menus[order[p]];
  int value = values[order[p]];
  int temp;

  // 返信を受け取る
  memset(res, 0, sizeof(res));
  if ((temp = read(client, res, sizeof(res))) == -1) {
    perror("read, step2");
    exit(1);
  } else if (temp == 0) {
    delete_client(client);
    return;
  }
  printf("%s\n", res);

  // cが押されたらメニューに戻る
  if (strlen(res) == 1 && res[0] == 'c') {
    sprintf(mes, "\nお釣り%d円\n\n", pay[p]);
    write_mes(client, mes);
    printf("descriptor %d step2 -> step1\n", client);
    step[p] = 1;
    pay[p] = 0;
    send_menu(client);
    return;
  }
  temp = atoi(res); 
  pay[p] = temp > 0 ? pay[p]+temp : pay[p];

  // お金のチェック
  sprintf(mes, "\n現在の投入金額 : %d円\n", pay[p]);
  write_mes(client, mes);
  if (pay[p] >= value) {
    sprintf(mes, "\n%sを購入しました（お釣り%d円）\n\n", menu, pay[p]-value);
    write_mes(client, mes);

    sales += value;
    printf("\n利用者 %d が %s を購入しました．現在の売上 -> %d\n\n", client, menu, sales);
    step[p] = 1;
    pay[p] = 0;
    send_menu(client);
  } else {
    sprintf(mes, "\nお金を入れて下さい.（%s : %d円）: ", menu, value);
    write_mes(client, mes);
  }

}

void write_mes (int client, char *mes) {
  write(client, mes, strlen(mes));
}

void okey (int client) {
  char *message = "Welcome to Vending Machine!\n\n";
  write_mes(client, message);
}

void sorry (int client) {
  char *message = "Sorry, it's full.\nPlease try again later.\n";
  write_mes(client, message);
}

void send_menu (int client) {
  int i;

  for (i = 0; i < nmenu; i++) {
    char message[100];
 
    sprintf(message, "%d -> %s : %d円\n", i+1, menus[i], values[i]);
    write_mes(client, message);
  }

  char *mes = "x -> 終了する\n\n購入したい商品の番号を入力して下さい. : ";
  write_mes(client, mes);
}

void delete_client (int client) {
  int i;
  for (i = 0; i < nclients; i++) {
    if (clients[i] == client) {
      clients[i] = clients[nclients - 1];
      step[i] = step[nclients - 1];
      order[i] = order[nclients - 1];
      pay[i] = pay[nclients - 1];
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

