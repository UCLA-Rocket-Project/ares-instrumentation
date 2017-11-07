#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <signal.h>
#include <sys/time.h>
#include <time.h>
//#include <sys/socket.h>
//#include <netdb.h>
//#include <netinet/in.h>
//#include <sys/types.h>
//#include <mysql/mysql.h>
#include <stdint.h>
#include <pthread.h>
#include <poll.h>

#include "sync_tools.h"

/* BEGIN DEFAULT VALUES DEFINITIONS */
#define N (1<<10)
#define M N-256
/* END DEFAULT VALUES DEFINITIONS */

int open_port(char* serial_path, int speed);
int wait_on_data(int fd, uint64_t millisec);
void imprint_current_time(char* s);
size_t parse(char* buffer, size_t buff_len);
str_node* add_str_node(str_node* str, char* line, double time);
void* obtain_data(void* arg);

#endif
