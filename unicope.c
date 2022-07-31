
// To compile a library:
// clang unicope.c -c -ounicope.lib

#define uc_implementation
#include "unicope.h"

#if defined(TEST)
#include <stdio.h>
#include <assert.h>

int main() {
  char8_t buf[4] = {0x80, 0xc0};
  size_t  buf_len = sizeof buf;
  char8_t *ptr = buf;
  int len = utf8_cenc(buf_len, &ptr, 0x0440);
  return 0;
}
#endif
