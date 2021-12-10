#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mysql/mysql.h>

#define MAXSQLLEN  256

char *server_addr = "localhost";
char *user        = "testuser";
char *passwd      = "testPassword#1234";
char *db_name     = "conn_test";

int main() {
  MYSQL *conn    = NULL;
  MYSQL_RES *res = NULL;
  MYSQL_ROW row;
  char sql[MAXSQLLEN];

  // connect mysql
  conn = mysql_init(NULL);
  if (mysql_real_connect(conn, server_addr, user, passwd, db_name, 0, NULL, 0) < 0) {
    perror("mysql_real_connect");
    exit(1);
  }

  // exec sql
  snprintf(sql, sizeof(sql)-1, "select * from test_table;");
  if (mysql_query(conn, sql) < 0) {
    perror("mysql_query");
    mysql_close(conn);
    exit(1);
  }

  // response
  res = mysql_use_result(conn);
  printf(" id : text\n");
  while ((row = mysql_fetch_row(res)) != NULL) {
    printf("%4d: %s\n", atoi(row[0]), row[1]);
  }

  // close connection
  mysql_free_result(res);
  mysql_close(conn);

  exit(0);
}