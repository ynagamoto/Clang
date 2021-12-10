#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/stat.h>

#define KILO 1024
#define SIZE (2*KILO)

char file_path[] = "./request_get.txt";
// char file_path[] = "./request_post.txt";

struct header {
  char key[KILO];
  char value[KILO];
};

struct query {
  char key[KILO];
  char value[KILO];
};

int main() {
  struct stat sb;
  FILE *fp;
  char req[SIZE], req_lines[KILO][SIZE], first_line[KILO], *line;
  int  req_line_num = 0, query_num = 0; 
  char method[KILO], target[KILO], *temp_target;
  
  // get file size
  if (stat(file_path, &sb) < 0) {
    perror("stat");
    exit(1);
  } 
  
  // open file
  if ((fp = fopen(file_path, "r")) == NULL) {
    perror("fopen");
    exit(1);
  }

  // read file
  if (fread(req, sizeof(unsigned char), sizeof(req), fp) < 0) {
    perror("fread");
    fclose(fp);
    exit(1);
  }
  // printf("%s", req);

  struct header headers[KILO];
  struct query  querys[KILO];
  line = strtok(req, "\n"); 
  strcpy(first_line, line);
  // printf("first line -> %s\n", line);

  // split request to line
  for (req_line_num = 0, line = strtok(NULL, "\n"); line != NULL; line = strtok(NULL, "\n")) {
    strcpy(req_lines[req_line_num], line);
    req_line_num++;
  }

  // analyze get query
  strcpy(method, strtok(first_line, " "));
  if (method == NULL) {
    perror("method");
    // close(clientfd);
    exit(1);
  }

  temp_target = strtok(NULL, " ");
  if (strchr(temp_target, (int)'?') != NULL) {
    strcpy(target, strtok(temp_target, "?"));
    // split text with & -> store get query
    char rem[KILO];
    char temp_query[KILO][KILO];
    strcpy(rem, strtok(NULL, "\0"));
    for (char *temp = strtok(rem, "&"); temp!= NULL; temp = strtok(NULL, "&")) {
      strcpy(temp_query[query_num], temp);
      query_num++;
    }

    for (int i = 0; i < query_num; i++) {
      char *temp = strtok(temp_query[i], "=");
      strcpy(querys[i].key, temp); 
      strcpy(querys[i].value, strtok(NULL, "\0")); 
    }
  } else {
    strcpy(target, temp_target); 
  }
  if (target == NULL) {
    perror("target");
    // close(clientfd);
    exit(1);
  }

  for (int i = 0; i < req_line_num; i++) {
    char *key;
    if (strchr(req_lines[i], (int)':') != NULL) {
      // printf("line -> %s\n", line);
      key = strtok(req_lines[i], ":");
      strcpy(headers[i].key, key);
      strcpy(headers[i].value, strtok(NULL, "\0"));
    } else { // analyze post query
      key = strtok(req_lines[i], "=");
      strcpy(querys[query_num].key, key);
      strcpy(querys[query_num].value, strtok(NULL, "\0"));
      query_num++;
    }
  } 

  for (int i = 0; i < query_num; i++) {
    printf("key -> %s, value -> %s\n", querys[i].key, querys[i].value);
  }
  
  exit(0);
}
