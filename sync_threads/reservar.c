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

  int num = ticket_dist++;

  // Si no hay suficiente espacio o no es su turno, le toca esperar
  while (cont_available < k || num != display)
    pthread_cond_wait(&c, &m);

  int start = -1;
  // Rellenamos los primeros estacionamientos disponibles
  for (int i = 0; i < 10; i++) {
    if (est[i] == 0) {
      for (int j = i; j < i + k; j++) {
        if (est[j] == 1) 
          continue;
      }

      for (int j = i; j < i + k; j++) 
        est[j] = 1;

      start = i;
      break;
    }
  }

  // Dado que ya estacionó, actualizamos los estacionamientos disponibles
  int count = 0;
  cont_available = 0;
  for (int i = 0; i < 10; i++) {
    if (est[i] == 0) 
      count++;
    else 
      count = 0;
    
    if (cont_available < count) 
      cont_available = count;
  }

  display++;
  pthread_cond_broadcast(&c);
  pthread_mutex_unlock(&m);
  return start;    
}

void liberar(int e, int k) {
  pthread_mutex_lock(&m);

  // Liberamos los estacionamientos
  for (int i = e; i < e + k; i++)
    est[i] = 0;

  // Actualizamos los estacionamientos disponibles
  int count = 0;
  cont_available = 0;
  for (int i = 0; i < 10; i++) {
    if (est[i] == 0) 
      count++;
    else 
      count = 0;
    
    if (cont_available < count) 
      cont_available = count;
  }

  // Como se liberaron, debemos notificar nuevamente
  pthread_cond_broadcast(&c);
  pthread_mutex_unlock(&m);
} 