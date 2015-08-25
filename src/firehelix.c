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

#include "heavy/c/Heavy_firehelix.h"
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
#define GPIO_SET(g) *(gpio.addr + 7 + (g/32)) = (1 << ((g)%32))
#define GPIO_CLR(g) *(gpio.addr + 10 + (g/32)) = (1 << ((g)%32))
#define GPIO_READ(g) *(gpio.addr + 13 + (g/32)) &= (1 << ((g)%32)) // NOTE(mhroth): unused for now

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

void timespec_subtract(struct timespec *result, struct timespec *end, struct timespec *start) {
  if (end->tv_nsec < start->tv_nsec) {
    result->tv_sec = end->tv_sec - start->tv_sec - 1;
    result->tv_nsec = SEC_TO_NS + end->tv_nsec - start->tv_nsec;
  } else {
    result->tv_sec = end->tv_sec - start->tv_sec;
    result->tv_nsec = end->tv_nsec - start->tv_nsec;
  }
}
static void hv_printHook(double timestamp, const char *name, const char *s,
    void *userData) {
  printf("[@h %.3fms] %s: %s\n", timestamp, name, s);
}

/*
    struct timespec tock, diff_tick;
    clock_gettime(CLOCK_REALTIME, &tock);
    timespec_subtract(&diff_tick, &tock, (struct timespec *) userData);
    const int64_t elapsed_ns = (((int64_t) diff_tick.tv_sec) * SEC_TO_NS) + diff_tick.tv_nsec;
    const double elapsed_ms = ((double) elapsed_ns) / 1000000.0;
    printf("[clock drift %.3f%%]: %s.\n", 100.0*(elapsed_ms-timestamp)/timestamp, receiverName);
*/
static void hv_sendHook(double timestamp, const char *receiverName,
    const HvMessage *m, void *userData) {
  if (!strcmp(receiverName, "#toGPIO")) {
    int pin = (int) hv_msg_getFloat(m, 0); // pin is zero indexed
    if (pin >= 0 && pin < NUM_GPIO_PINS) { // error checking
      const float f = hv_msg_getFloat(m, 1);
      if (pin == 16) pin = 40; // TODO(mhroth): for some reason pin 16 often gets reset
      if (f == 0.0f) GPIO_CLR(pin);
      else GPIO_SET(pin);

      {
        // send the pin state back to TouchOSC
        int fd = *((int *) userData);
        char addr[16];
        if (pin == 40) pin = 16; // NOTE(mhroth): undo the above transformation
        snprintf(addr, sizeof(addr), "/1/led%i", pin);
        char buffer[64];
        int len = tosc_write(buffer, sizeof(buffer), addr, "f", f);
        send(fd, buffer, len, 0);
      }
    } else {
      printf("Received message to #toGPIO with OOB pin index: %i\n", pin);
    }
  } else if (!strcmp(receiverName, "#time-remaining-label")) {
    // send sequence time to TouchOSC
    int fd = *((int *) userData);
    char str[32];
    snprintf(str, sizeof(str), "%g:%g",
      hv_msg_getFloat(m, 0), hv_msg_getFloat(m, 1));
    char buffer[64];
    int len = tosc_write(buffer, sizeof(buffer),
        "/time-remaining-label", "s", str);
    send(fd, buffer, len, 0);
  } else if (!strcmp(receiverName, "#status-label")) {
    // send status label to TouchOSC
    int fd = *((int *) userData);
    char buffer[64];
    int len = tosc_write(buffer, sizeof(buffer),
        "/status-label", "s", hv_msg_getSymbol(m, 0));
    send(fd, buffer, len, 0);
  } else {
    // NOTE(mhroth): this can get annoying, uncomment if necessary
    // char *msg_string = hv_msg_toString(m);
    // printf("Received unknown Pd message: [@h %0.3fms] %s: %s\n", timestamp, receiverName, msg_string);
    // free(msg_string);
  }
}

static int openReceiveSocket() {
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  fcntl(fd, F_SETFL, O_NONBLOCK); // set the socket to non-blocking
  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(2015);
  sin.sin_addr.s_addr = INADDR_ANY;
  bind(fd, (struct sockaddr *) &sin, sizeof(struct sockaddr_in));
  return fd;
}

static int openTouchOscSocket() {
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(2013);
  inet_aton("192.168.0.13", &sin.sin_addr);
  int err = connect(fd, (struct sockaddr *) &sin, sizeof(struct sockaddr_in));
  if (err != 0) {
    printf("Failed to open TouchOSC socket: %i\n", err);
    return -1;
  } else return fd;
}

static int openRpiSocket() {
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(2015);
  inet_aton("192.168.0.11", &sin.sin_addr);
  int err = connect(fd, (struct sockaddr *) &sin, sizeof(struct sockaddr_in));
  if (err != 0) {
    printf("Failed to open RPI socket: %i\n", err);
    return -1;
  } else return fd;
}

// http://man7.org/linux/man-pages/man3/getifaddrs.3.html
// http://beej.us/guide/bgnet/output/html/multipage/inet_ntopman.html
// http://beej.us/guide/bgnet/output/html/multipage/sockaddr_inman.html
static void printWlanIpPort() {
  struct ifaddrs *ifaddr = NULL;
  char host[INET_ADDRSTRLEN];
  getifaddrs(&ifaddr);
  for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (!strcmp(ifa->ifa_name, "wlan0")) { // only display IP for interface wlan0
      if (ifa->ifa_addr->sa_family == AF_INET) {
        struct sockaddr_in *sa = (struct sockaddr_in *) ifa->ifa_addr;
        inet_ntop(AF_INET, &(sa->sin_addr), host, INET_ADDRSTRLEN);
        printf("WiFi: %s:2015 (74:DA:38:33:AF:EC)\n", host);
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
  printf("Audio: %i @ %gHz (%0.3fms)\n",
      HEAVY_BLOCKSIZE, HEAVY_SAMPLE_RATE,
      1000.0*HEAVY_BLOCKSIZE/HEAVY_SAMPLE_RATE);
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

  // for receiving commands (from TouchOSC and whack-a-mole rpi)
  const int fd_receive = openReceiveSocket();

  // for sending to TouchOSC
  const int fd_touchosc = openTouchOscSocket();

  // for sending to RPI
  const int fd_rpi = openRpiSocket();

  // buffer into which network data is received
  char buffer[1024];

  struct timespec start_tick, tock, diff_tick;
  struct sockaddr_in sin;
  int len = 0;
  int sa_len = sizeof(struct sockaddr_in);
  tosc_tinyosc osc;

  // initialise and configure Heavy
  printf("Instantiating and configuring Heavy... ");
  Hv_firehelix *hv_context = hv_firehelix_new_with_pool(HEAVY_SAMPLE_RATE, 100);
  hv_setPrintHook(hv_context, &hv_printHook);
  hv_setSendHook(hv_context, &hv_sendHook);
  hv_setUserData(hv_context, (void *) &fd_touchosc);
  printf("done.\n");

  printf("Starting runloop.\n");
  clock_gettime(CLOCK_REALTIME, &start_tick);
  while (_keepRunning) {

    // receive and parse all OSC message received since the last block
    while ((len = recvfrom(fd_receive, buffer, sizeof(buffer), 0, (struct sockaddr *) &sin, (socklen_t *) &sa_len)) > 0) {
      if (!tosc_read(&osc, buffer, len)) { // parse the osc packet, continue on success
        if (!strcmp(osc.address, "/slider")) {
          hv_vscheduleMessageForReceiver(
              hv_context, "#slider", 0.0, "f", tosc_getNextFloat(&osc));
        } else if (!strcmp(osc.address, "/trail-length")) {
          hv_vscheduleMessageForReceiver(
              hv_context, "#trail-length", 0.0, "f", tosc_getNextFloat(&osc));
        } else if (!strcmp(osc.address, "/auto-speed")) {
          hv_vscheduleMessageForReceiver(
              hv_context, "#auto-speed", 0.0, "f", tosc_getNextFloat(&osc));
        } else if (!strcmp(osc.address, "/auto-off")) {
          hv_vscheduleMessageForReceiver(
              hv_context, "#auto-off", 0.0, "f", tosc_getNextFloat(&osc));
        } else if (!strcmp(osc.address, "/start-sequence")) {
          hv_vscheduleMessageForReceiver(hv_context, "#start-sequence", 0.0, "b");
        } else if (!strncmp(osc.address, "/poofer-", 8)) {
          // convert e.g. "/poofer-0" to "#poofer-0"
          char str[16];
          strncpy(str, osc.address, sizeof(str));
          str[0] = '#';
          hv_vscheduleMessageForReceiver(
              hv_context, str, 0.0, "f", tosc_getNextFloat(&osc));
        } else if (!strncmp(osc.address, "/branch-index", 13)) {
          if (tosc_getNextFloat(&osc) == 1.0f) { // if is on
            // e.g. /branch-index/1/1 ... /branch-index/9/1
            const int branch_index = osc.address[14] - '0';
            hv_vscheduleMessageForReceiver(
                hv_context, "#branch-index", 0.0, "f", (float) branch_index);
          }
        } else if (!strncmp(osc.address, "/mode-index", 11)) {
          const bool is_on = (tosc_getNextFloat(&osc) == 1.0f);
          if (!strcmp(osc.address, "/mode-index/1/1") && is_on) {
            hv_vscheduleMessageForReceiver(hv_context, "#mode-index", 0.0, "f", 1.0f);
          } else if (!strcmp(osc.address, "/mode-index/2/1") && is_on) {
            hv_vscheduleMessageForReceiver(hv_context, "#mode-index", 0.0, "f", 2.0f);
          } else if (!strcmp(osc.address, "/mode-index/3/1") && is_on) {
            hv_vscheduleMessageForReceiver(hv_context, "#mode-index", 0.0, "f", 3.0f);
          } else if (!strcmp(osc.address, "/mode-index/4/1") && is_on) {
            hv_vscheduleMessageForReceiver(hv_context, "#mode-index", 0.0, "f", 4.0f);
          } else if (!strcmp(osc.address, "/mode-index/5/1") && is_on) {
            hv_vscheduleMessageForReceiver(hv_context, "#mode-index", 0.0, "f", 5.0f);
          } else if (!strcmp(osc.address, "/mode-index/6/1") && is_on) {
            hv_vscheduleMessageForReceiver(hv_context, "#mode-index", 0.0, "f", 6.0f);
          } else if (!strcmp(osc.address, "/mode-index/7/1") && is_on) {
            hv_vscheduleMessageForReceiver(hv_context, "#mode-index", 0.0, "f", 7.0f);
          } else if (!strcmp(osc.address, "/mode-index/8/1") && is_on) {
            hv_vscheduleMessageForReceiver(hv_context, "#mode-index", 0.0, "f", 8.0f);
          } else if (!strcmp(osc.address, "/mode-index/9/1") && is_on) {
            hv_vscheduleMessageForReceiver(hv_context, "#mode-index", 0.0, "f", 9.0f);
          } else if (!strcmp(osc.address, "/mode-index/10/1") && is_on) {
            hv_vscheduleMessageForReceiver(hv_context, "#mode-index", 0.0, "f", 10.0f);
          } else if (!strcmp(osc.address, "/mode-index/11/1") && is_on) {
            hv_vscheduleMessageForReceiver(hv_context, "#mode-index", 0.0, "f", 11.0f);
          } else if (!strcmp(osc.address, "/mode-index/12/1") && is_on) {
            hv_vscheduleMessageForReceiver(hv_context, "#mode-index", 0.0, "f", 12.0f);
          } else if (!strcmp(osc.address, "/mode-index/13/1") && is_on) {
            hv_vscheduleMessageForReceiver(hv_context, "#mode-index", 0.0, "f", 13.0f);
          } else if (!strcmp(osc.address, "/mode-index/14/1") && is_on) {
            hv_vscheduleMessageForReceiver(hv_context, "#mode-index", 0.0, "f", 14.0f);
          } else if (!strcmp(osc.address, "/mode-index/15/1") && is_on) {
            hv_vscheduleMessageForReceiver(hv_context, "#mode-index", 0.0, "f", 15.0f);
          } else if (!strcmp(osc.address, "/mode-index/16/1") && is_on) {
            hv_vscheduleMessageForReceiver(hv_context, "#mode-index", 0.0, "f", 16.0f);
          }
        } else if (!strcmp(osc.address, "/all-off")) {
          if (tosc_getNextFloat(&osc) == 1.0f) {
            hv_vscheduleMessageForReceiver(hv_context, "#all-off", 0.0, "b");
          }
        } else if (!strcmp(osc.address, "/start-button")) {
          send(fd_rpi, buffer, len, 0); // forward the message to the rpi
        } else if (!strcmp(osc.address, "/reset-button")) {
          send(fd_rpi, buffer, len, 0); // forward the message to the rpi
        } else if (!strcmp(osc.address, "/sigint")) {
          sigintHandler(0); // allow the program to exit
        } else {
          printf("Received unknown OSC message: ");
          tosc_printOscBuffer(buffer, len);
        }
      }
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

  // close the UDP sockets
  close(fd_receive); // the listening socket
  close(fd_touchosc); // the TouchOSC socket
  close(fd_rpi); // the RPI socket

  // clear all pins
  for (int i = 0; i < NUM_GPIO_PINS; i++) GPIO_CLR(i);

  // unmap memory
  unmap_peripheral(&gpio);

  // free heavy
  hv_firehelix_free(hv_context);

  printf("done.\n");
}
