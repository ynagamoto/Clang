#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>

int main(int argc, char *argv[]) {

  pid_t pid;
  int status;

  if (argc < 2) {
    printf("コマンドライン引数で実行するプログラムを設定して下さい\n");
    exit(1);
  }

  pid = fork();
  if (pid == 0) {
    execlp(argv[1], "", argv[2], NULL);
  } else {
    wait(&status);
  }

  return 0;
}

