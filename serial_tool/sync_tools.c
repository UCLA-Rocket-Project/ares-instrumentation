#include "sync_tools.h"

pthread_mutex_t atomic64;
pthread_mutex_t atomic32;

int64_t atomicRead64(int64_t* i) {
  pthread_mutex_lock(&atomic64);
  int64_t r = *i;
  pthread_mutex_unlock(&atomic64);
  return r;
}

void atomicWrite64(int64_t* i, int64_t v) {
  pthread_mutex_lock(&atomic64);
  *i = v;
  pthread_mutex_unlock(&atomic64);
  return;
}

int64_t atomicAdd64(int64_t* i, int64_t v) {
  pthread_mutex_lock(&atomic64);
  *i += v;
  int64_t r = *i;
  pthread_mutex_unlock(&atomic64);
  return r;
}

void atomicSafeUnlock64() {
  if (pthread_mutex_trylock(&atomic64) > 0)
    pthread_mutex_unlock(&atomic64);
}

int32_t atomicRead32(int32_t* i) {
  pthread_mutex_lock(&atomic32);
  int64_t r = *i;
  pthread_mutex_unlock(&atomic32);
  return r;
}

void atomicWrite32(int32_t* i, int32_t v) {
  pthread_mutex_lock(&atomic32);
  *i = v;
  pthread_mutex_unlock(&atomic32);
  return;
}

int32_t atomicAdd32(int32_t* i, int32_t v) {
  pthread_mutex_lock(&atomic32);
  *i += v;
  int32_t r = *i;
  pthread_mutex_unlock(&atomic32);
  return r;
}

void atomicSafeUnlock32() {
  if (pthread_mutex_trylock(&atomic32) > 0)
    pthread_mutex_unlock(&atomic32);
}
