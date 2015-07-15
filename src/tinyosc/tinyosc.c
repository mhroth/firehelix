/**
 * Copyright (c) 2015, Martin Roth (mhroth@gmail.com)
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
Received: [8] /2
2f|32|0|0|2c|0|0|0|
Received: [20] /2/push1
2f|32|2f|70|75|73|68|31|0|0|0|0|2c|66|0|0|3f|80|0|0|
Received: [20] /2/push1
2f|32|2f|70|75|73|68|31|0|0|0|0|2c|66|0|0|0|0|0|0|
Received: [20] /2/push13
2f|32|2f|70|75|73|68|31|33|0|0|0|2c|66|0|0|3f|80|0|0|
Received: [20] /2/push13
2f|32|2f|70|75|73|68|31|33|0|0|0|2c|66|0|0|0|0|0|0|
Received: [8] /3
2f|33|0|0|2c|0|0|0|
Received: [20] /3/xy
2f|33|2f|78|79|0|0|0|2c|66|66|0|3e|98|73|e|3e|7b|34|bc|
Received: [20] /3/xy
2f|33|2f|78|79|0|0|0|2c|66|66|0|3e|a5|df|67|3e|93|a7|ca|
Received: [20] /3/xy
2f|33|2f|78|79|0|0|0|2c|66|66|0|3e|ad|8b|7|3e|9f|29|3a|
Received: [20] /3/xy
2f|33|2f|78|79|0|0|0|2c|66|66|0|3e|b5|36|a7|3e|a9|b5|37|
Received: [20] /3/xy
2f|33|2f|78|79|0|0|0|2c|66|66|0|3e|bb|ec|d3|3e|b4|41|33|
Received: [20] /3/xy
2f|33|2f|78|79|0|0|0|2c|66|66|0|3e|c3|98|73|3e|be|cd|2f|
Received: [20] /3/xy
2f|33|2f|78|79|0|0|0|2c|66|66|0|3e|ca|4e|9f|3e|cb|44|13|
Received: [20] /3/xy
2f|33|2f|78|79|0|0|0|2c|66|66|0|3e|d1|4|cb|3e|d5|d0|f|
Received: [20] /3/xy
2f|33|2f|78|79|0|0|0|2c|66|66|0|3e|d6|c5|83|3e|dd|7b|af|
*/

#include <netinet/in.h>
#include <string.h>
#include "tinyosc.h"

int tosc_init(tosc_tinyosc *o, const char *buffer, const int len) {
  o->buffer = buffer;
  o->len = len;

  // extract the address
  int i = 0;
  while (i < len && buffer[i] != '\0') ++i;
  ++i; // address string includes trailing '\0'
  if (i >= TOSC_MAX_LEN_ADDRESS) return -1;
  strncpy(o->address, buffer, i);

  // extract the format
  while (i < len && buffer[i] != ',') ++i; // find the format comma
  ++i; // skip the comma
  int j = i;
  for (; j < len && buffer[j] != '\0'; ++j); // find the end of the format
  o->size = (j-i);
  strncpy(o->format, buffer+i, o->size);
  o->format[o->size] = '\0';
  ++j;  // include the '\0'
  if ((j-i) >= TOSC_MAX_LEN_FORMAT) return -2;

  while (j & 0x3) ++j; // advance to the next multiple of 4
  o->marker = buffer + j;

  return 0;
}

int32_t tosc_getNextInt32(tosc_tinyosc *o) {
  const int32_t i = (int32_t) ntohl(*((uint32_t *) o->marker)); // convert from big-endian
  o->marker += 4;
  return i;
}

float tosc_getNextFloat(tosc_tinyosc *o) {
  const uint32_t i = ntohl(*((uint32_t *) o->marker)); // convert from big-endian
  const float f = *((float *) (&i));
  o->marker += 4;
  return f;
}

int tosc_getNextString(tosc_tinyosc *o, char *buffer, int len) {
  // TODO(mhroth)
  return -1;
}
