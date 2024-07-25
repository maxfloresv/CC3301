#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include "elim.h"

int equalsPat(char *str, char *pat); 
size_t countAllocSize(char *str, char *pat);

void eliminar(char *str, char *pat) {
  int ls = strlen(str);
  int lp = strlen(pat);

  // Manejamos los casos borde para evitar problemas
  if (ls == 0 || lp == 0 || ls < lp)
    return;

  // Apunta al inicio del string
  char *start = str;
  // Esto apunta al último caracter '\0' del string
  char *end = str + ls;

  while (start <= end) {
    /*
      Si encontramos una coincidencia, no copiamos el resultado en el string
      modificado. En caso contrario, sí.
    */
    if (*start == *pat) {
      char *cpy_start = start;
      char *cpy_pat = pat;
      /*
        Necesitamos pasar copias para que no se modifiquen las direcciones
        de los punteros originales.
      */
      if (equalsPat(cpy_start, cpy_pat)) {
        // Nos movemos según el largo del patrón para ignorar esta parte
        start += lp;
      } else {
        // No era parte del patrón, entonces lo agregamos al string.
        *str = *start;
        str++;
        start++;
      }
    } else {
      // No hubo coincidencias con el patrón.
      *str = *start;
      str++;
      start++;
    }
  }
}
 
char *eliminados(char *str, char *pat) {
  int ls = strlen(str);
  int lp = strlen(pat);

  char *res;
  // Casos donde no se modifica el string
  if (ls == 0 || lp == 0 || ls < lp) {
    res = (char*) malloc((ls + 1) * sizeof(char));
    strcpy(res, str);
    return res;
  }
  
  char *cpy_str = str;
  char *cpy_pat = pat;
  
  res = (char*) malloc(countAllocSize(cpy_str, cpy_pat) * sizeof(char));

  char *aux = res;
  char *start = str;
  char *end = str + ls;

  while (start <= end) {
    if (*start == *pat) {
      char *cpy_start = start;
      char *cpy_pat = pat;
      if (equalsPat(cpy_start, cpy_pat)) {
        start += lp;
      } else {
        *aux = *start;
        aux++;
        start++;
      }
    } else {
      *aux = *start;
      aux++;
      start++;
    }
  }

  return res;
}

int equalsPat(char *str, char *pat) {
  int ans = 1;
  int len = strlen(pat);
  while (len--) {
    if (*str++ != *pat++) {
      ans = 0;
      break;
    }
  }

  return ans;
}

size_t countAllocSize(char *str, char *pat) {
  // Se le suma 1 por el '\0' del final
  size_t size = strlen(str) + 1;
  size_t lp = strlen(pat);

  while (*str != '\0') {
    if (*str == *pat) {
      char *cpy_str = str;
      char *cpy_pat = pat;
      if (equalsPat(cpy_str, cpy_pat)) {
        // Vamos restando las veces que aparece el patrón
        size -= lp;
        str += lp;
      } else {
        str++;
      }
    } else {
      str++;
    }
  }

  return size;
}
