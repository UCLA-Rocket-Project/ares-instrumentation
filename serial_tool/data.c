#include "serial_handler.h"
#include "data.h"

#define MAX_NODES_BEFORE_COUNTERMEASURES (1<<20)
#define SQL_N (1<<18)

#if USE_MYSQL==1
MYSQL* con = NULL;
#endif
char* sql = NULL;
FILE* log_fd = NULL;
int sending_query = 0;

FILE* open_new_log(char* path_to_log_dir) {
  FILE* fd = NULL;
  char file_name[1024];
  char file_name_temp[1024];
  size_t path_len = strlen(path_to_log_dir);
  if (path_len > 512)
    return NULL;
  if (path_to_log_dir[path_len-1] != '/') {
    path_to_log_dir[path_len+1] = 0;
    path_to_log_dir[path_len] = '/';
  }

  int i = 0;
  do {
    if (fd != NULL)
      fclose(fd);
    sprintf(file_name_temp, "log%03i.txt", i);
    strcpy(file_name, path_to_log_dir);
    strncat(file_name, file_name_temp, 1024 - strlen(file_name) - 1);
    fd = fopen(file_name, "r");
    if (i++ >= MAX_LOG_N) 
      return NULL;
  } while (fd != NULL);
  //if (verbose) printf("Opening log with %s name\n", file_name);
  fd = fopen(file_name, "w+"); 
  if (fd == NULL)
    return NULL;
  //if (fd == NULL) printf("INVALID file desc\n");
  char line[1024];
  sprintf(line, "abs_t,%s\n", model);
  fwrite(line, sizeof(char), strlen(line), fd);
  return fd;
}

str_node* add_str_node(str_node* str, char* line, double time) {
  str_node* str_curr;
  str_curr = (str_node*)malloc(sizeof(str_node));
  str_curr->next = NULL;
  if (str != NULL)
    str->next = str_curr;
  strncpy(str_curr->text, line, sizeof(str_curr->text));
  (str_curr->text)[sizeof(str_curr->text)-1] = 0;
  str_curr->time = time;

  return str_curr;
}

void free_str_nodes(str_node* str) {
  str_node* last_node;
  str_node* curr_node;
  curr_node = str;
  int i = 0;
  //printf("There seem to be %i nodes left\n", atomicRead32(&str_list_size));
  while (curr_node != NULL) {
    last_node = curr_node; 
    curr_node = curr_node->next;
    free(last_node);
    i++;
  }
  //printf("There were %i nodes freed\n", i);
}

#if USE_MYSQL==1
void handle_mysql_str_list(str_node** head_ptr) {
  uint64_t t_handle_str_list1 = millis();
  status->mysql_t = t_handle_str_list1; 

  if (*head_ptr == NULL) return;

  int i;
  char** model_data_keys;
  int model_data_n = parse_CSV(model, &model_data_keys);

  sql = (char*)malloc(sizeof(char) * SQL_N);
  char temp[256];
  sprintf(sql, "INSERT INTO %s (%s,", mysql_table, "abs_t");
  int k = 0;
  for (k = 0; k < model_data_n; k++) {
    sprintf(temp, "%s,", model_data_keys[k]);
    strcat(sql, temp);
  }
  sql[strlen(sql)-1] = 0; // delete last comma
  strcat(sql, ") VALUES ");
  size_t sql_len = strlen(sql);

  str_node* curr_node = *head_ptr;
  str_node* last_node = NULL;
  size_t valid_nodes = 0;
  size_t parsed_nodes = 0;

  while (curr_node->next != NULL && (sql_len + 256) < SQL_N) {
    last_node = curr_node;
    curr_node = curr_node->next;
    parsed_nodes++;

    char values_line[512];
    values_line[0] = 0;
    //int text_len = strlen(curr_node->text);
    //int comma_nb = 0;
    /*
       for (i = 0; i < text_len; i++) {
       char c = (curr_node->text)[i];
       if (c == ',') comma_nb++;
       }
     */
    char** data_fields;
    int data_n = parse_CSV(curr_node->text, &data_fields);
    if (data_n == model_data_n) { // node is valid
      double rel_t;
      if (data_n > 0) {
        sscanf(data_fields[0], "%lf", &rel_t);
        //printf("abs_t = %f            rel_t = %f\n", curr_node->time, rel_t);
      }
      sprintf(values_line, "(%f,", curr_node->time);
      strcat(values_line, curr_node->text);
      strcat(values_line, "), ");
      int values_len = strlen(values_line);
      memcpy(sql+sql_len, values_line, sizeof(char)*(values_len+1));
      sql_len += values_len;
      valid_nodes++;

    }
    free_CSV_fields(data_fields, data_n);
    if (parsed_nodes > (1<<15))
      break;
  }
  sql[sql_len-2] = 0; // delete last comma
  sql_len--;

  status->mysql_nodes = valid_nodes;
  //if (verbose) printf("Analyzed %zu nodes\n", parsed_nodes);

  if (valid_nodes > 0 && atomicRead32(&str_list_size_mysql) < MAX_NODES_BEFORE_COUNTERMEASURES) {
    sending_query = 1;
    connect_to_mysql(0);
    //uint64_t t1 = millis(); 
    if (con != NULL && mysql_query(con, sql)) {
      //if (verbose) printf("%s\n", mysql_error(con));
      mysql_close(con);
      con = NULL;
      status->mysql_error = MYSQL_INSERTION_ERROR;
    }
    sending_query = 0;
    //uint64_t t2 = millis(); 
    if (con != NULL) {
      //if (verbose) printf("MYSQL insertion took %llu ms\n", t2 - t1);
      mysql_close(con);
      con = NULL;
    }
  }

  free_CSV_fields(model_data_keys, model_data_n);
  free(sql);
  sql = NULL;

  curr_node = *head_ptr;
  for (i = 0; i < parsed_nodes; i++) {
    last_node = curr_node;
    curr_node = curr_node->next;
    *head_ptr = curr_node;
    free(last_node);
    //str_list_size--;
    atomicAdd32(&str_list_size_mysql, -1);
  }
  //uint64_t t_handle_str_list2 = millis();

  //if (verbose) printf("Whole loop-run took %llu ms\n", t_handle_str_list2 - t_handle_str_list1);

  return;
}
#endif

void handle_log_str_list(str_node** head_ptr) {
  uint64_t t_handle_str_list1 = millis();
  status->log_t = t_handle_str_list1;

  if (*head_ptr == NULL) return;

  int i;
  char** model_data_keys;
  int model_data_n = parse_CSV(model, &model_data_keys);

  str_node* curr_node = *head_ptr;
  str_node* last_node = NULL;
  size_t valid_nodes = 0;
  size_t parsed_nodes = 0;

  while (curr_node->next != NULL) {
    last_node = curr_node;
    curr_node = curr_node->next;
    parsed_nodes++;

    char values_line[512];
    values_line[0] = 0;
    char** data_fields;
    int data_n = parse_CSV(curr_node->text, &data_fields);
    if (data_n == model_data_n) { // node is valid
      double rel_t;
      if (data_n > 0) {
        sscanf(data_fields[0], "%lf", &rel_t);
        //printf("abs_t = %f            rel_t = %f\n", curr_node->time, rel_t);
      }
      valid_nodes++;
      char try_opening_new_log = 0;
      while (try_opening_new_log < 3 && log_fd == NULL) {
        status->log_error = LOG_LOG_FD_ERROR;
        log_fd = open_new_log(path_to_log_dir);
        try_opening_new_log++;
      }
      if (log_fd != NULL) {
        status->log_error = 0;
        sprintf(values_line, "%f,%s\n", curr_node->time, curr_node->text);
        fwrite(values_line, sizeof(char), strlen(values_line), log_fd);

        soc_node* soc_list = head_soc_list;
        //skip the dummy, dummy necessary for safe linked list access of concurrent
        //threads, I think, unless something can come up with something better
        if (soc_list != NULL) soc_list = soc_list->next;
        while (valid_nodes > 0 && soc_list != NULL) {
          sendto(soc_list->fd, values_line, strlen(values_line), 0, (struct sockaddr*)&(soc_list->servaddr), sizeof(soc_list->servaddr));
          soc_list = soc_list->next;
        }
      } else {
        status->log_error = LOG_LOG_FD_ERROR;
        log_fd = open_new_log(path_to_log_dir);
      }

    }
    free_CSV_fields(data_fields, data_n);
    if (parsed_nodes > (1<<15))
      break;
  }
  status->log_nodes = valid_nodes;

  free_CSV_fields(model_data_keys, model_data_n);

  curr_node = *head_ptr;
  for (i = 0; i < parsed_nodes; i++) {
    last_node = curr_node;
    curr_node = curr_node->next;
    *head_ptr = curr_node;
    free(last_node);
    atomicAdd32(&str_list_size_log, -1);
  }
  return;
}

#if USE_MYSQL==1
void connect_to_mysql(int init) {
  if (init) {
    //printf("##Connecting to mysql database at %s with the login: %s and password: %s; using database: %s and table: %s\n",
    //     mysql_addr, mysql_login, mysql_pswd, mysql_db, mysql_table);
  }
  char** model_data_keys;
  int model_data_n = parse_CSV(model, &model_data_keys);

  char temp[1024];
  char sql_temp[512]; 
  unsigned int mysql_arg = 3; // 3 seconds timeout

  con = mysql_init(NULL);
  if (con == NULL) {
    //if (verbose) printf("##%s\n", mysql_error(con));
    mysql_close(con);
    status->mysql_error = MYSQL_INIT_ERROR;
    con = NULL;
    return;
  }
  if (mysql_options(con, MYSQL_OPT_READ_TIMEOUT, &mysql_arg)) {
    //if (verbose) printf("##%s\n", mysql_error(con));
    mysql_close(con);
    con = NULL;
    return;
  }
  if (mysql_options(con, MYSQL_OPT_CONNECT_TIMEOUT, &mysql_arg)) {
    //if (verbose) printf("##%s\n", mysql_error(con));
    mysql_close(con);
    status->mysql_error = MYSQL_TIMEOUT_SET_ERROR;
    con = NULL;
    return;
  }

  if (mysql_real_connect(con, mysql_addr, mysql_login, mysql_pswd, NULL, 0, NULL, 0) == NULL) {
    //if (verbose) printf("##%s\n", mysql_error(con));
    mysql_close(con);
    status->mysql_error = MYSQL_CONNECT_ERROR;
    con = NULL;
    return;
  }

  if (init) {
    sprintf(sql_temp, "CREATE DATABASE IF NOT EXISTS %s", mysql_db);
    //if (verbose) printf("##%s\n", sql_temp);
    if (mysql_query(con, sql_temp)) {
      //if (verbose) printf("##%s\n", mysql_error(con));
      mysql_close(con);
      status->mysql_error = MYSQL_CREATE_DB_ERROR;
      con = NULL;
    }
  }

  sprintf(sql_temp, "USE %s", mysql_db);
  //if (verbose) printf("##%s\n", sql_temp);
  if (mysql_query(con, sql_temp)) {
    //if (verbose) printf("##%s\n", mysql_error(con));
    mysql_close(con);
    status->mysql_error = MYSQL_USE_DB_ERROR;
    con = NULL;
  }

  if (init) {
    sprintf(sql_temp, "CREATE TABLE IF NOT EXISTS %s (id int primary key not null auto_increment,", mysql_table);
    strcat(sql_temp, "abs_t double,");
    int k = 0;
    for (k = 0; k < model_data_n; k++) {
      sprintf(temp, "%s double,", model_data_keys[k]);
      strcat(sql_temp, temp);
    }
    sql_temp[strlen(sql_temp)-1] = 0; // delete last comma
    strcat(sql_temp, ")");
    //if (verbose) printf("##%s\n", sql_temp);
    if (mysql_query(con, sql_temp)) {
      //if (verbose) printf("##%s\n", mysql_error(con));
      mysql_close(con);
      con = NULL;
      status->mysql_error = MYSQL_CREATE_TABLE_ERROR;
    }
  }
  free_CSV_fields(model_data_keys, model_data_n);
  status->mysql_error = 0;
}
#endif

#if USE_MYSQL==1
void* handle_mysql_data(void* arg) {

  uint64_t last_written = millis();

  connect_to_mysql(1);

  if (head_str_list_mysql == NULL) {
    //if (verbose) printf("Error: head_str_list_mysql is NULL, exiting mysql thread\n");
    status->mysql_error = LIST_NULL_ERROR;
    return NULL;
  }

  //printf("##Starting data logging loop\n");
  while (running) {
    if (millis() - last_written > 100) {
      handle_mysql_str_list(&head_str_list_mysql);
      last_written = millis();
    }
    delay(10);
  }
  //if (verbose) printf("Exiting mysql thread\n");
  if (con != NULL) {
    mysql_close(con);
  }
  free_str_nodes(head_str_list_mysql);
  head_str_list_mysql = NULL;
  status->mysql_error = CLEAN_EXIT_ERROR;
  return NULL;
}
#endif

void* handle_log_data(void* arg) {
  uint64_t last_written = millis();

  log_fd = open_new_log(path_to_log_dir);

  if (head_str_list_log == NULL) {
    //if (verbose) printf("Error: head_str_list_log is NULL, exiting mysql thread\n");
    status->log_error = LIST_NULL_ERROR;
    return NULL;
  }

  //printf("##Starting data logging loop\n");
  while (running) {
    if (millis() - last_written > 100) {
      handle_log_str_list(&head_str_list_log);
      fflush(log_fd);
    }
    delay(10);
  }
  free_str_nodes(head_str_list_log);
  head_str_list_log = NULL;
  status->log_error = CLEAN_EXIT_ERROR;
  fclose(log_fd);
  return NULL;
}
