#include <stdio.h>

#include "suma.h"

/*
  Máximo Flores Valenzuela
  RUT: 21.123.385-0
  Tarea 1 CC3301-1
*/

Bcd sumaBcd(Bcd x, Bcd y) {
  Bcd mask = 0xf, ans = 0, carry = 0;

  for (int i = 0; i < 16; i++) {
    Bcd x_masked = x & mask;
    Bcd y_masked = y & mask;

    // Debemos correr a la posición de la unidad
    x_masked >>= (i << 2); 
    y_masked >>= (i << 2);

    Bcd sum = x_masked + y_masked + carry;
    // Reiniciamos el carry en cada iteración
    carry = 0x0;

    // Tratamos el caso donde existe una reserva
    if (sum > 9) {
      sum -= 10;
      carry = 1;
    }

    // Para poder posicionar el bit en la respuesta
    sum <<= (i << 2);
    ans |= sum;
    // Actualizamos la máscara
    mask <<= 4;
  }

  // Si en la última iteración quedó reserva, entonces
  // corresponde al caso borde señalado.
  if (carry)
    ans = 0xffffffffffffffff;

  return ans;
}
