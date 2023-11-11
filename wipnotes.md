# WIPNotes - Flash

- We have a rough implementation that can store our configuration to flash, and read it at startup.
- This means we can send config via lora and even after power down, it will come back and read it's config.
- Downside is that even after flashing, the config remains, so we need some kind of 'clear flash' option.
- It also doesn't solve the 'how do we program the DevEUI/AppKey'. The SODAQ had a console built in, not sure if we can do that here.
- The example has an option to program via bluetooth, this wouldn't be a bad option but we typically don't connect the BT antenna. Also, this is a lot more stuff.
