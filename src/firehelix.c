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

#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h> // for open
#include <sys/mman.h>
#include <sys/time.h> // gettimeofday
#include <time.h> // nanosleep
#include <unistd.h> // for close

#include "heavy/Heavy_firehelix.h"

// http://www.susa.net/wordpress/2012/06/raspberry-pi-relay-using-gpio/
// http://pieter-jan.com/node/15
// https://enzienaudio.com/h/mhroth/firehelix/

#define IOBASE 0x20000000
#define GPIO_BASE (IOBASE + 0x200000)

struct bcm2835_peripheral {
  unsigned long addr_p;
  int mem_fd;
  void *map;
  volatile unsigned int *addr;
};

struct bcm2835_peripheral gpio = {GPIO_BASE};

#define PIN_07 4

#define INP_GPIO(g) *(gpio.addr + ((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio.addr + ((g)/10)) |=  (1<<(((g)%10)*3))
// sets bits which are 1 ignores bits which are 0
#define GPIO_SET(g) *(gpio.addr + 7) = (1 << (g))
// clears bits which are 1 ignores bits which are 0
#define GPIO_CLR(g) *(gpio.addr + 10) = (1 << (g))
#define GPIO_READ(g) *(gpio.addr + 13) &= (1 << (g))

// configure pins for output
// INP_GPIO(PIN_07);
// OUT_GPIO(PIN_07);

#define MMAP_PAGE_SIZE 4096
#define MMAP_BLOCK_SIZE 4096

#define HEAVY_SAMPLE_RATE 48000.0 // Hz
#define HEAVY_BLOCKSIZE 256 // samples

#define SEC_TO_NS_L 1000000000L
#define US_TO_NS_L 1000L

// Some forward declarations...
// Exposes the physical address defined in the passed structure using mmap on /dev/mem
static int map_peripheral(struct bcm2835_peripheral *p) {
  // Open /dev/mem
  if ((p->mem_fd = open("/dev/mem", O_RDWR|O_SYNC)) < 0) {
    printf("Failed to open /dev/mem, try checking permissions.\n");
    return -1;
  }

  p->map = mmap(
    NULL,
    MMAP_BLOCK_SIZE,
    PROT_READ|PROT_WRITE,
    MAP_SHARED,
    p->mem_fd, // File descriptor to physical memory virtual file '/dev/mem'
    p->addr_p // Address in physical map that we want this memory block to expose
  );

  if (p->map == MAP_FAILED) {
    perror("mmap");
    return -1;
  }

  p->addr = (volatile unsigned int *) p->map;

  return 0;
}

static void unmap_peripheral(struct bcm2835_peripheral *p) {
  munmap(p->map, MMAP_BLOCK_SIZE);
  close(p->mem_fd);
}

static volatile bool _keepRunning = true;

// http://stackoverflow.com/questions/4217037/catch-ctrl-c-in-c
static void sigintHandler(int x) {
  printf("Termination signal received.\n"); // handle Ctrl+C
  _keepRunning = false;
}

static void hv_printHook(double timestamp, const char *name, const char *s,
    void *userData) {
  printf("[@h %.3fms] %s: %s\n", timestamp, name, s);
}

static void hv_sendHook(double timestamp, const char *receiverName,
    const HvMessage *m, void *userData) {
  if (receiverName[0] == '#') { // minimise overhead of sendhook
    struct timeval *tick = (struct timeval *) userData;
    struct timeval tock;
    gettimeofday(&tock, NULL);
    const int64_t elapsed_ns = ((tock.tv_sec - tick->tv_sec) * SEC_TO_NS_L) + // sec to ns
        ((tock.tv_usec - tick->tv_usec) * US_TO_NS_L); // us to ns
    const double elapsed_ms = ((double) elapsed_ns) / 1000000.0;
    printf("[clock drift %.3f]: %s.\n", elapsed_ms/timestamp, receiverName);

    if (!strncmp(receiverName, "#PIN_00", 7)) {
      // // TODO(mhroth): do this correctly
      // if (hv_msg_getFloat(m, 0) == 0.0f) GPIO_CLR(PIN_07);
      // else GPIO_SET(PIN_07);
    } else if (!strncmp(receiverName, "#PIN_01", 7)) {
      // TODO(mhroth): etc.
    }
  }
}

int main(int argc, char *argv[]) {
  printf("Welcome to Firehelix @ Burning Man 2015.\n");
  printf("PID: %i\n", getpid());
  printf("Audio: %i @ %gHz\n", HEAVY_BLOCKSIZE, HEAVY_SAMPLE_RATE);
  printf("Network: xxx.xxx.xxx.xxx:2015\n");
  printf("Press Ctrl+C to exit.\n");
  printf("\n");

  // register the SIGINT handler
  signal(SIGINT, &sigintHandler);
/*
  if(map_peripheral(&gpio) == -1) {
      printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
      return -1;
  }
*/

  const int64_t blocksize_ns =
      (int64_t) (1000000000.0 * HEAVY_BLOCKSIZE / HEAVY_SAMPLE_RATE);

  struct timeval start_tick, tick, tock;

  // initialise and configure Heavy
  printf("Instantiating and configuring Heavy... ");
  Hv_firehelix *hv_context = hv_firehelix_new(HEAVY_SAMPLE_RATE);
  hv_setPrintHook(hv_context, &hv_printHook);
  hv_setSendHook(hv_context, &hv_sendHook);
  hv_setUserData(hv_context, &start_tick);
  printf("done.\n");

  printf("Starting runloop.\n");
  gettimeofday(&start_tick, NULL); // get wall time start of runloop
  while (_keepRunning) {
    // process Heavy
    gettimeofday(&tick, NULL);
    hv_firehelix_process(hv_context, NULL, NULL, HEAVY_BLOCKSIZE); // no IO buffers
    gettimeofday(&tock, NULL);

    const int64_t elapsed_ns = ((tock.tv_sec - tick.tv_sec) * SEC_TO_NS_L) + // sec to ns
        ((tock.tv_usec - tick.tv_usec) * US_TO_NS_L); // us to ns

    const int64_t sleep_us = blocksize_ns - elapsed_ns;
    if (sleep_us > 0) {
      struct timespec sleep_nano;
      // sleep_nano.tv_sec = (time_t) (sleep_us/SEC_TO_NS_L);
      // sleep_nano.tv_nsec = (long) (sleep_us - (sleep_nano.tv_sec*SEC_TO_NS_L));
      sleep_nano.tv_sec = 0;
      // nothing will ever take longer than 1 second (famous last words...)
      sleep_nano.tv_nsec = (long) sleep_us;
      nanosleep(&sleep_nano, NULL);
    }
    else printf("Buffer underrun by %llius\n", -1*sleep_us);
  }
  printf("Firehelix shutting down... ");

  // TODO(mhroth): set all pins to 0 and unmap everything
  //GPIO_CLR(PIN_07);
  //unmap_peripheral(&gpio);

  // free heavy
  hv_firehelix_free(hv_context);

  printf("done.\n");

  return 0;
}
