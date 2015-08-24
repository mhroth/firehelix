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

#multiplier for SPI digipot output
multiplier = 256

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

def printIt(path, tags, args, source):
    if (args[0]==0):
        print("Direction: Reverse")
    else:
        print("Direction: Forward")

def sendToPot(path, tags, args, source):
    write_pot(int(args[0]*multiplier))
    print("Speed:", args[0]*multiplier)

server.addMsgHandler( "/force", sendToPot)
server.addMsgHandler( "/direction", printIt)

def each_frame():
    server.timed_out = False
    while not server.timed_out:
        server.handle_request()

while run:
    sleep(1)
    each_frame()

server.close()
