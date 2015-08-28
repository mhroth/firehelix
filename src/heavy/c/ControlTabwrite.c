/**
 * Copyright (c) 2015 Enzien Audio, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, and/or
 * sublicense copies of the Software, strictly on a non-commercial basis,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "ControlTabwrite.h"
#include "HvTable.h"

hv_size_t cTabwrite_init(ControlTabwrite *o, struct HvTable *table) {
  o->table = table;
  return 0;
}

void cTabwrite_onMessage(HvBase *_c, ControlTabwrite *o, int letIn, const HvMessage *const m,
    void (*sendMessage)(HvBase *, int, const HvMessage *const)) {
  switch (letIn) {
    case 0: {
      if (msg_isFloat(m,0) && o->table != NULL) {
        hTable_getBuffer(o->table)[o->x] = msg_getFloat(m,0); // update Y value
      }
      break;
    }
    case 1: {
      if (msg_isFloat(m,0) && o->table != NULL) {
        const int x = (int) msg_getFloat(m,0);
        if (x >= 0 && x < hTable_getSize(o->table)) {
          o->x = x; // update X value
        }
      }
      break;
    }
    case 2: {
      if (msg_isHashLike(m,0)) {
        o->table = ctx_getTableForHash(_c,msg_getHash(m,0)); // update table
      }
      break;
    }
    default: return;
  }
}
