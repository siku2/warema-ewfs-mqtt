# WAREMA EWFS MQTT

Library for controlling a zombified EWFS controller over [MQTT](http://mqtt.org/).
To be a bit more specific: I was unable to reliably control the WAREMA EWFS shutters using RF signals.
This was most likely due to the receiver and transmitter that I was using but instead of spending more time on it I decided to instead abuse a real EWFS remote control to do the job for me.
So here we are, this is the software for controlling this abomination.

This project uses [PlatformIO](https://platformio.org/).
It's not strictly required that you use it but it will make your life a whole lot easier.

Currently only the [ESP32](https://www.espressif.com/en/products/socs/esp32/overview) is supported.

## Quickstart

You can find out how to prepare your transmitter on the [Wiring Wiki page](https://github.com/siku2/warema-ewfs-mqtt/wiki/Wiring).

The [Configuration Wiki page](https://github.com/siku2/warema-ewfs-mqtt/wiki/Configuration) goes over how to configure the code.

Documentation for the usage can be found on the [MQTT Wiki page](https://github.com/siku2/warema-ewfs-mqtt/wiki/MQTT).

## Documentation

Check out the [Wiki](https://github.com/siku2/warema-ewfs-mqtt/wiki) for the documentation.
