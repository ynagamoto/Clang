#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>

int hoge = 0;

int main() {
  pid_t pid, status;
  
  // 子プロセスの生成
  pid = fork();
  if (pid == 0) {
    // 子プロセスの処理
    hoge += 10;
    printf("----------子プロセス----------\nグローバル変数 = %d\n\n", hoge);
  } else {
    wait(&status);
    printf("----------親プロセス----------\nグローバル変数 = %d\n", hoge);
  }
  
  return 0;
} 
