#ifndef TIMING_H
#define TIMING_H

#include <stdio.h>
#include <stdlib.h>
//#include <string.h>  
//#include <unistd.h>  
//#include <fcntl.h>   
//#include <errno.h>   
//#include <termios.h> 
//#include <signal.h>
#include <sys/time.h>
#include <time.h>
//#include <sys/socket.h>
//#include <netdb.h>
//#include <netinet/in.h>
//#include <sys/types.h>
//#include <mysql/mysql.h>
#include <stdint.h>
//#include <pthread.h>

uint64_t millis();
uint64_t micros();
void delay(uint64_t millisec);
void hard_delay(uint64_t millisec);
void delayMicroseconds(uint64_t microsec);
void hard_delayMicroseconds(uint64_t microsec);

#endif
