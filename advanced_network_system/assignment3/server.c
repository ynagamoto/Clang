#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/select.h>
#include<sys/stat.h>
#include<sys/wait.h>

#define KILO      1024
#define MAXCONN   10
#define SIZE      (2*KILO)

enum {
  NQUEUESIZE = MAXCONN,
  MAXNCLIENTS = MAXCONN, // 接続可能なクライアントの上限
};

struct header {
  char key[KILO];
  char value[KILO];
};

struct query {
  char key[KILO];
  char value[KILO];
};

char static_path[]    = "./documentroot";
char header_content[] = "Content-Length: ";

void httpServer(int clientfd);

int main(int argc, char *argv[]) {
  int listenfd, connfd, nbytes, client, port, pid, status;
  struct sockaddr_in servaddr;
  char buf[256];
  
  // コマンドライン引数をチェック
  if (argc != 2) {
    printf("コマンドライン引数でポートが指定されていないため 8080 ポートでサーバを起動します\n");
    port = 8080;
  } else {
    port = atoi(argv[1]);
  }

  // ./code 以下の .c ファイルをコンパイルする
  printf("Compiling Clang source ... ");
  if ((pid = fork()) == 0) {
    execlp("./gcc_all.sh", "./gcc_all.sh", NULL);
    perror("execlp");
    exit(1);
  }
  wait(&status);
  printf("Done!\n");

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
      if ((pid = fork()) == 0) {
        httpServer(clientfd);
        close(clientfd);
        exit(0);
      } 
      close(clientfd);
    }
  }

  exit(0);
}

void httpServer(int clientfd) {
  int  status, req_size, res_size, file_size = 0, flag = 1;
  char req_mes[SIZE], res_mes[SIZE*2], req_lines[KILO][2*KILO], first_line[KILO];
  char method[KILO/4], target[KILO/4], *temp_target, *line, query_line[KILO];
  char file_path[SIZE], file_str[SIZE/2], header_field[SIZE];
  int  req_line_num = 0, query_num = 0; 
  FILE *res_filefp;
  struct header headers[KILO];
  struct query  querys[KILO];

  if (read(clientfd, req_mes, sizeof(req_mes)) < 0) {
    perror("read");
    // close(clientfd);
    flag = 0;
  }

  printf("%s\n", req_mes);

  // check method and target
  line = strtok(req_mes, "\n");
  strcpy(first_line, line);

  // split request to line
  for (req_line_num = 0, line = strtok(NULL, "\n"); line != NULL; line = strtok(NULL, "\n")) {
    strcpy(req_lines[req_line_num], line);
    req_line_num++;
  }

  // split method
  strcpy(method, strtok(first_line, " "));
  if (method == NULL) {
    // perror("method");
    // close(clientfd);
    // exit(1);
    flag = 0;
  }
  // printf("method -> %s\n", method);

  // split target and analyze get query
  temp_target = strtok(NULL, " ");
  if (strchr(temp_target, (int)'?') != NULL) {
    strcpy(target, strtok(temp_target, "?"));
    // split text with & -> store get query
    char temp_query[KILO][KILO];
    char rem[KILO];
    strcpy(rem, strtok(NULL, "\0"));
    strcpy(query_line, rem);
    for (char *temp = strtok(rem, "&"); temp!= NULL; temp = strtok(NULL, "&")) {
      strcpy(temp_query[query_num], temp);
      query_num++;
    }

    for (int i = 0; i < query_num; i++) {
      char *temp = strtok(temp_query[i], "=");
      strcpy(querys[i].key, temp); 
      strcpy(querys[i].value, strtok(NULL, "\0")); 
    }
  } else {
    strcpy(target, temp_target); 
  }
  if (target == NULL) {
    // perror("target");
    // close(clientfd);
    // exit(1);
    flag = 0;
  }
  // printf("target -> %s\n", target);

  // analyze post query
  for (int i = 0; i < req_line_num; i++) {
    char *key;
    if (strchr(req_lines[i], (int)':') != NULL) {
      // printf("line -> %s\n", line);
      key = strtok(req_lines[i], ":");
      strcpy(headers[i].key, key);
      strcpy(headers[i].value, strtok(NULL, "\0"));
    } else if (strchr(req_lines[i], (int)'=') != NULL) { // analyze post query
      key = strtok(req_lines[i], "=");
      strcpy(querys[query_num].key, key);
      strcpy(querys[query_num].value, strtok(NULL, "\0"));
      snprintf(query_line, sizeof(query_line), "%s&%s", query_line, req_lines[i]);
      query_num++;
    }
  }
  printf("query_line -> %s\n", query_line);

  // .html のときは ./static 以下の html を，.c のときはプログラムの出力結果を返す
  if (flag && (strcmp(method, "GET") == 0 || strcmp(method, "POST") == 0)) {
    if (strcmp(target, "/") == 0) {
      strcpy(target, "/index.html");
    }
    
    struct stat sb;
    snprintf(file_path, sizeof(file_path), "%s%s", static_path, target);
    if (stat(file_path, &sb) == -1) { // 指定されたファイルが存在しなかった
      status = 404;
    } else {
      if (strstr(target, ".html") != NULL) {
        printf("html\n");
        snprintf(file_path, sizeof(file_path), "%s%s", static_path, target);
        // printf("file_path: %s\n", file_path);
        if ((file_size = sb.st_size) > 0) {
          if ((res_filefp = fopen(file_path, "r")) != NULL) {
            if (fread(file_str, sizeof(unsigned char), sizeof(file_str)/sizeof(unsigned char), res_filefp) < 0) {
              perror("fread");
              // close(clientfd);
              status = 500;
            } else {
              status = 200;
            }
            fclose(res_filefp);
          } else { // 存在するが開けない
            status = 500;
          }
        } else {
          status = 404;
        }
      } else if (strstr(target, ".c") != NULL){
        printf("c\n");
        int execpid;
        int ctop[2];
        pipe(ctop);
        if ((execpid = fork()) == 0) {
          char *bin;
          bin = strtok(&target[1], "."); // target[0] は '/' なので target[1] から
          printf("target -> %s, bin -> %s, query_line -> %s\n", &target[1], bin, query_line);
          close(ctop[0]);
          dup2(ctop[1], STDOUT_FILENO);
          execlp("./crun.sh", "./crun.sh", bin, query_num, query_line, NULL); 
          perror("execlp");
          close(ctop[1]);
          close(clientfd);
          exit(1);
        }
        // 結果を受け取る
        close(ctop[1]);
        if (read(ctop[0], file_str, sizeof(file_str)) < 0) {
          perror("read");
          status = 500;
        }
        close(ctop[0]);
        status = 200;
        file_size = strlen(file_str);
      }
    }
  } else { // GET と POST 以外
    status = 404;
  }

  // make response
  switch (status) {
    case 200:
      snprintf(header_field, sizeof(header_field), "%s%u\r\n", header_content, file_size); // make header field
      snprintf(res_mes, sizeof(res_mes), "HTTP/1.1 %d OK\r\n%s\r\n%s", status, header_field, file_str); 
      break;
    case 500:
      snprintf(file_path, sizeof(file_path), "%s/500.html", static_path);
      if ((res_filefp = fopen(file_path, "r")) == NULL) { // 流石にここでエラーが起きるとWebサーバとしてまずいので止める
        perror("500 fopen");
        exit(1);
      }
      if ((fread(file_str, sizeof(unsigned char), sizeof(file_str), res_filefp)) < 0) { // 同様
        perror("500 fread");
        exit(1);
      }
      snprintf(header_field, sizeof(header_field), "%s%lu\r\n", header_content, strlen(file_str)); // make header field
      snprintf(res_mes, sizeof(res_mes), "HTTP/1.1 %d Internal Server Error\r\n%s\r\n%s", status, header_field, file_str); 
      break;
    case 404:
    default:
      snprintf(file_path, sizeof(file_path), "%s/404.html", static_path);
      if ((res_filefp = fopen(file_path, "r")) == NULL) { // 流石にここでエラーが起きるとWebサーバとしてまずいので止める
        perror("404 fopen");
        exit(1);
      }
      if ((fread(file_str, sizeof(unsigned char), sizeof(file_str), res_filefp)) < 0) { // 同様
        perror("404 fread");
        exit(1);
      }
      snprintf(header_field, sizeof(header_field), "%s%lu\r\n", header_content, strlen(file_str)); // make header field
      snprintf(res_mes, sizeof(res_mes), "HTTP/1.1 %d Not Found\r\n%s\r\n%s", status, header_field, file_str); 
  }
  res_size = strlen(res_mes);

  /* for (int i = 0; i < query_num; i++) {
    printf("key -> %s, value -> %s\n", querys[i].key, querys[i].value);
  } */
  
  // send response
  printf("%s\n", res_mes);
  if (write(clientfd, res_mes, sizeof(res_mes)) < 0) {
    perror("write");
    close(clientfd);
    exit(1);
  }
}
