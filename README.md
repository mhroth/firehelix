# Firehelix
A Burning Man 2015 art project by Trey Watkins, [Yuli Levtov](https://github.com/ylevtov) and [Martin Roth](https://github.com/mhroth), et al.

Firehelix [@onemandown](http://onemandown.com/FireHelix.htm) [@youtube](https://www.youtube.com/watch?v=B5KVtUZQuUs) [@reactify](http://reactifymusic.com/portfolio/the-firehelix/) is a twelve foot tall construction of 36 bidirectional flame poofers in a double helix configuration. It was premiered at Burning Man 2015. The structure was physically built and assembled in Oakland, California. The software and electronics were developed in London, UK.

## How it Works
The Firehelix is controlled by two Raspberry Pi (RPi) computers. A [Compute Module](https://www.raspberrypi.org/products/compute-module-development-kit/) (CM) is located on the rotating structure and a [Model B](https://www.raspberrypi.org/products/raspberry-pi-2-model-b/) is mounted on the supporting frame. They are networked together via a local WiFi network. CM is responsible for individually controlling the 36 flame jets and acting as the central server for the system. B is used to control the game buttons which trigger a performance, as well as the motor which rotates the sculpture.

Both RPis are programmed using the [Pure Data](http://puredata.info) (Pd) audio programming language and can be controlled wirelessly via a TouchOSC iPad interface.

In order to make the sculpture appear as if it is being rotated by the force of the flame jets, CM reports to B the number of flame jets active in each direction (i.e. clockwise or anti-clockwise). This number is used to calculate the speed and direction of the motor rotation.

The flame jets perform various patterns including the genome sequence of a frog, algorithmic rhythm generators, and other basic chase and spiral modes. For future versions of the Firehelix there is plenty of scope for extending the types of sequences that the flame jets perform.

## Repository Structure
This repository contains all code necessary for both CM (in the `src` directory) and B (in the `pd/whackamole` directory). The former runs a native executable generated from a [Heavy](https://enzienaudio.com) compiled version of the Pd patch at `pd/_main.pd`. B runs its Pd patch directly in the Pd virtual machine.

### RPi Compute Module
The glue that allows Pd to interact with the Compute Module is in `src/firehelix.c` This code is responsible for enabling control of the GPIO pins, incoming and outgoing network connections (OSC over UDP), and defining the timing loop which ensures that the Heavy process function is called regularly. The application is single threaded. It consumes between one and two per cent of the CM CPU when managing all poofers and network connections simultaneously. The equivalent patch running in the Pure Data virtual machine might consume twenty or thirty per cent CPU. There are only control objects in the original Pd patch; there is no signal-rate computation.

### TinyOSC
Work on the Firehelix spawned an independent project called [TinyOSC](https://github.com/mhroth/tinyosc) which is a simple C-based parser and writer for OSC messages. Existing C-language OSC libraries were felt to have overly-complicated APIs for such a simple application.

## License
All Firehelix code is published under the [ISC license](http://opensource.org/licenses/ISC). Please see the `LICENSE` file included in this repository, also reproduced below. In short, you are welcome to use this code for any purpose, including commercial and closed-source use.

```
Copyright (c) 2015, Martin Roth <mhroth@gmail.com>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
```

## Setting up the RPi Compute Module
1. Follow the [instructions for flashing](https://www.raspberrypi.org/documentation/hardware/computemodule/cm-emmc-flashing.md) the RPi compute module
2. [Setup](http://www.howtogeek.com/167425/how-to-setup-wi-fi-on-your-raspberry-pi-via-the-command-line/) the WiFi connection
  * `$ sudo nano /etc/network/interfaces`
  * add:
  ```
allow-hotplug wlan0
iface wlan0 inet dhcp
wpa-conf /etc/wpa_supplicant/wpa_supplicant.conf
iface default inet dhcp
  ```
  * `$ sudo nano /etc/wpa_supplicant/wpa_supplicant.conf`
  * add:
  ```
network={
    ssid="YOURSSID"
    psk="YOURPASSWORD"
    # Protocol type can be: RSN (for WP2) and WPA (for WPA1)
    proto=WPA
}
  ```
3. [Enable](http://cplus.about.com/od/raspberrypi/a/How-Do-I-Setup-Ssh-On-Raspberry-Pi.htm) `sshd`
  * `$ sudo update-rc.d ssh defaults`
  * `$ sudo /etc/init.d/ssh start`
4. Turn off [WiFi power saving](http://www.averagemanvsraspberrypi.com/2014/10/how-to-set-up-wifi-on-raspberry-pi.html)
  * `$ sudo nano /etc/modprobe.d/8192cu.conf `
  * Add the line `options 8192cu rtw_power_mgnt=0 rtw_enusbss=0 rtw_ips_mode=1`.
5. Restart the Rpi
  * `$ sudo reboot`

