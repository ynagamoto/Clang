#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>

#define SNSIZ     256
#define CHARN     64
#define COMN      3
#define IMAGESIZ  1024

struct monster {
  char name[CHARN];
  int  hp;
  char path[CHARN];
  char commands[CHARN][COMN];
  int   damages[COMN];
  char status[2*CHARN];
  char aa[IMAGESIZ];
};

int Load_Monster(char *fpath, struct monster monsters[]);

int MAX = 10;

int main() {
  char fpath[] = "./monsters.txt"; 
  struct monster monsters[MAX], *monster;
  int nmons = Load_Monster(fpath, monsters);

  for (int i = 0; i < nmons; i++) {
    monster = &monsters[i];
    printf("path: %s, name: %s, hp: %d, ", monster->path, monster->name, monster->hp);
    printf("status: %s\n", monster->status);
    for (int j = 0; j < 3; j++) printf(" command%d: %s/%d,", j, monster->commands[j], monster->damages[j]);
    printf("\b\n");
    printf("%s\n", monster->aa);
  }

  return 0;
}

int Load_Monster(char *fpath, struct monster monsters[]) {
  FILE *fp;
  int nmons = 0;
  if ((fp = fopen(fpath, "r")) == NULL) {
    perror("fopen");
    exit(1);
  }
  
  char line[BUFSIZ], *prof;
  for (int i = 0; fgets(line, BUFSIZ, fp) != NULL; i++, nmons++) {
    char *name = strtok(line, ","); // name
    for (int j = 0; name[j] != '\0'; j++) {
      monsters[i].name[j] = name[j];
      monsters[i].name[j+1] = '\0';
    }
    monsters[i].hp = atoi(strtok(NULL, ",")); // hp
    char *path = strtok(NULL, ","); // path
    for (int j = 0; path[j] != '\0'; j++) {
      monsters[i].path[j] = path[j];
      monsters[i].path[j+1] = '\0';
    }
    for (int j = 0; j < COMN; j++) { // commands
      char *command = strtok(NULL, ",");
      for (int k = 0; command[k] != '\0'; k++) {
        monsters[i].commands[j][k] = command[k];
        monsters[i].commands[j][k+1] = '\0';
      }
    }
    for (int j = 0; j < COMN; j++) { // damages
      monsters[i].damages[j] = atoi(strtok(NULL, ","));
    }
    snprintf(monsters[i].status, SNSIZ, "%s,%d", monsters[i].name, monsters[i].hp);

    FILE *aafp;
    int file_size;
    if ((aafp = fopen(monsters[i].path, "r")) == NULL) {
      perror("aa fopen");
      exit(1);
    }

    struct stat stbuf;
    if (fstat(aafp->_fileno, &stbuf) == -1) {
      perror("aa fstat");
      exit(1);
    }

    file_size = stbuf.st_size;
    if (file_size >= IMAGESIZ) {
      perror("fread");
      exit(1);
    }
    fread(monsters[i].aa, sizeof(unsigned char), file_size, aafp);

    fclose(aafp);
  }

  fclose(fp);
  return nmons;
}
