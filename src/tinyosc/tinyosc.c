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
  memcpy(o->address, buffer, i);

  // extract the format
  while (i < len && buffer[i] != ',') ++i; // find the format comma
  ++i; // skip the comma
  int j = i;
  for (; j < len && buffer[j] != '\0'; ++j); // find the end of the format
  ++j;  // include the '\0'
  memcpy(o->format, buffer+i, (j-i));
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
  o->marker += 4;
  return *((float *) (&i));
}

int tosc_getNextString(tosc_tinyosc *o, char *buffer, int len) {
  // TODO(mhroth)
  return -1;
}
