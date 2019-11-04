#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>

// パイプのread, write用
#define READ  (0)
#define WRITE (1)

// 関数のプロトタイプ宣言
int make_pipe(int pp[]);
void close_pipe(int pp[]);

int main() {
  char read_str[100], write_str[100];
  int pp[2], str_len, status;
  pid_t pid, ch;

  printf("文字列を入力して下さい(100文字以内): ");
  scanf("%s", write_str);
  str_len = strlen(write_str) + 1;

  // make pipe
  if (make_pipe(pp) == 1) {
    printf("エラーが発生したためプログラムを終了します\n");
    return 1;
  }

  // make children process
  ch = fork();
  if (ch < 0) {
    // failed
    printf("fork に失敗しました\n");

    // 開いたパイプを閉じて終了
    close_pipe(pp);
    return 1;
  }

  if (ch == 0) { // children process
    // children process id
    pid = getpid();
    printf("children process id: %d\n", pid);

    // パイプの文字列を読み込む
    read(pp[READ], read_str, str_len);
    printf("受け取った文字列: %s\n", read_str);
  } else { // parent process
    // show process id
    pid = getpid();
    printf("parent process id: %d\n", pid);

    // パイプへ文字列を書き込む
    write(pp[WRITE], write_str, str_len);
    wait(&status);
  }
  
  close_pipe(pp);
  return 0;
}

// パイプを生成する関数
int make_pipe(int pp[]) {
  // make pipe
  if (pipe(pp) < 0) {
    // failed
    printf("パイプの生成に失敗しました\n");
    return 1;
  } 

  return 0;
}

// パイプを閉じる関数
void close_pipe(int pp[]) {
  // close parent pipe
  close(pp[READ]);
  close(pp[WRITE]);
}
