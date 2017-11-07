#include "timing.h"

uint64_t millis() {
  struct timeval tv;  
  gettimeofday(&tv, NULL);
  return ((uint64_t)tv.tv_sec * 1000LLU) + ((uint64_t)tv.tv_usec) / 1000LLU;
}

uint64_t micros() {
  struct timeval tv;  
  gettimeofday(&tv, NULL);
  return (((uint64_t)tv.tv_sec) * 1000000LLU) + ((uint64_t)tv.tv_usec);
}

void delay(uint64_t millisec) {
  struct timespec tv;
  tv.tv_sec = millisec / 1000LLU;
  tv.tv_nsec = (millisec % 1000LLU) * 1000000LLU;
  nanosleep(&tv, NULL);
}

void delayMicroseconds(uint64_t microsec) {
  struct timespec tv;
  tv.tv_sec = microsec / 1000000LLU;
  tv.tv_nsec = (microsec % 1000000LLU) * 1000LLU;
  nanosleep(&tv, NULL);
}

void hard_delay(uint64_t millisec) {
  uint64_t init_time = millis(); 
  while (millis() - init_time < millisec) {
    asm("nop");
  }
}

void hard_delayMicroseconds(uint64_t microsec) {
  uint64_t init_time = micros(); 
  while (micros() - init_time < microsec) {
    asm("nop");
  }
}
