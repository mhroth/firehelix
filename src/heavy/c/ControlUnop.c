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

#include "ControlUnop.h"
#include "HvBase.h"

void cUnop_onMessage(HvBase *_c, UnopType op, const HvMessage *const m,
    void (*sendMessage)(HvBase *, int, const HvMessage *const )) {
  if (msg_isFloat(m, 0)) {
    float f = msg_getFloat(m, 0);
    switch (op) {
      case HV_UNOP_SIN: f = hv_sin_f(f); break;
      case HV_UNOP_SINH: f = hv_sinh_f(f); break;
      case HV_UNOP_COS: f = hv_cos_f(f); break;
      case HV_UNOP_COSH: f = hv_cosh_f(f); break;
      case HV_UNOP_TAN: f = hv_tan_f(f); break;
      case HV_UNOP_TANH: f = hv_tanh_f(f); break;
      case HV_UNOP_ASIN: f = hv_asin_f(f); break;
      case HV_UNOP_ASINH: f = hv_asinh_f(f); break;
      case HV_UNOP_ACOS: f = hv_acos_f(f); break;
      case HV_UNOP_ACOSH: f = hv_acosh_f(f); break;
      case HV_UNOP_ATAN: f = hv_atan_f(f); break;
      case HV_UNOP_ATANH: f = hv_atanh_f(f); break;
      case HV_UNOP_EXP: f = hv_exp_f(f); break;
      case HV_UNOP_ABS: f = hv_abs_f(f); break;
      case HV_UNOP_SQRT: f = (f > 0.0f) ? hv_sqrt_f(f) : 0.0f; break;
      case HV_UNOP_LOG: f = (f > 0.0f) ? hv_log_f(f) : 0.0f; break;
      case HV_UNOP_LOG2: f = (f > 0.0f) ? hv_log2_f(f) : 0.0f; break;
      case HV_UNOP_LOG10: f = (f > 0.0f) ? hv_log10_f(f) : 0.0f; break;
      case HV_UNOP_CEIL: f = hv_ceil_f(f); break;
      case HV_UNOP_FLOOR: f = hv_floor_f(f); break;
      case HV_UNOP_ROUND: f = hv_round_f(f); break;
      default: return;
    }
    HvMessage *n = HV_MESSAGE_ON_STACK(1);
    msg_initWithFloat(n, m->timestamp, f);
    sendMessage(_c, 0, n);
  }
}
