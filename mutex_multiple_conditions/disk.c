#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "disk.h"
#include "pss.h"

typedef struct {
  int ready;
  pthread_cond_t c;
} Request;

PriQueue *less_pq, *geq_pq;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

void iniDisk(void) {
  less_pq = makePriQueue();
  geq_pq = makePriQueue();
}

void cleanDisk(void) {
  destroyPriQueue(less_pq);
  destroyPriQueue(geq_pq);
}

void requestDisk(int track) {
  pthread_mutex_lock(&m);
  Request req = {0, PTHREAD_COND_INITIALIZER};

  // Este es un caso borde cuando entra una pista y la cola está vacía.
  if (emptyPriQueue(geq_pq) && emptyPriQueue(less_pq)) 
    req.ready = 1;

  /**
   * Aquí, priBest(...) siempre será la pista que solicitó acceso al disco, pues
   * es la menor entre las que cumplen t' >= t. Sino, asignamos a las pistas que
   * cumplen t' < t (para particionar la muestra).
   */ 

  if (emptyPriQueue(geq_pq) || track >= priBest(geq_pq)) 
    priPut(geq_pq, &req, track);
  else 
    priPut(less_pq, &req, track);

  while (!req.ready) 
    pthread_cond_wait(&req.c, &m);

  pthread_mutex_unlock(&m);
}

void releaseDisk() {
  pthread_mutex_lock(&m);

  // Lo saco de la cola de prioridad >= porque ya lo procesamos.
  if (!emptyPriQueue(geq_pq))
    priGet(geq_pq);

  // Obtenemos el que sigue y cumple t' >= t.
  Request *req = priPeek(geq_pq);

  // Si no existe, y tampoco existen t' tales que t' < t, se termina.
  if (req == NULL && emptyPriQueue(less_pq)) {
    pthread_mutex_unlock(&m);
    return;
  }

  // En este punto, sí existen t' tales que t' < t, entonces seleccionamos el más cercano al centro.
  if (req == NULL) {
    // Como sabemos que en less_pq está el más cercano al centro, y cuando estemos leyendo los nuevos que
    // cumplen t' >= t pueden aparecer menores al más cercano en este punto, intercambiamos las colas.
    while (!emptyPriQueue(less_pq)) {
      int less_track = priBest(less_pq);
      Request *less_req = priGet(less_pq);
      priPut(geq_pq, less_req, less_track);
    }

    req = priPeek(geq_pq);
  }
  
  req->ready = 1;
  pthread_cond_signal(&(req->c));
  pthread_mutex_unlock(&m);
}
