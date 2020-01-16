#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<sys/sem.h>

void *func(void *param);
char buff[256];

// セマフォ識別子の宣言
int semid;

int main() {
  pthread_t tid;

  // セマフォ設定用構造体の宣言
  struct sembuf sop;

  // セマフォの作成，初期化
  if ((semid = semget(IPC_PRIVATE, 1, 0600)) == -1) {
    printf("semget\n");
    exit(1);
  } 

  if (semctl(semid, 0, SETVAL, 0) == -1) {
    printf("semctl\n");
    exit(1);
  }

  pthread_create(&tid, NULL, func, NULL);

  printf("\nParent process : Input message ->");
  scanf("%s", buff);

  // セマフォの解放
  sop.sem_num = 0;
  sop.sem_op = 1;
  sop.sem_flg = 0;
  if (semop(semid, &sop, 1) == -1) {
    printf("semop / parent\n");
    exit(1);
  }

  pthread_join(tid, NULL);

  return 0;
}

void *func(void *param) {
  // セマフォ設定用構造体の宣言
  struct sembuf sop;

  // セマフォの獲得
  sop.sem_num = 0;
  sop.sem_op = -1;
  sop.sem_flg = 0;
  if (semop(semid, &sop, 1) == -1) {
    printf("semop / thread\n");
    exit(1);
  }

  printf("\nThread : Receive Message -> %s\n", buff);

  pthread_exit(0);
}
