#include <string.h>

int strWords(char *s) {
    int res = 0;
    char c = *s;
    
    // Contamos las palabras de s
    for (;;) {
      // Llegamos al final del string
      if (c == '\0')
        break;

      // Si es un espacio, lo ignoramos
      if (c == ' ') {
        s++;
        c = *s;
        continue;
      }

      // Procesamos una palabra encontrada
      for (;;) {
        // Si encontramos un espacio o llegamos al final, salimos del ciclo
        if (c == ' ' || c == '\0')
          break;

        s++;
        c = *s;
      }

      res++;
    }

    return res;
}

void sort(char **a, int n) {
  char **ult = &a[n-1];
  char **p = a;
  while (p < ult) {
    int l1 = strWords(p[0]);
    int l2 = strWords(p[1]);

    /**
     * Estan ordenados por palabras. La condición ahora es menor o igual
     * porque pueden haber palabras con el mismo largo, en dicho caso, <
     * no termina jamás el programa.
     */
    if (l1 <= l2) {
      p++;
    } else {
      char *tmp = p[0];
      p[0] = p[1];
      p[1] = tmp;
      // Volvemos al inicio del arreglo
      p = a;
    }
  }
}
