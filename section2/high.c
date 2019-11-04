#include<stdio.h>

char *NAME = "永元 陽一"; // 名前
int AGE = 20; // 年齢

int main(int argc, char *argv[]) {
  // コマンドライン引数でファイル名が指定されていない場合の処理
  if (argc != 2) {
    printf("ファイル名が指定されていません．\nプログラムを終了します．\n");
    return 1;
  }

  FILE *fd;
  fd = fopen(argv[1], "w"); // ファイルを開く

  // ファイルを開くのに失敗した場合
  if (fd == NULL) {
    printf("ファイルを開くのに失敗しました\n");
    return 1;
  }

  // ファイルへの書き込み
  fprintf(fd, "%s\n%d\n", NAME, AGE);

  // ファイルを閉じる
  fclose(fd);

  return 0;
}
