
// To compile a library:
// clang unicope.c -c -ounicope.lib

#define uc_implementation
#include "unicope.h"

#if defined(TEST)
#include <stdio.h>
#include <assert.h>

int main() {
  char16_t buf[4] = {0xD801, 0xDC37};
  size_t  buf_len = sizeof buf;
  char16_t *ptr = buf;
  uint32_t c;
  int len = utf16_cdec(buf_len, &ptr, &c);
  return 0;
}
#endif
