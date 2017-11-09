#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

struct Array {
  int32_t length;
  int8_t* buf;
};

struct Array Array_new(int n) {
  struct Array t;
  t.length = n;
  t.buf = malloc(n);
  return t;
}

int8_t Array_get(struct Array *this, int i) {
  return this->buf[i];
}

int main() {
  struct Array a = Array_new(8);
  printf("Hello\n");
}
