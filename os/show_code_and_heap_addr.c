#include<stdio.h>
#include<stdlib.h>

void code_area() {

}

int main() {
  int *hoge;

  // 動的メモリの確保
  hoge = (int *)malloc(sizeof(int));
  *hoge = 0;

  printf("code_addr = %p\n", code_area);
  printf("heap_addr = %p\n", &hoge);
  
  // メモリの解放
  free(hoge);

  return 0;
}

