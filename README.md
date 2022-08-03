# RAK Tracker Thing

This is a simple project which aims to implement a GPS tracker. There's other examples out there and this is just /my/ version of it with some specifics that i needed.

## Device

The RAK WisBlock modules used:
- RAK19007 (WisBlock Base Board) 
- RAK4631 (WisBlock LPWAN Module)

We've experimented with the RAK11310, which is the Raspberry PI Pico/RP2040. You might find code related to that.
We need to make this device 'low-power' or 'deep-sleep'. So it means we can't do that currently with the RP2040. We're waiting for support of low-power mode in the Arduino BSP.


## Goals

The goal of this tracker is to be able to:
- Send a GPS update every 5 minutes
- Be able to run without solar charge for 3 days on a 2000mAh battery.
- With a solar charge, be able to run indefinetly (until the battery runs out - let's say 3+ years)

### Nice to haves
- Be able to configure some parameters remotely (via lora). For example:
  - Configure the update interval
- Have a mode where it sends updates more often when movement is detected during certain hours
