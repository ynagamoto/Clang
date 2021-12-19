#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mysql/mysql.h>

#define KILO       1024
#define MAXSQLLEN  1024

struct query {
  char key[KILO];
  char value[KILO];
};

//    <p>%s</p>\n
char html[] = "<!DOCTYPE html>\n\
<html>\n\
  <head>\n\
    <meta charset=\"utf-8\">\n\
  </head>\n\
  <body>\n\
    <h1>Search List</h1>\n\
    <table>\n\
      <tr>\n\
        <td>\n\
          <form action=\"\" method=\"get\">\n\
            <th><input type=\"text\" name=\"query\" size=\"20\"></th>\n\
            <th><input type=\"submit\" value=\"search\"></th>\n\
          </form>\n\
          <form action=\"\" method=\"get\">\n\
            <th><input type=\"submit\" value=\"reset\"></th>\n\
          </form>\n\
        </td>\n\
      </tr>\n\
    </table>\n\
    <table>\n\
      <tr>\n\
        <th>name</th>\n\
        <th>type</th>\n\
        <th>price</th>\n\
        <th>explain_text</th>\n\
        <th>seller</th>\n\
      </tr>\n\
      %s\
    </table>\n\
  </body>\n\
</html>\n";

char tr[] = "<tr>\n\
        <td>%s</td>\n\
        <td>%s</td>\n\
        <td>%s</td>\n\
        <td>%s</td>\n\
        <td>%s</td>\n\
      </tr>\n";

char *server_addr = "localhost";
char *user        = "testuser";
char *passwd      = "testPassword#1234";
char *db_name     = "merukari";

int main(int argc, char *argv[]) {
  MYSQL     *conn = NULL;
  MYSQL_RES *res  = NULL;
  MYSQL_ROW  row;
  char context[1024];
  context[0] = '\0'; // 念のため初期化
  char debug[KILO];

  // argv[1] > 0 
  char argv2_cp[KILO];
  struct query query;
  int query_num = atoi(argv[1]);
  strcpy(query.value, "");
  if (query_num) {
    strcpy(argv2_cp, argv[2]);
    char *temp_key = strtok(argv[2], "=");
    if (strtok(NULL, "\0") != NULL) {
      char *temp = strtok(argv2_cp, "=");
      strcpy(query.value, strtok(NULL, "\0"));
    } 
  }

  // connect mysql
  conn = mysql_init(NULL);
  if (mysql_real_connect(conn, server_addr, user, passwd, db_name, 0, NULL, 0) < 0) {
    perror("mysql_real_connect");
    exit(1);
  }
  // printf("connect OK\n");

  // exec sql
  // query_num = 0 のときは *
  // それ以外は %(query)%
  char sql[MAXSQLLEN];
  char where[KILO/4];
  strcpy(where, "");
  if (strcmp(query.value, "") != 0) {
    snprintf(where, sizeof(where), "where name like \"%%%s%%\"", query.value);
  }
  snprintf(sql, sizeof(sql)-1, "select * from products %s;", where);
  // snprintf(sql, sizeof(sql)-1, "select * from products;");
  if (mysql_query(conn, sql) < 0) {
    perror("mysql_query");
    mysql_close(conn);
    exit(1);
  }
  // printf("exec OK\n");

  // response (0 -> id, 1 -> name, 2 -> price, 3 -> type, 4 -> explain_text, 5 -> seller)
  // 必要なもの name, type, price, explain_text, seller
  res = mysql_use_result(conn);
  // printf("result OK"); // ここでセグメンテーションフォルスが出た場合はユーザの権限周りの可能性あり
  while ((row = mysql_fetch_row(res)) != NULL) {
    char tmp_tr[256];
    /* 
    for (int i = 0; i < 6; i++) {
      printf("%d -> %s, ", i, row[i]);
    }
    printf("\b\b \n"); */
    snprintf(tmp_tr, sizeof(tmp_tr), tr, row[1], row[3], row[2], row[4], row[5]);
    // printf("%s", tmp_tr);
    snprintf(context, sizeof(context), "%s%s", context, tmp_tr);
  }

  // close connection
  mysql_free_result(res);
  mysql_close(conn);

  printf(html, context);
  // printf(html, sql, context);

  exit(0);
}
