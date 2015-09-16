# Firehelix
A Burning Man 2015 art project by Trey Watkins, [Yuli Levtov](https://github.com/yulilevtov) and [Martin Roth](https://github.com/mhroth), et al.

Firehelix is a twelve foot tall construction of 36 bidirectional flame poofers in a double helix configuration. It was premiered at Burning Man 2015. The structure was physically built and assembled in Oakland, California. The software and electronics were developed in London, UK.

## How it Works
The Firehelix is controlled by two Raspberry Pi (RPi) computers. A [Compute Module](https://www.raspberrypi.org/products/compute-module-development-kit/) (CM) is located on the rotating structure and a [Model B](https://www.raspberrypi.org/products/raspberry-pi-2-model-b/) is mounted on the supporting frame. They are networked together via a local WiFi network. CM is responsible for individually controlling the 36 flame jets and acting as the central server for the system. B is used to control the game buttons which trigger the performance, as well as the motor which rotates the sculpture.

Both RPis are programmed using the Pure Data (Pd) audio programming language and can be controlled wirelessly via a TouchOSC iPad interface.

In order to make the sculpture appear as if it is being rotated by the force of the flame jets, CM reports to B the number of flame jets active in each direction (i.e. clockwise or anti-clockwise). This number is used to calculate the speed and direction of the motor rotation.

The flame jets perform various patterns including the genome sequence of a frog, algorithmic rhythm generators, and other basic chase and spiral modes. For future versions of the Firehelix there is plenty of scope for extending the types of sequences that the flame jets perform.

## What's in the Repository
This repository contains all code necessary for both CM (in the `src` directory) and B (in the `pd/whackamole` directory). The former runs a native executable generated from a [Heavy](https://enzienaudio.com) compiled version of the Pd patch at `pd/_main.pd`. B runs its Pd patch directly in the Pd virtual machine.

### Compute Module
The glue that allows Pd to interact with the Compute Module is in `src/firehelix.c` This code is responsible for enabling control of the GPIO pins, incoming and outgoing network connections (OSC over UDP), and defining the timing loop which ensures that Heavy is called regularly. The application is single threaded. It consumes between one and two per cent of the CM CPU when managing all poofers and network connections simultaneously. The equivalent patch running in the Pure Data virtual machine might consume twenty or thirty per cent CPU. There are only control objects in the original Pd patch; there is no signal-rate computation.

Work on the Firehelix spawned an independent project called [TinyOSC](https://github.com/mhroth/tinyosc) which is a simple C-based parser and writer for OSC messages. Existing C-language OSC libraries were felt to have overly-complicated APIs for such a simple application.

### B+
