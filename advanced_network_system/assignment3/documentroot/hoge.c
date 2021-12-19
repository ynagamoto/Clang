#include<stdio.h>
#include<stdlib.h>

char html[] = "<!DOCTYPE html>\n\
<html>\n\
  <head>\n\
    <meta charset=\"utf-8\">\n\
  </head>\n\
  <body>\n\
    <h1>Test Hoge Page.</h1>\n\
    %s\n\
  </body>\n\
</html>\n";

int main(int argc, char *argv[]) {
  char line[] = "<p>hogehoge</p>";
  printf(html, line);
  exit(0);
}
