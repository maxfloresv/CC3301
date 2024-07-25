#define _XOPEN_SOURCE 500

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "reservar.h"

// 0 si no hay personas estacionadas, 1 si hay alguien estacionado.
int est[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;

int cont_available = 10;
int ticket_dist = 0;
int display = 0;

void initReservar() {}

void cleanReservar() {}

int reservar(int k) {
  pthread_mutex_lock(&m);

  display++;
  pthread_cond_broadcast(&c);
  pthread_mutex_unlock(&m);
}

void liberar(int e, int k) {
  pthread_mutex_lock(&m);

  pthread_cond_broadcast(&c);
  pthread_mutex_unlock(&m);
} 