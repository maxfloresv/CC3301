#include <stddef.h>

#include "prev.h"

void asignarPrev(Nodo *t, Nodo **pprev) {
  // Si el nodo es vacío, no me importa procesarlo
  if (t == NULL) {
    return;
  } else {
    // &previo hace que sea puntero a puntero, porque
    // previo solito es un puntero nomás
    asignarPrev(t->izq, pprev);

    Nodo *previo = *pprev;
    Nodo *sgte = NULL;

    t->prev = previo;
    t->prox = sgte;

    // No es *previo, porque previo ya es un puntero de por sí,
    // con *previo accedes al nodo
    if (previo != NULL) {
        (*pprev)->prox = t;
    }
    *pprev = t;

    // Lo mismo de arriba
    asignarPrev(t->der, pprev);
  }
}
