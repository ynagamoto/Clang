#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>

char *NAME = "永元 陽一"; // 名前
int AGE = 20; // 年齢
char *nl = "\n"; // 改行
int nl_len = 1;

int main(int argc, char *argv[]) {
  // コマンドライン引数でファイル名が指定されていない場合の処理
  if (argc != 2) {
    printf("ファイル名が指定されていません．\nプログラムを終了します．\n");
    return 1;
  }

  // 名前の長さを調べる
  int name_len;
  for (name_len = 0; NAME[name_len] != '\0'; name_len++);

  // 年齢の長さを調べる
  int age = AGE;
  int i, age_len;
  int hoge[4] = {0};
  for (i = 0, age_len = 0; age != 0; i++, age /= 10, age_len++) {
    hoge[i] = (age % 10);
  }

  // 年齢を文字列にする
  int j = 0;
  char age_str[4];
  for (--i, j = 0; i >= 0; i--, j++) {
    age_str[j] = hoge[i] + '0';
  }
  age_str[j] = '\0';

  // ファイルを開く
  int fd = open(argv[1], O_WRONLY);
  // 開けなければエラー
  if (fd < 0) {
    printf("エラーが発生しました\n");
    return 1;
  } else {
    write(fd, NAME, name_len);   
    write(fd, nl, nl_len);
    write(fd, age_str, age_len);
    write(fd, nl, nl_len);
  }
  
  // ファイルを閉じて終了
  close(fd);
  return 0;
}
