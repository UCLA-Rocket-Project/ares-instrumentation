#ifndef SERIAL_HANDLER_H
#define SERIAL_HANDLER_H 

/* BEGIN EXTERNAL LIBRARY INCLUDES */
#include <netdb.h>

#if USE_MYSQL == 1
#include <mysql/mysql.h>
//#include <mysql.h>
#endif
/*END EXTERNAL LIBRARY INCLUDES */


/*BEGIN INTERNAL LIBRARY INCLUDES */
#include "JSON.h"
#include "CSV.h"
/*END INTERNAL LIBRARY INCLUDES */


/* BEGIN DEFAULT VALUES DEFINITIONS */
/* END DEFAULT VALUES DEFINITIONS */


/* BEGIN TYPES DEFINITIONS */
struct str_node {
  char text[256];
  double time;
  struct str_node* next;
};
struct soc_node {
  int fd;
  struct sockaddr_in servaddr;
  struct soc_node* next;
};
typedef struct str_node str_node;
typedef struct soc_node soc_node;
struct status_struct {
  uint64_t mysql_t;
  uint64_t server_t;
  uint64_t log_t;
  uint32_t mysql_nodes;
  uint64_t server_requests;
  uint32_t log_nodes;
  uint32_t mysql_error;
  uint32_t log_error;
  uint32_t server_error;
  uint32_t pipe_error;
  str_node values;
};
typedef struct status_struct status_struct;
/* END TYPES DEFINITIONS */


/* BEGIN GLOBALS DEFINITIONS */
extern volatile int running;
extern char model[1024];
extern char mysql_addr[256];
extern char mysql_login[256];
extern char mysql_pswd[256];
extern char mysql_table[256];
extern char mysql_db[256];
char serial_path[2048];
#if USE_MYSQL==1
extern MYSQL* con;
#endif
extern volatile int verbose;
extern FILE* log_fd;
extern int fd;
extern char path_to_log_dir[4096];
extern soc_node* head_soc_list;
extern soc_node* tail_soc_list;
extern str_node* head_str_list_mysql;
extern str_node* tail_str_list_mysql;
extern str_node* head_str_list_server;
extern str_node* tail_str_list_server;
extern str_node* head_str_list_log;
extern str_node* tail_str_list_log;
extern int32_t str_list_size_mysql;
extern int32_t str_list_size_server;
extern int32_t str_list_size_log;
extern int serial_speed;
extern char* sql;
extern status_struct* status;
extern char use_log;
extern char use_mysql;
extern char use_server;
extern int server_port;
/* END GLOBALS DEFINITIONS */


/* BEGIN FUNCTIONS DEFINITIONS */
//used by main
//void sighandler(int signum);
//void print_help_message();
//soc_node* add_soc_node(soc_node* soc, char* ip, int port);
//void free_soc_nodes(soc_node* soc);

//used by utils
//int open_port(char* serial_path, int speed);
//int wait_on_data(int fd, uint64_t millisec);
//void imprint_current_time(char* s);
//size_t parse_newline_and_log(char* buffer, size_t buff_len, soc_node* soc_list, str_node** str_list);
str_node* add_str_node(str_node* str, char* line, double time);
void* obtain_data(void* arg);
void ignore_sigusr1(int signum);

//low level timing functions
uint64_t millis();
uint64_t micros();
void delay(uint64_t millisec);
void hard_delay(uint64_t millisec);
void delayMicroseconds(uint64_t microsec);
void hard_delayMicroseconds(uint64_t microsec);

void* server(void* arg);

//used by data
//FILE* open_new_log(char* path_to_log_dir);
void free_str_nodes(str_node* str);
//void handle_str_list(str_node** head_ptr);
void* handle_mysql_data(void* arg);
void* handle_log_data(void* arg);
//void connect_to_mysql(int init);
/* END FUNCTIONS DEFINITIONS */

//error definitions
#define MYSQL_INIT_ERROR 1
#define MYSQL_TIMEOUT_SET_ERROR 2
#define MYSQL_CONNECT_ERROR 3
#define MYSQL_CREATE_DB_ERROR 4
#define MYSQL_USE_DB_ERROR 5
#define MYSQL_CREATE_TABLE_ERROR 6
#define LIST_NULL_ERROR 100
#define CLEAN_EXIT_ERROR 101 //not really an error, but a naming convention
#define MYSQL_INSERTION_ERROR 102
#define LOG_LOG_FD_ERROR 103
#define PIPE_COULD_NOT_OPEN_ERROR 200
#define PIPE_CLOSED_ERROR 201
#define SERVER_SOCKET_CREATE_ERROR 300
#define SERVER_SOCKET_BIND_ERROR 301
#define SERVER_LISTEN_ERROR 302
#define SERVER_ACCEPT_ERROR 303
#define SERVER_NON_BLOCKING_ERROR 304
#define SERVER_RECEIVE_ZERO_ERROR 305

#endif
