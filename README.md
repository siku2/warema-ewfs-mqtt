# WAREMA EWFS MQTT

Library for controlling a zombified EWFS controller over [MQTT](http://mqtt.org/).
To be a bit more specific: I was unable to reliably control the WAREMA EWFS shutters using RF signals.
This was most likely due to the receiver and transmitter that I was using but instead of spending more time on it I decided to instead abuse a real EWFS remote control to do the job for me.
So here we are, this is the software for controlling this abomination.

This project uses [PlatformIO](https://platformio.org/).
It's not strictly required that you use it but it will make your life a whole lot easier.

Currently only the [ESP32](https://www.espressif.com/en/products/socs/esp32/overview) is supported.

## Quickstart

First you need to set up the controller.
To do this follow the instructions on the [Wiring Wiki page](https://github.com/siku2/warema-ewfs-mqtt/wiki/Wiring) to hook up your transmitter.

Next, follow the instructions on the [Configuration Wiki page](https://github.com/siku2/warema-ewfs-mqtt/wiki/Configuration) to configure the program.

Build and upload the program to your microcontroller and you're ready to go.

It's recommended that you open the serial monitor the first time to check whether it's connecting to the network and to the MQTT broker.

Refer to the [MQTT Wiki page](https://github.com/siku2/warema-ewfs-mqtt/wiki/MQTT) to find out how to control it.

## Documentation

Check out the [Wiki](https://github.com/siku2/warema-ewfs-mqtt/wiki) for the documentation.
