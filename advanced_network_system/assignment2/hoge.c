#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/select.h>

#define INPUT_MAX 256

int main () {
  pid_t pid, i;
  int status, ptoc[2], ctop[2];

  // make pipe
  pipe(ptoc);
  pipe(ctop);
  // FILE *readFp, *writeFp;
  // if ((readFp = fdopen(pfd[0], "r")) == NULL) {
  //   perror("readFp fdopen");
  //   exit(1);
  // }
  // setlinebuf(readFp);
  // if ((writeFp = fdopen(pfd[1], "w")) == NULL) {
  //   perror("writeFp fdopen");
  //   exit(1);
  // }
  // setlinebuf(writeFp);

  // make child process
  if ((pid = fork()) == 0) { // child
    char rec[INPUT_MAX+2];
    char out[INPUT_MAX+2];
    FILE *readFp, *writeFp;

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

    // パイプ1を標準出力に変更
    dup2(writeFp->_fileno, STDOUT_FILENO);
    fclose(writeFp);

    // 文字列を受け取る
    if (fgets(rec, sizeof(rec), readFp) == NULL) {
      perror("child fgets");
      exit(1);
    }
    fclose(readFp);
    printf("child: %s", rec);

    // exec low to high
    execlp("./low2high.sh", "./low2high.sh", rec, NULL);
    perror("execlp");
    exit(-1);
  } else { // parent
    char mes[INPUT_MAX];
    char send[INPUT_MAX+2];
    char rec[INPUT_MAX+2];
    FILE *readFp, *writeFp;

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

    printf("input -> ");
    scanf("%256s", mes);
    snprintf(send, sizeof(send), "%s\n", mes);
    if (fwrite(send, sizeof(unsigned char), strlen(send), writeFp) == -1) {
      perror("fwrite");
      exit(1);
    }

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(readFp->_fileno, &readfds);
    if (select(readFp->_fileno+1, &readfds, NULL, NULL, NULL) < 0) {
      perror("select");
      exit(1);
    }
    
    if (FD_ISSET(readFp->_fileno, &readfds) && (fgets(rec, sizeof(rec), readFp) == NULL)) {
      perror("parent fgets");
      exit(1);
    }
    printf("parent: %s!", rec);
    fclose(readFp);
    fclose(writeFp);
  }
  
  exit(0);
}
