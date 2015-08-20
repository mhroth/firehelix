#!/usr/bin/python

from OSC import OSCServer
import sys
import struct
from time import sleep
import spidev
import time

spi = spidev.SpiDev()
spi.open(0, 0)
spi.max_speed_hz = 976000
 
# Split an integer input into a two byte array to send via SPI
def write_pot(val):
    val = int(val)
    msb = val >> 8
    lsb = val & 0xFF
    spi.xfer([msb, lsb])

server = OSCServer( ("localhost", 5005) )
server.timeout = 0
run = True

def float_to_hex(f):
    return hex(struct.unpack('<I', struct.pack('<f', f))[0])

def printit(path, tags, args, source):
#    print("This", path, tags, args, source)
 #   print("Hex", float_to_hex(args[0]))
    write_pot(args[0])

server.addMsgHandler( "/volume", printit)

def each_frame():
    server.timed_out = False
    while not server.timed_out:
        server.handle_request()

while run:
    sleep(1)
    each_frame()

server.close()
