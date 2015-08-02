/**
 * Copyright (c) 2014,2015 Enzien Audio, Ltd.
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

#include "ControlRandom.h"
#include "HvBase.h"

// http://www.firstpr.com.au/dsp/rand31
// http://en.wikipedia.org/wiki/Lehmer_random_number_generator

hv_size_t cRandom_init(ControlRandom *o, int seed) {
  o->state = (seed != 0) ? seed : 1;
  return 0;
}

void cRandom_onMessage(HvBase *_c, ControlRandom *o, int inletIndex, const HvMessage *m,
    void (*sendMessage)(HvBase *, int, const HvMessage *)) {
  switch (inletIndex) {
    case 0: {
      HvMessage *n = HV_MESSAGE_ON_STACK(1);
      o->state = (int) ((((unsigned long long) o->state) * 279470273UL) % 4294967291UL);
      float f = ((float) (o->state >> 9)) * 0.00000011920929f;
      msg_initWithFloat(n, msg_getTimestamp(m), f);
      sendMessage(_c, 0, n);
      break;
    }
    case 1: {
      if (msg_isFloat(m,0)) {
        o->state = (int) msg_getFloat(m,0);
      }
      break;
    }
    default: break;
  }
}
