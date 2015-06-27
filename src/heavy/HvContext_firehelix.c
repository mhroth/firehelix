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
 *
 * DO NOT MODIFY. THIS CODE IS MACHINE GENERATED BY THE SECTION6 HEAVY COMPILER.
 */

/*
 * System Includes
 */

#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include "HvContext_firehelix.h"
#include "HeavyMath.h"


/*
 * Function Declarations
 */

static void cSend_pv63N_sendMessage(HvBase *, int, const HvMessage *const);
static void cLoadbang_JbrFY_sendMessage(HvBase *, int, const HvMessage *const);
static void cBinop_Pmp4F_sendMessage(HvBase *, int, const HvMessage *const);
static void cVar_G98MT_sendMessage(HvBase *, int, const HvMessage *const);
static void cSwitchcase_t3lVp_onMessage(HvBase *, void *, int letIn, const HvMessage *const, void *);
static void cBinop_e1pv9_sendMessage(HvBase *, int, const HvMessage *const);
static void cDelay_R8XhZ_sendMessage(HvBase *, int, const HvMessage *const);
static void cCast_bdoeR_sendMessage(HvBase *, int, const HvMessage *const);
static void cMsg_Y9m3P_sendMessage(HvBase *, int, const HvMessage *const);
static void cMsg_6iQSS_sendMessage(HvBase *, int, const HvMessage *const);
static void cSystem_ykZjF_sendMessage(HvBase *, int, const HvMessage *const);
static void cBinop_puAvm_sendMessage(HvBase *, int, const HvMessage *const);
static void cLoadbang_iorDM_sendMessage(HvBase *, int, const HvMessage *const);



/*
 * Static Helper Functions
 */

static void ctx_intern_scheduleMessageForReceiver(HvBase *const _c, const char *name, HvMessage *m) {
  switch (msg_symbolToHash(name)) {

    default: return;
  }
}

static struct HvTable *ctx_intern_getTableForHash(HvBase *const _c, hv_uint32_t h) {
  switch (h) {

    default: return NULL;
  }
}



/*
 * Context Include and Implementatons
 */

Hv_firehelix *hv_firehelix_new(double sampleRate) {
  hv_assert(sampleRate > 0.0); // can't initialise with sampling rate of 0
  Hv_firehelix *const _c = (Hv_firehelix *) hv_malloc(sizeof(Hv_firehelix));

  Base(_c)->numInputChannels = 0;
  Base(_c)->numOutputChannels = 0;
  Base(_c)->sampleRate = sampleRate;
  Base(_c)->blockStartTimestamp = 0;
  Base(_c)->f_scheduleMessageForReceiver = &ctx_intern_scheduleMessageForReceiver;
  Base(_c)->f_getTableForHash = &ctx_intern_getTableForHash;
  mq_initWithPoolSize(&Base(_c)->mq, 10); // 10KB MessagePool
  Base(_c)->basePath = NULL;
  Base(_c)->printHook = NULL;
  Base(_c)->sendHook = NULL;
  Base(_c)->userData = NULL;
  Base(_c)->name = "firehelix";

  Base(_c)->numBytes = sizeof(Hv_firehelix);
  Base(_c)->numBytes += cBinop_init(&_c->cBinop_Pmp4F, 0.0f); // __mul
  Base(_c)->numBytes += cVar_init_f(&_c->cVar_G98MT, 1000.0f);
  Base(_c)->numBytes += cDelay_init(Base(_c), &_c->cDelay_R8XhZ, 0.0f);

  // loadbang
  ctx_scheduleMessage(Base(_c), msg_initWithBang(HV_MESSAGE_ON_STACK(1), 0), &cLoadbang_iorDM_sendMessage, 0);
  ctx_scheduleMessage(Base(_c), msg_initWithBang(HV_MESSAGE_ON_STACK(1), 0), &cLoadbang_JbrFY_sendMessage, 0);

  return _c;
}

void hv_firehelix_free(Hv_firehelix *_c) {


  hv_free(Base(_c)->basePath);
  mq_free(&Base(_c)->mq); // free queue after all objects have been freed, messages may be cancelled

  hv_free(_c);
}



/*
 * Static Function Implementation
 */

static void cSend_pv63N_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  if (_c->sendHook != NULL) _c->sendHook(ctx_samplesToMilliseconds(_c, msg_getTimestamp(m)), "#PIN_00", m, ctx_getUserData(_c));
}
static void cLoadbang_JbrFY_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  cSwitchcase_t3lVp_onMessage(_c, NULL, 0, m, NULL);
}
static void cBinop_Pmp4F_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  cBinop_k_onMessage(_c, NULL, HV_BINOP_MAX, 1.0f, 0, m, &cBinop_puAvm_sendMessage);
}
static void cVar_G98MT_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  cBinop_onMessage(_c, &Context(_c)->cBinop_Pmp4F, HV_BINOP_MULTIPLY, 0, m, &cBinop_Pmp4F_sendMessage);
}
static void cSwitchcase_t3lVp_onMessage(HvBase *_c, void *o, int letIn, const HvMessage *const m, void *sendMessage) {
  switch (msg_getHash(m,0)) {
    case 0x0: { // "0.0"
      cMsg_Y9m3P_sendMessage(_c, 0, m);
      break;
    }
    case 0x7A5B032D: { // "stop"
      cMsg_Y9m3P_sendMessage(_c, 0, m);
      break;
    }
    default: {
      cCast_onMessage(_c, HV_CAST_BANG, 0, m, &cCast_bdoeR_sendMessage);
      break;
    }
  }
}
static void cBinop_e1pv9_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  cBinop_onMessage(_c, &Context(_c)->cBinop_Pmp4F, HV_BINOP_MULTIPLY, 1, m, &cBinop_Pmp4F_sendMessage);
}
static void cDelay_R8XhZ_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  cDelay_clearExecutingMessage(&Context(_c)->cDelay_R8XhZ, m);
  cDelay_onMessage(_c, &Context(_c)->cDelay_R8XhZ, 0, m, &cDelay_R8XhZ_sendMessage);
  cSend_pv63N_sendMessage(_c, 0, m);
}
static void cCast_bdoeR_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  cMsg_Y9m3P_sendMessage(_c, 0, m);
  cDelay_onMessage(_c, &Context(_c)->cDelay_R8XhZ, 0, m, &cDelay_R8XhZ_sendMessage);
  cSend_pv63N_sendMessage(_c, 0, m);
}
static void cMsg_Y9m3P_sendMessage(HvBase *_c, int letIn, const HvMessage *const n) {
  HvMessage *m = NULL;
  m = HV_MESSAGE_ON_STACK(1);
  msg_init(m, 1, msg_getTimestamp(n));
  msg_setSymbol(m, 0, "clear");
  cDelay_onMessage(_c, &Context(_c)->cDelay_R8XhZ, 0, m, &cDelay_R8XhZ_sendMessage);
}
static void cMsg_6iQSS_sendMessage(HvBase *_c, int letIn, const HvMessage *const n) {
  HvMessage *m = NULL;
  m = HV_MESSAGE_ON_STACK(1);
  msg_init(m, 1, msg_getTimestamp(n));
  msg_setSymbol(m, 0, "samplerate");
  cSystem_onMessage(_c, NULL, 0, m, &cSystem_ykZjF_sendMessage);
}
static void cSystem_ykZjF_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  cBinop_k_onMessage(_c, NULL, HV_BINOP_DIVIDE, 1000.0f, 0, m, &cBinop_e1pv9_sendMessage);
}
static void cBinop_puAvm_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  cDelay_onMessage(_c, &Context(_c)->cDelay_R8XhZ, 2, m, &cDelay_R8XhZ_sendMessage);
}
static void cLoadbang_iorDM_sendMessage(HvBase *_c, int letIn, const HvMessage *const m) {
  cMsg_6iQSS_sendMessage(_c, 0, m);
  cVar_onMessage(_c, &Context(_c)->cVar_G98MT, 0, m, &cVar_G98MT_sendMessage);
}



/*
 * Context Process Implementation
 */

int hv_firehelix_process(Hv_firehelix *const _c, float **const inputBuffers, float **const outputBuffers, int nx) {
  const int n4 = nx & ~HV_N_SIMD_MASK; // ensure that the block size is a multiple of HV_N_SIMD

  // temporary signal vars
  // no temporary float buffers
  // no temporary integer buffers

  // input and output vars
  // no input channels
  // no output channels

  // declare and init the zero buffer
  hv_bufferf_t ZERO; __hv_zero_f(VOf(ZERO));

  hv_uint32_t nextBlock = Base(_c)->blockStartTimestamp;
  for (int n = 0; n < n4; n += HV_N_SIMD) {

    // process all of the messages for this block
    nextBlock += HV_N_SIMD;
    while (mq_hasMessageBefore(&Base(_c)->mq, nextBlock)) {
      MessageNode *const node = mq_peek(&Base(_c)->mq);
      node->sendMessage(Base(_c), node->let, node->m);
      mq_pop(&Base(_c)->mq);
    }

    // no input channels

    // no buffers to zero

    // process all signal functions


    // save output vars to output buffer

  }

  Base(_c)->blockStartTimestamp = nextBlock;

  return n4; // return the number of frames processed
}

int hv_firehelix_process_inline(Hv_firehelix *c, float *const inputBuffers, float *const outputBuffers, int n4) {
  int i = ctx_getNumInputChannels(Base(c));
  float **bIn = (float **) hv_alloca(i*sizeof(float *));
  while (i--) bIn[i] = inputBuffers+(i*n4);

  i = ctx_getNumOutputChannels(Base(c));
  float **bOut = (float **) hv_alloca(i*sizeof(float *));
  while (i--) bOut[i] = outputBuffers+(i*n4);

  int n = hv_firehelix_process(c, bIn, bOut, n4);
  return n;
}
