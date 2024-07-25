#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <pthread.h>

#include "maleta.h"

typedef struct {
  double *w;
  double *v;
  int *z;
  int n;
  double maxW;
  int k;
  double res;
} Args;

void *thread(void *ptr) {
  Args *pargs = (Args*) ptr;

  double *w = pargs -> w;
  double *v = pargs -> v;
  int *z = pargs -> z;
  int n = pargs -> n;
  double maxW = pargs -> maxW;
  int k = pargs -> k;

  double res = llenarMaletaSec(w, v, z, n, maxW, k / 8);
  pargs -> res = res;

  return NULL;
}

double llenarMaletaPar(double w[], double v[], int z[], int n, double maxW, int k) {
  pthread_t pid[8];
  Args args[8];

  for (int i = 0; i < 8; i++) {
    int *z_thread = (int *) malloc(n * sizeof(int));

    args[i].w = w;
    args[i].v = v;
    args[i].z = z_thread;
    args[i].n = n;
    args[i].maxW = maxW;
    args[i].k = k;

    pthread_create(&pid[i], NULL, thread, &args[i]);
  }

  double best_answer = -1;
  int best_thread = -1;

  for (int i = 0; i < 8; i++) {
    pthread_join(pid[i], NULL);

    if (args[i].res > best_answer) {
      best_answer = args[i].res;
      best_thread = i;
    }
  }

  int *z_solution = args[best_thread].z;
  for (int i = 0; i < n; i++) {
    z[i] = z_solution[i];
  }

  // Libero toda la memoria reservada
  for (int i = 0; i < 8; i++) {
    free(args[i].z);
  }

  return best_answer;
}
