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

## Testing

Some data from our testing (primarily around battery usage).

### Distance sensor

- This is with the vl53l0x sensor.
- Sleep time of 300 seconds.

Data Point 1: 2022-08-05 08:31 - 4.114 (volt) - 0 messages
Data point 2: 2022-08-05 20:01 - 4.087 (volt) - 138 messages
Data Point 3: 2022-06-07 20:05 - 4.045 (volt) - 426 messages
Data Point 4: 2022-06-07 08:00 - 4.025 (volt) - 569 messages

Over 2 days, we lose (4.114 - 4.025) = 0.089 volt. This is basically 0.1 volt per 2 days.
We do see that the 5 minutes sleep time is not ending up being a exact interval, otherwise we'd have 288 messages per day, and we seem to be doing (569/2=)284.5 per day.