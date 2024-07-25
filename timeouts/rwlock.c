#define _XOPEN_SOURCE 500
#define TRUE 1
#define FALSE 0

#include "rwlock.h"

#include "nthread-impl.h"

struct rwlock {
  NthQueue *readers;
  NthQueue *writers;
  int nReaders;
  int isWriterWorking;
};

nRWLock *nMakeRWLock() {
  nRWLock *controller = (nRWLock *)malloc(sizeof(nRWLock));
  controller->readers = nth_makeQueue();
  controller->writers = nth_makeQueue();
  controller->nReaders = 0;
  controller->isWriterWorking = FALSE;

  return controller;
}

void nDestroyRWLock(nRWLock *rwl) {
  // nth_destroyQueue(rwl->readers);
  // nth_destroyQueue(rwl->writers);
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

void wakeUpFun(nThread nth) {
  NthQueue *writers = (NthQueue *)nth->ptr;
  nth_delQueue(writers, nth);
  nth->ptr = NULL;
}

int nEnterWrite(nRWLock *rwl, int timeout) {
  START_CRITICAL

  nThread thisTh = nSelf();
  /* Inicializamos todos con la cola, para que los que tienen timeout < 0 no
   * obtengan NULL. */
  // thisTh->ptr = rwl->writers;

  // Suspendemos el thread si hay lectores o hay un escritor trabajando.
  if ((rwl->nReaders > 0 || rwl->isWriterWorking) && timeout != 0) {
    nth_putBack(rwl->writers, thisTh);
    if (timeout < 0) {
      suspend(WAIT_RWLOCK);
    } else {
      thisTh->ptr = rwl->writers;
      suspend(WAIT_RWLOCK_TIMEOUT);
      nth_programTimer((long long)(timeout) * 1000000LL, wakeUpFun);
    }
    schedule();

    END_CRITICAL

    return timeout > 0 && thisTh->ptr == NULL ? 0 : 1;
  } else {
    rwl->isWriterWorking = TRUE;
  }

  END_CRITICAL
  return 1;
}

void nExitRead(nRWLock *rwl) {
  START_CRITICAL

  rwl->nReaders--;

  // Para aceptar al escritor que lleva esperando más tiempo, debemos asegurar
  // que exista uno.
  if (rwl->nReaders == 0 && !nth_emptyQueue(rwl->writers)) {
    nThread th = nth_getFront(rwl->writers);

    if (th->status == WAIT_RWLOCK_TIMEOUT) {
      nth_cancelThread(th);
    }

    rwl->isWriterWorking = TRUE;
    setReady(th);
  }

  schedule();
  END_CRITICAL
}

void nExitWrite(nRWLock *rwl) {
  START_CRITICAL

  rwl->isWriterWorking = FALSE;

  // Si no hay lectores pero sí escritores pendientes, aceptamos al que lleva
  // más esperando. En caso contrario, aceptamos a todos los lectores hasta que
  // no queden más.
  if (nth_emptyQueue(rwl->readers) && !nth_emptyQueue(rwl->writers)) {
    nThread th = nth_getFront(rwl->writers);

    if (th->status == WAIT_RWLOCK_TIMEOUT) {
      nth_cancelThread(th);
    }

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
