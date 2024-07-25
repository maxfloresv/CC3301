#include "bolsa.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pss.h"
#include "spinlocks.h"

enum { FALSE, TRUE };
/* Possible status */
enum { WAITING_STATUS, ADJUDICATED, REJECTED };

int mutex_sl = OPEN;
/* Contains the memory direction of the waiting seller spinlock. */
int *seller_sl = NULL;
int cheapestPrice = 0;

/* Memory direction of the best seller. */
char *seller;
/* Memory direction where we have to type in the buyer. */
char *buyer;
/* Contains the status of the "previous" seller (the one that was waiting). */
int *status = NULL;

int vendo(int precio, char *vendedor, char *comprador) {
  spinLock(&mutex_sl);

  int VL = WAITING_STATUS;

  if (cheapestPrice == 0) {
    cheapestPrice = precio;
    seller = vendedor;
    buyer = comprador;
    status = &VL;
  } else if (precio <= cheapestPrice) {
    cheapestPrice = precio;
    seller = vendedor;
    buyer = comprador;
    *status = REJECTED;
    status = &VL;
    spinUnlock(seller_sl);
  } else {
    /* If it's already greater or equal, we reject inmediately. */
    spinUnlock(&mutex_sl);
    return FALSE;
  }

  int w = CLOSED;
  seller_sl = &w;

  spinUnlock(&mutex_sl);
  spinLock(&w);

  /* The unique way to get to this point is by being unlocked by ADJUD or
   * REJECT because w locks execution. */
  return VL == ADJUDICATED;
}

int compro(char *comprador, char *vendedor) {
  spinLock(&mutex_sl);
  int cheapest = cheapestPrice;

  if (cheapest > 0) {
    strcpy(buyer, comprador);
    strcpy(vendedor, seller);
    *status = ADJUDICATED;
    cheapestPrice = 0;
    spinUnlock(seller_sl);
  }

  spinUnlock(&mutex_sl);
  return cheapest;
}
