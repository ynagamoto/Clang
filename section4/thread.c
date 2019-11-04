#include<stdio.h>
#include<pthread.h>

int hoge = 0;

// 生成したスレッドで実行する関数
void *add_hoge() {
  int *res = 0;
  hoge += 10;
  printf("----------子スレッド----------\nグローバル変数 = %d\n\n", hoge);

  // スレッドの終了
  pthread_exit(NULL);
}

int main() {
  pthread_t handle;
  int res;

  // プロセスの生成
  pthread_create(&handle, NULL, add_hoge, NULL);
  res = pthread_join(handle, NULL);

  if (res != 0) {
    printf("スレッドが正しく終了しませんでした\n");
    return 1;
  }

  // グローバル変数の表示
  printf("----------親プロセス----------\nグローバル変数 = %d\n", hoge);

  return 0;
}
