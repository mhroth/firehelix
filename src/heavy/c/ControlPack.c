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

#include "ControlPack.h"

hv_size_t cPack_init(ControlPack *o, int n) {
  hv_size_t numBytes = msg_getByteSize(n);
  o->msg = (HvMessage *) hv_malloc(numBytes);
  msg_init(o->msg, n, 0);
  for (int i = 0; i < n; ++i) {
    msg_setFloat(o->msg, i, 0.0f);
  }
  return numBytes;
}

void cPack_free(ControlPack *o) {
  hv_free(o->msg);
}

void cPack_onMessage(HvBase *_c, ControlPack *o, int letIn, const HvMessage *const m,
    void (*sendMessage)(HvBase *, int, const HvMessage *const)) {
  // ensure let index is less than number elements in internal msg
  int numElements = msg_getNumElements(o->msg);
  switch (letIn) {
    case 0: { // first inlet stores all values of input msg and triggers an output
      for (int i = hv_min_i(numElements, msg_getNumElements(m))-1; i >= 0; --i) {
        switch (msg_getType(m, i)) {
          case FLOAT: msg_setFloat(o->msg, i, msg_getFloat(m, i)); break;
          case SYMBOL:
          case HASH: msg_setHash(o->msg, i, msg_getHash(m, i)); break;
          default: break;
        }
      }
      msg_setTimestamp(o->msg, msg_getTimestamp(m));
      sendMessage(_c, 0, o->msg);
      break;
    }
    default: { // rest of inlets just store values
      switch (msg_getType(m, 0)) {
        case FLOAT: msg_setFloat(o->msg, letIn, msg_getFloat(m, 0)); break;
        case SYMBOL:
        case HASH: msg_setHash(o->msg, letIn, msg_getHash(m, 0)); break;
        default: break;
      }
      break;
    }
  }
}
