#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>
#include<string.h>

int main() {
  pid_t pid, ch, status;
  char *hoge;
  scanf("%s", hoge);
  printf("%s, %ld\n", hoge, strlen(hoge));

  ch = fork();
  if (ch == 0) {
    pid = getpid();
    printf("Children process id: %d\n", pid);
  } else {
    if (ch != -1){
      wait(&status);
      pid = getpid();
      printf("Parent process id: %d\n", pid);
    }
  }

  return 0;
}

