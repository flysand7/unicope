
// See bottom of the file for license

#pragma once

#include <stdint.h>
#include <stddef.h>

#if defined(uc_single_header)
  #define uc_func static
#else
  #define uc_func
#endif

//---------------------------------------------------------------------------//

#if !defined(uc_char8_type)
  typedef unsigned char char8_t;
#else
  typedef uc_char8_type char8_t;
#endif

typedef uint_least16_t char16_t;
typedef uint_least32_t char32_t;

// Decode a single Unicode character from a UTF-8 string
// INPUT:
// - len: length of a UTF-8 string, or -1 if the string is NUL-terminated. If
//   len is zero, no character is decoded and 0 is returned.
// - utf8: pointer to a string. After successfull decoding, the string is
//   string is advanced by the number of bytes decoded. If decoding
//   results in an error the contents are unspecified.
// OUTPUT:
// - *utf8: if decoded successfully, this is a pointer to the next character.
//   otherwise the value of the pointer is unspecified.
// - *c: if decoded successfull, this is a decoded character. Otherwise the
//   value is unspecified.
// - Return Value:
//   If a decoding error occurs, the return value is negative
//   if a NUL character was decoded, the return value is 0
//   Otherwise returns the number of bytes that were read
uc_func int utf8_cdec(size_t len, char8_t **utf8, char32_t *c);

// Fast decode a single Unicode character from UTF-8 string. This function
// assumes that 4 first bytes are readable.
// INPUT:
// - utf8: pointer to a string to decode from
// OUTPUT:
// - *utf8: if decoded successfully, it is pointer to the next character.
//   otherwise the value of this pointer is unspecified.
// - *c: if decoded successfully, holds a value of decoded character.
//   otherwise the value is unspecified
// - Return Value:
//   If a decoding error occurs, it's negative
//   If a NUL character was decoded, the return value is 0
//   Otherwise returns the number of bytes read
uc_func int utf8_cdecf(char8_t **utf8, char32_t *c);

// Encode a single Unicode character and write it into a string
// INPUT:
// - utf8: pointer to a string to decode from
// OUTPUT:
// - *utf8: string, advanced by number of bytes that got written
// - Return value: number of bytes written, or negative value of encoding
//   failed
uc_func int utf8_cenc(size_t len, char8_t **utf8, char32_t c);

// Decode a single Unicode character from UTF-16 string
// INPUT:
// - len: length of the input UTF-16 string
// - utf16: pointer to UTF-16 string
// OUTPUT:
// - c: decoded character
// - utf16: pointer to UTF-16 string after the decoded character
// - Return value: negative value, if decoding error occurs. Zero if the
//   decoded character is NUL, otherwise returns number of code units that
//   got read.
uc_func int utf16_cdec(size_t len, char16_t **utf16, char32_t *c);

//---------------------------------------------------------------------------//

#if defined(uc_implementation)

const int uc__utf8_unit_class[] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0,
};

const char32_t uc__utf8_cp_mins[] = {0x400000, 0, 0x80, 0x800, 0x10000};

uc_func int utf8_cdec(size_t size, char8_t **ptr, char32_t *cp) {
  char8_t *utf8 = *ptr;
  int len = uc__utf8_unit_class[*utf8>>3];
  char32_t c;
  int res;
  if(len == 0)  return -1;
  if(size >= 0 && len > size) return -1;
  res = len;
  if (len == 1) {
    c = utf8[0];
  } else if (len == 2) {
    uint32_t byte1 = utf8[0] & 0x1f;
    uint32_t byte2 = utf8[1] & 0x3f;
    c = (byte1 << 6) | byte2;
  } else if (len == 3) {
    uint32_t byte1 = utf8[0] & 0x0f;
    uint32_t byte2 = utf8[1] & 0x3f;
    uint32_t byte3 = utf8[2] & 0x3f;
    c = (byte1 << 12) | (byte2 << 6) | byte3;
  } else if (len == 4) {
    uint32_t byte1 = utf8[0] & 0x07;
    uint32_t byte2 = utf8[1] & 0x3f;
    uint32_t byte3 = utf8[2] & 0x3f;
    uint32_t byte4 = utf8[3] & 0x3f;
    c = (byte1 << 18) | (byte2 << 12) | (byte3 << 6) | byte4;
  } else {
    res = -1;
  }
  if(c == 0) res = 0;
  if(c >= 0xd800 && c <= 0xdfff) res = -1;
  if(c >= 0x110000) res = -1;
  if(c < uc__utf8_cp_mins[len]) res = -1;
  *cp = c;
  *ptr = utf8 + len;
  return res;
}

uc_func int utf8_cdecf(char8_t **ptr, char32_t *cp) {
  char8_t *utf8 = *ptr;
  int len = uc__utf8_unit_class[*utf8>>3];
  char32_t c;
  // Details:
  // https://nullprogram.com/blog/2017/10/06/
  // Decode the character
  const int shifts[] = {0, 18, 12, 6, 0};
  const int masks[] = {0x00, 0x7f, 0x1f, 0x0f, 0x07};
  c  = (char32_t)(utf8[0] & masks[len]) << 18;
  c |= (char32_t)(utf8[1] & 0x3f) << 12;
  c |= (char32_t)(utf8[2] & 0x3f) <<  6;
  c |= (char32_t)(utf8[3] & 0x3f) <<  0;
  c >>= shifts[len];
  // Error handling
  const int shifte[] = {0, 6, 4, 2, 0};
  int res;
  res  = (c < uc__utf8_cp_mins[len]) << 6; // Overlong sequences
  res |= ((c >> 11) == 0x1b) << 7; // Surrogate codepoint
  res |= (utf8[1] & 0xc0) >> 2;
  res |= (utf8[2] & 0xc0) >> 4;
  res |= (utf8[3]) >> 6;
  res ^= 0x2a; // Use XOR to check the high bits of continuation bytes
  res >>= shifte[len];
  res = -res; // Negate the error to return it (error of 0 will be unchanged)
  res |= len; // If there's an error, the value stays negative, else its len
  *cp = c;
  *ptr = utf8 + len;
  if(res>0 && c == 0) res = 0;
  return res;
}

uc_func int utf8_cenc(size_t size, char8_t **strp, char32_t c) {
  if(size == 0) return -1;
  char8_t *utf8 = *strp;
  int len = 0;
  if(c < 0x80) {
    len = 1;
    if(len > size) return -1;
    *utf8++ = (char8_t)c;
  }
  else if(c < 0x800) {
    len = 2;
    if(len > size) return -1;
    *utf8++ = 0xc0 | (char8_t)(c >> 6);
    *utf8++ = 0x80 | (char8_t)(c & 0x3f);
  }
  else if(c < 0x10000) {
    if(c <= 0xd800 && c < 0xe000) {
      return -1;
    }
    len = 3;
    if(len > size) return -1;
    *utf8++ = 0xe0 | (char8_t)(c >> 12);
    *utf8++ = 0x80 | (char8_t)((c >> 6) & 0x3f);
    *utf8++ = 0x80 | (char8_t)(c & 0x3f);
  }
  else if(c < 0x110000) {
    len = 4;
    if(len > size) return -1;
    *utf8++ = 0xe0 | (char8_t)(c >> 18);
    *utf8++ = 0x80 | (char8_t)((c >> 12) & 0x3f);
    *utf8++ = 0x80 | (char8_t)((c >> 6) & 0x3f);
    *utf8++ = 0x80 | (char8_t)(c & 0x3f);
  }
  else {
    len = -1;
  }
  *strp = utf8;
  return len;
}

uc_func int utf16_cdec(size_t size, char16_t **p, char32_t *c) {
  if(size == 0) return -1;
  char16_t *utf16 = *p;
  if((utf16[0] & 0xfc00) == 0xd800) {
    if(size < 2) return -1;
    if((utf16[1] & 0xfc00) != 0xdc00) {
      return -1;
    }
    *c = (((utf16[0] & 0x3ff) << 10) | (utf16[1] & 0x3ff)) + 0x10000;
    *p = utf16 + 2;
    return 2;
  }
  else if((utf16[0] & 0xfc00) == 0xd800) {
    return -1;
  }
  else {
    *c = utf16[0];
    *p = utf16+1;
    return 1;
  }
}

#endif // uc_implementation


/*-----------------------------------------------------------------------------
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
-----------------------------------------------------------------------------*/
