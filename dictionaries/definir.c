#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "pss.h"

typedef struct {
  char line[80];
  char newLine;
} Line;

int main(int argc, char *argv[]) {
  int line_size = sizeof(Line);
  char buffer[line_size + 1]; // line_size + \0

  // Hay menos o más argumentos de los pedidos
  if (argc != 4) {
    fprintf(stderr, "Uso: ./definir <diccionario> <llave> <definicion>\n");
    exit(1);
  }

  // El formato es [..., arch-dicc, llave, def]
  char *file_name = argv[1];
  char *key = argv[2];
  char *def = argv[3];

  FILE *file = fopen(file_name, "r+");
  // Si el archivo no existe, debemos retornar error
  if (file == NULL) {
    perror(file_name);
    exit(1);
  }

  fseek(file, 0, SEEK_END);
  int file_size = ftell(file);
  int n_lines = file_size / line_size;

  int curr_line = hash_string(key) % n_lines;
  int initial_line = curr_line;

  // Nos posicionamos en la línea del hash
  fseek(file, curr_line * line_size, SEEK_SET);

  while (1) {
    if (fread(buffer, 1, line_size, file) > 0) {
      char *ptr_line = buffer;
      // Si el primer caracter es un espacio, es una línea vacía
      if (*ptr_line == ' ') {
        fseek(file, -line_size, SEEK_CUR);
        fwrite(key, 1, strlen(key), file);
        fputc(':', file);
        fwrite(def, 1, strlen(def), file);
        break;
      } else {
        int len_key = 0;
        while (*ptr_line++ != ':')
            len_key++;

        if (strncmp(buffer, key, len_key) == 0) {
          fprintf(stderr, "La llave %s ya se encuentra en el diccionario\n", key);
          fclose(file);
          exit(1);
        }
      }
    }

    curr_line += 1;
    curr_line %= n_lines;

    if (curr_line == initial_line) {
      fprintf(stderr, "%s: el diccionario esta lleno\n", file_name);
      fclose(file);
      exit(1);
    }
  }

  fclose(file);
  return 0;
}
