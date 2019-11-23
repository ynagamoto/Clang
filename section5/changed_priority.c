#include<stdio.h>
#include<unistd.h>
#include<sys/time.h>

int main() {
  pid_t pid, ch, status;
  int i, limit = 1000000;
  int start_time, run_time;
  struct timeval now;

  gettimeofday(&now, NULL);
  start_time = now.tv_usec;
  

  ch = fork();
  if (ch == 0) {
    // 優先度の設定
    nice(19);

    // 100万回ループ
    for (i = 0; i < limit; i++);

    // 実行にかかった時間を表示
    gettimeofday(&now, NULL);
    run_time = now.tv_usec - start_time;
    printf("子プロセス，優先度：19，実行時間：%d\n", run_time);
  } else {
    // 優先度の設定
    nice(-19);

    // 100万回ループ
    for (i = 0; i < limit; i++);

    // 実行にかかった時間を表示
    gettimeofday(&now, NULL);
    run_time = now.tv_usec - start_time;
    printf("親プロセス，優先度：-19，実行時間：%d\n", run_time);
    
    // 子プロセスを待つ
    wait(&status);
  }

  return 0;
}