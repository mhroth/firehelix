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

#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <ifaddrs.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <time.h> // nanosleep, clock_gettime
#include <unistd.h> // for close

#include "heavy/Heavy_firehelix.h"
#include "tinyosc/tinyosc.h"

// http://www.susa.net/wordpress/2012/06/raspberry-pi-relay-using-gpio/
// http://pieter-jan.com/node/15
// https://enzienaudio.com/h/mhroth/firehelix/

// https://www.raspberrypi.org/forums/viewtopic.php?t=61665&p=479174
// https://www.raspberrypi.org/documentation/hardware/computemodule/cm-peri-sw-guide.md
// http://elinux.org/RPi_BCM2835_GPIOs
// https://www.raspberrypi.org/wp-content/uploads/2012/02/BCM2835-ARM-Peripherals.pdf

#define IOBASE 0x20000000
#define GPIO_BASE (IOBASE + 0x200000)

struct bcm2835_peripheral {
  unsigned long addr_p;
  int mem_fd;
  void *map;
  volatile unsigned int *addr;
};

struct bcm2835_peripheral gpio = {GPIO_BASE};

#define INP_GPIO(g) *(gpio.addr + ((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio.addr + ((g)/10)) |=  (1<<(((g)%10)*3))
#define GPIO_SET(g) *(gpio.addr + 7 + (g/32)) = (1 << (g))
#define GPIO_CLR(g) *(gpio.addr + 10 + (g/32)) = (1 << (g))
#define GPIO_READ(g) *(gpio.addr + 13) &= (1 << (g)) // NOTE(mhroth): unused for now

#define MMAP_PAGE_SIZE 4096
#define MMAP_BLOCK_SIZE 4096

#define HEAVY_SAMPLE_RATE 48000.0 // Hz
#define HEAVY_BLOCKSIZE 256 // samples

#define SEC_TO_NS 1000000000

#define NUM_GPIO_PINS 46

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

void timespec_subtract(struct timespec *result, struct timespec *end, struct timespec *start)
{
  if ((end->tv_nsec - start->tv_nsec) < 0) {
    result->tv_sec = end->tv_sec - start->tv_sec - 1;
    result->tv_nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
  } else {
    result->tv_sec = end->tv_sec - start->tv_sec;
    result->tv_nsec = end->tv_nsec - start->tv_nsec;
  }
}
static void hv_printHook(double timestamp, const char *name, const char *s,
    void *userData) {
  printf("[@h %.3fms] %s: %s\n", timestamp, name, s);
}

static void hv_sendHook(double timestamp, const char *receiverName,
    const HvMessage *m, void *userData) {
  if (receiverName[0] == '#') { // minimise overhead of sendhook
/*
    struct timespec tock, diff_tick;
    clock_gettime(CLOCK_REALTIME, &tock);
    timespec_subtract(&diff_tick, &tock, (struct timespec *) userData);
    const int64_t elapsed_ns = (((int64_t) diff_tick.tv_sec) * SEC_TO_NS) + diff_tick.tv_nsec;
    const double elapsed_ms = ((double) elapsed_ns) / 1000000.0;
    printf("[clock drift %.3f%%]: %s.\n", 100.0*(elapsed_ms-timestamp)/timestamp, receiverName);
*/
    if (!strncmp(receiverName, "#PIN_00", 7)) {
      if (hv_msg_getFloat(m, 0) == 0.0f) GPIO_CLR(0);
      else GPIO_SET(0);
    } else if (!strncmp(receiverName, "#PIN_01", 7)) {
      // TODO(mhroth): etc.
    }
  }
}

static int openOscSocket() {
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  fcntl(fd, F_SETFL, O_NONBLOCK); // set the socket to non-blocking
  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(2015);
  sin.sin_addr.s_addr = INADDR_ANY;
  bind(fd, (struct sockaddr *) &sin, sizeof(struct sockaddr_in));
  return fd;
}

// http://man7.org/linux/man-pages/man3/getifaddrs.3.html
// http://beej.us/guide/bgnet/output/html/multipage/inet_ntopman.html
// http://beej.us/guide/bgnet/output/html/multipage/sockaddr_inman.html
static void printWlanIpPort() {
  struct ifaddrs *ifaddr = NULL;
  char host[INET_ADDRSTRLEN];
  getifaddrs(&ifaddr);
  for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (!strncmp(ifa->ifa_name, "wlan0", 5)) { // only display IP for interface wlan0
      if (ifa->ifa_addr->sa_family == AF_INET) {
        struct sockaddr_in *sa = (struct sockaddr_in *) ifa->ifa_addr;
        inet_ntop(AF_INET, &(sa->sin_addr), host, INET_ADDRSTRLEN);
        printf("WiFi: %s:2015\n", host);
        break;
      }
    }
  }
  freeifaddrs(ifaddr);
}

static void printClockResolution() {
  struct timespec tick;
  clock_getres(CLOCK_REALTIME, &tick);
  printf("Clock resolution: %ins\n", tick.tv_nsec);
}

void main(int argc, char *argv[]) {
  printf("Welcome to Firehelix - Burning Man 2015.\n");
  printf("PID: %i\n", getpid());
  printf("Audio: %i @ %gHz\n", HEAVY_BLOCKSIZE, HEAVY_SAMPLE_RATE);
  printWlanIpPort();
  printClockResolution();
  printf("Press Ctrl+C to exit.\n");
  printf("\n");

  // register the SIGINT handler
  signal(SIGINT, &sigintHandler);

  // map peripherals
  map_peripheral(&gpio);

  // configure all 46 GPIO pins for output
  // NOTE(mhroth): confirm state with '$ sudo raspi-gpio get'
  printf("Initialising GPIO pins... ");
  for (int i = 0; i < NUM_GPIO_PINS; i++) {
    INP_GPIO(i); // clear all config bits for pin
    OUT_GPIO(i); // set pin as output
    GPIO_CLR(i); // clear output
  }
  printf("done.\n");

  const int socket_fd = openOscSocket();
  char buffer[1024]; // buffer into which network data is received

  struct timespec start_tick, tock, diff_tick;
  struct sockaddr_in sin;
  int len = 0;
  int sa_len = sizeof(struct sockaddr_in);
  tosc_tinyosc osc;

  // initialise and configure Heavy
  printf("Instantiating and configuring Heavy... ");
  Hv_firehelix *hv_context = hv_firehelix_new(HEAVY_SAMPLE_RATE);
  hv_setPrintHook(hv_context, &hv_printHook);
  hv_setSendHook(hv_context, &hv_sendHook);
  hv_setUserData(hv_context, &start_tick);
  printf("done.\n");

  printf("Starting runloop.\n");
  clock_gettime(CLOCK_REALTIME, &start_tick);
  while (_keepRunning) {
    while ((len = recvfrom(socket_fd, buffer, sizeof(buffer), 0, (struct sockaddr *) &sin, (socklen_t *) &sa_len)) > 0) {
      tosc_init(&osc, buffer, len);
      printf("Received: [%i bytes] %s %s ", len, osc.address, osc.format);
      for (int i = 0; osc.format[i] != '\0'; i++) {
        switch (osc.format[i]) {
          case 'f': printf("%g ", tosc_getNextFloat(&osc)); break;
          case 'i': printf("%i ", tosc_getNextInt32(&osc)); break;
          default: continue;
        }
      }
      printf("\n");

      // hv_vscheduleMessage(hv_context, "receiverName", "f", f);
    }

    // process Heavy
    hv_firehelix_process(hv_context, NULL, NULL, HEAVY_BLOCKSIZE); // no IO buffers
    clock_gettime(CLOCK_REALTIME, &tock);

    timespec_subtract(&diff_tick, &tock, &start_tick);
    const int64_t elapsed_ns = (((int64_t) diff_tick.tv_sec) * SEC_TO_NS) + diff_tick.tv_nsec;
    const int64_t block_ns = (int64_t) (hv_getCurrentTime(hv_context) * 1000000000.0);
    const int64_t sleep_ns = block_ns - elapsed_ns;
    if (sleep_ns > 0) {
      struct timespec sleep_nano;
      // there is never a need to sleep longer than 1 second (famous last words...)
      sleep_nano.tv_sec = 0;
      sleep_nano.tv_nsec = (long) sleep_ns;
      nanosleep(&sleep_nano, NULL);
    }
    else printf("Buffer underrun by %llins\n", -1*sleep_ns);
  }
  printf("Firehelix shutting down... ");

  // close the UDP socket
  close(socket_fd);

  // clear all pins
  for (int i = 0; i < NUM_GPIO_PINS; i++) GPIO_CLR(i);

  // unmap memory
  unmap_peripheral(&gpio);

  // free heavy
  hv_firehelix_free(hv_context);

  printf("done.\n");
}
