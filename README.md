# RAK Tracker Thing

This is a simple project which aims to implement a GPS tracker. There's other examples out there and this is just /my/ version of it with some specifics that i needed.

## Device

The RAK WisBlock modules used:

- RAK19009 (WisBlock Mini Base Board)
- RAK4631 (WisBlock LPWAN Module Nordic nRF52840)
- RAK1904 (WisBlock 3-Axis Acceleration Sensor STMicroelectronics LIS3DH) - optional
- RAK12500 (WisBlock GNSS GPS Location Module u-blox ZOE-M8Q)

We've experimented with the RAK11310, which is the Raspberry PI Pico/RP2040. You might find code related to that.
We need to make this device 'low-power' or 'deep-sleep'. So it means we can't do that currently with the RP2040. We're waiting for support of low-power mode in the Arduino BSP.

## Goals

The goal of this tracker is to be able to:
- Send a GPS update every 5 minutes
- Be able to run without solar charge for 3 days on a 2000mAh battery.
- With a solar charge, be able to run indefinetly (until the battery runs out - let's say 3+ years)
- Be able to configure the device via Lora


## Dev Environment
We're using VSCode and have added the .vscode folder as well to make it easy.
Also note that the git repo has submodules, so you'll need to do a `git submodule init && git submodule update` after the clone.


## Halt codes:

The blue led is used to indicate halt codes. Halt codes are used for an unrecoverable error. Quite often this is hardware that's missing or can't be initialized.
The halt codes blink pretty fast (200ms between blink), with 2 second between the blinks.


```
2 = Initialization of config failed
3 = GPS Initialization failed
4 = powersave failed
5 = Motion sensor failure

```

## Status blinks

The device also provides a blinking pattern for failed things. These are more for debug purposses as in normal operation you wouldn't look at them.
We have however made them unique, so that you can just look at the blinks. There is half a second between the blink.

```
2 = Lora join failed
3 = Lora send busy
4 = GPS fix timeout
5 = Lora send error
6 = Lora confirmation failed, resending

```