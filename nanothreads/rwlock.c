#define _XOPEN_SOURCE 500
#define TRUE 1
#define FALSE 0

#include "nthread-impl.h"

#include "rwlock.h"

struct rwlock {
  NthQueue *readers;
  NthQueue *writers;
  int nReaders;
  int isWriterWorking;
};

nRWLock *nMakeRWLock() {
  nRWLock *controller = (nRWLock*) malloc(sizeof(nRWLock));
  controller->readers = nth_makeQueue();
  controller->writers = nth_makeQueue();
  controller->nReaders = 0;
  controller->isWriterWorking = FALSE;

  return controller;
}

void nDestroyRWLock(nRWLock *rwl) {
  //nth_destroyQueue(rwl->readers);
  //nth_destroyQueue(rwl->writers);
  free(rwl);
}

int nEnterRead(nRWLock *rwl, int timeout) {
  START_CRITICAL

  nThread thisTh = nSelf();

  // Si hay un escritor trabajando o escritores pendientes, queda en espera.
  if (!nth_emptyQueue(rwl->writers) || rwl->isWriterWorking) {
    nth_putBack(rwl->readers, thisTh);
    suspend(WAIT_RWLOCK);
    schedule();
  } else {
    rwl->nReaders++;
  }

  END_CRITICAL
  return 1;
}

int nEnterWrite(nRWLock *rwl, int timeout) {
  START_CRITICAL
  
  nThread thisTh = nSelf();

  // Suspendemos el thread si hay lectores o hay un escritor trabajando.
  if (rwl->nReaders > 0 || rwl->isWriterWorking) {
    nth_putBack(rwl->writers, thisTh);
    suspend(WAIT_RWLOCK);
    schedule();
  } else {
    rwl->isWriterWorking = TRUE;
  }

  END_CRITICAL
  return 1;
}

void nExitRead(nRWLock *rwl) {
  START_CRITICAL

  rwl->nReaders--;

  // Para aceptar al escritor que lleva esperando más tiempo, debemos asegurar que exista uno.
  if (rwl->nReaders == 0 && !nth_emptyQueue(rwl->writers)) {
    nThread th = nth_getFront(rwl->writers);
    rwl->isWriterWorking = TRUE;
    setReady(th);
  }

  schedule();
  END_CRITICAL
}

void nExitWrite(nRWLock *rwl) {
  START_CRITICAL

  rwl->isWriterWorking = FALSE;

  // Si no hay lectores pero sí escritores pendientes, aceptamos al que lleva más esperando.
  // En caso contrario, aceptamos a todos los lectores hasta que no queden más.
  if (nth_emptyQueue(rwl->readers) && !nth_emptyQueue(rwl->writers)) {
    nThread th = nth_getFront(rwl->writers);
    rwl->isWriterWorking = TRUE;
    setReady(th);
  } else if (!nth_emptyQueue(rwl->readers)) {
    while (!nth_emptyQueue(rwl->readers)) {
      nThread th = nth_getFront(rwl->readers);
      rwl->nReaders++;
      setReady(th);
    }
  }

  schedule();
  END_CRITICAL
}
