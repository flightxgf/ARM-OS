#include "io.h"

int log_n(int x, int base){
  int L = 0;
  for (int I = 1; I <= 0xFFFFFF; I*=base){
    if (x / I == 0){
      break;
    }
    L++;
  }
  if (L == 0){
    return 1;
  }
  return L;
}

void int_to_string(int x, char str[11]){
  int length = log_n(x, 10);
  int big = 1;
  int pos = 0;
  for (int I = length - 1; I >= 0; I--){
    str[I] = itox((x / big) % 10);
    big *=10;
  }
  str[length] = '\0';
}

void hex_to_string(int x, char str[11]){
  int length = log_n(x, 16);
  int big = 1;
  int pos = 0;
  for (int I = length - 1; I >= 0; I--){
    str[I] = itox((x / big) % 16);
    big *=16;
  }
  str[length] = '\0';
}

void working_printf(char* str, ...){
  va_list list;
  va_start(list, str);
  int I = 0;
  int J = 0;
  while (str[I] != '\0'){
    if (str[I] == '%'){
      _write(STDOUT_FILENO, &(str[J]), I - J);
      J = I+2;
      I++;
      if (str[I] == 'c'){
        int c = va_arg(list, int);
        _write(STDOUT_FILENO, &c, 1);
      }
      if (str[I] == 'd'){
        int i = va_arg(list, int);
        char str[11];
        int_to_string(i, str);
        _write(STDOUT_FILENO, &str, log_n(i, 10));
      }
      if (str[I] == 'o'){
        int h = va_arg(list, int);
        char str[11];
        hex_to_string(h, str);
        working_printf("0x");
        _write(STDOUT_FILENO, &str, log_n(h, 16));
      }
      if (str[I] == 's'){
        char *s = va_arg(list, char*);
        working_printf(s);
      }
    }
    I++;
  }
  _write(STDOUT_FILENO, &(str[J]), I - J);
  va_end(list);
}

void get_input(char *str, int n){
  for (int I = 0; I < n; I++){
    _read(STDIN_FILENO, &(str[I]), 1);
    if (str[I] == '\r'){
      str[I] = '\0';
      break;
    }
  }
}