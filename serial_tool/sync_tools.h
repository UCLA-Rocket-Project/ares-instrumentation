#ifndef SYNC_TOOLS_H
#define SYNC_TOOLS_H

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>

extern pthread_mutex_t atomic64;
extern pthread_mutex_t atomic32;

int64_t atomicRead64(int64_t* i);
void atomicWrite64(int64_t* i, int64_t v);
int64_t atomicAdd64(int64_t* i, int64_t v);
void atomicSafeUnlock64();

int32_t atomicRead32(int32_t* i);
void atomicWrite32(int32_t* i, int32_t v);
int32_t atomicAdd32(int32_t* i, int32_t v);
void atomicSafeUnlock32();

#endif
