#ifndef DATA_H
#define DATA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
//#include <unistd.h>  
//#include <fcntl.h>   
//#include <errno.h>   
//#include <termios.h> 
#include <signal.h>
//#include <sys/time.h>
//#include <time.h>
#include <sys/socket.h>
//#include <netdb.h>
//#include <netinet/in.h>
#include <sys/types.h>
#if USE_MYSQL==1
#include <mysql/mysql.h>
//#include <mysql.h>
#endif
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#include "sync_tools.h"

#define MAX_LOG_N 256

FILE* open_new_log(char* path_to_log_dir);
void free_str_nodes(str_node* str);
void handle_mysql_str_list(str_node** head_ptr);
void handle_log_str_list(str_node** head_ptr);
void* handle_mysql_data(void* arg);
void* handle_log_data(void* arg);
void connect_to_mysql(int init);
#endif
