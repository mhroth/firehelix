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

#ifndef _HEAVY_CONTROL_UNOP_H_
#define _HEAVY_CONTROL_UNOP_H_

struct HvBase;
struct HvMessage;

typedef enum UnopType {
  HV_UNOP_ASIN,
  HV_UNOP_ASINH,
  HV_UNOP_ACOS,
  HV_UNOP_ACOSH,
  HV_UNOP_ATAN,
  HV_UNOP_ATANH,
  HV_UNOP_SIN,
  HV_UNOP_SINH,
  HV_UNOP_COS,
  HV_UNOP_COSH,
  HV_UNOP_TAN,
  HV_UNOP_TANH,
  HV_UNOP_EXP,
  HV_UNOP_ABS,
  HV_UNOP_SQRT,
  HV_UNOP_LOG,
  HV_UNOP_LOG2,
  HV_UNOP_LOG10,
  HV_UNOP_CEIL,
  HV_UNOP_FLOOR,
  HV_UNOP_ROUND
} UnopType;

void cUnop_onMessage(struct HvBase *_c, UnopType op, const struct HvMessage *const m,
    void (*sendMessage)(struct HvBase *, int, const struct HvMessage *const));

#endif // _HEAVY_CONTROL_UNOP_H_
