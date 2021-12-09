#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/select.h>
#include<sys/stat.h>

#define MAXCONN  10
#define METHSIZE 256
#define SIZE     (5*1024)

enum {
  NQUEUESIZE = MAXCONN,
  MAXNCLIENTS = MAXCONN, // 接続可能なクライアントの上限
};

char static_path[]    = "./static";
char header_content[] = "Content-Length: ";

void httpServer(int clientfd);

int main(int argc, char *argv[]) {
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
    close(listenfd);
    exit(1);
  }

  // listen
  if (listen(listenfd, NQUEUESIZE) < 0) {
    perror("listen");
    close(listenfd);
    exit(1);
  }
  printf("Waiting for connection from clients...\nEnter x when you want to close this server.\n");

  int count_access = 0;
  for (;;) {
    int i, maxfd;
    fd_set readfds;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(listenfd, &readfds);
    maxfd = listenfd >= STDIN_FILENO ? listenfd : STDIN_FILENO;
    
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
        close(listenfd);
        exit(0);
      }
    }

    // 新規接続
    if (FD_ISSET(listenfd, &readfds)) {
      int clientfd;
      if ((clientfd = accept(listenfd, (struct sockaddr *)NULL, NULL)) < 0) {
        perror("accept");
        close(listenfd);
        continue;
      } else {
        printf("New connection!\n");
      } 

      count_access++;
      httpServer(clientfd);
      
      close(clientfd);
    }
  }

  exit(0);
}

void httpServer(int clientfd) {
  int  status, req_size, res_size, file_size;
  char req_mes[SIZE], res_mes[SIZE*2], method[METHSIZE], target[METHSIZE], *fline;
  char file_path[SIZE], file_str[SIZE/2], header_field[SIZE];
  FILE *res_filefp;

  if (read(clientfd, req_mes, sizeof(req_mes)) < 0) {
    perror("read");
    close(clientfd);
  }

  printf("%s\n", req_mes);

  // check method and target
  fline  = strtok(req_mes, "\n");
  strcpy(method, strtok(fline, " "));
  if (method == NULL) {
    perror("method");
    close(clientfd);
    exit(1);
  }

  strcpy(target, strtok(NULL, " "));
  if (target == NULL) {
    perror("target");
    close(clientfd);
    exit(1);
  }


  if (strcmp(method, "GET") == 0) {
    if (strcmp(target, "/") == 0) {
      strcpy(target, "/index.html");
    }

    snprintf(file_path, sizeof(file_path), "%s%s", static_path, target);
    printf("file_path: %s\n", file_path);
    struct stat sb;
    if (stat(file_path, &sb) == -1) {
      perror("stat");
      close(clientfd);
      exit(1);
    }
    if ((file_size = sb.st_size) > 0) {
      if ((res_filefp = fopen(file_path, "r")) == NULL) {
        perror("fopen");
        close(clientfd);
        exit(1);
      }
      if (fread(file_str, sizeof(unsigned char), sizeof(file_str)/sizeof(unsigned char), res_filefp) < 0) {
        perror("fread");
        close(clientfd);
        fclose(res_filefp);
        exit(1);
      }
      fclose(res_filefp);
      status = 200;
    } else {
      status = 404;
    }
  } else {
    status = 404;
  }

  // make header field
  snprintf(header_field, sizeof(header_field), "%s%u\r\n", header_content, file_size);    

  // make response
  switch (status) {
    case 200:
      snprintf(res_mes, sizeof(res_mes), "HTTP/1.1 %d OK\r\n%s\r\n%s", status, header_field, file_str); 
      break;
    case 404:
    default:
      snprintf(res_mes, sizeof(res_mes), "HTTP/1.1 %d Not Found\r\n%s\r\n", status, header_field); 
  }
  res_size = strlen(res_mes);

  // send response
  // printf("%s\n", res_mes);
  if (write(clientfd, res_mes, sizeof(res_mes)) < 0) {
    perror("write");
    close(clientfd);
    exit(1);
  }
}
