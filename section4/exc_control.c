#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

// プロトタイプ宣言
void *func1(void *param);
void *func2(void *param);

// グローバル変数
int counter = 0;
pthread_mutex_t mp; // mutex の宣言

int main() {
	pthread_t tid1, tid2;

	pthread_mutex_init(&mp, &counter); // mutex の初期化

	pthread_create(&tid1, NULL, func1, NULL);
	pthread_create(&tid2, NULL, func2, NULL);

	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);

	printf("\nFinish!\n"); // 終わったことを伝える文章を追加

}

void *func1(void *param) {
	int i, tmp;

	for (i = 0; i < 10; i++) {
		pthread_mutex_lock(&mp); // counter のロック

		tmp = counter;
		sleep(rand()%4);
		tmp = tmp + 3;
		counter = tmp;
		printf("\n\n i = %d counter -> %d", i, counter);

		pthread_mutex_unlock(&mp); // counter のロックを解除
	}

	pthread_exit(0); 
}

void *func2(void *param) {
	int k, tmp;
	
	for (k = 0; k < 10; k++) {
		pthread_mutex_lock(&mp); // counter をロック


		tmp = counter;
		sleep(rand()%4);
		tmp = tmp + 5;
		counter = tmp;
		printf("\n k = %d coutner -> %d", k, counter);
		
		pthread_mutex_unlock(&mp); // counter のロックを解除
	}

	pthread_exit(0); 
}
