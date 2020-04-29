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
To do this follow the instructions on the [Wiring Wiki](https://github.com/siku2/warema-ewfs-mqtt/wiki/Wiring) page to hook up your transmitter.
Connect the transistor bases for the buttons to the pins according to the following table:

| Button   | Pin |
|----------|----:|
| Up       |  13 |
| Stop     |   ? |
| Down     |  12 |
| Next     |   0 |
| Previous |  15 |

Don't forget to connect the power to the transmitter board using the *3v3* and *GND* pins.


Adjust the configuration in the [platformio.ini](platformio.ini).
All configuration is hard-coded into the program when building it.
There are more values that can be configured. Refer to the [Configuration Wiki](https://github.com/siku2/warema-ewfs-mqtt/wiki/Configuration) page for more information.

Build and upload the program to your microcontroller and you're ready to go.

It's recommended that you open the serial monitor the first time to check whether it's properly connecting to the network and the MQTT broker.

## Documentation

Check out the [Wiki](https://github.com/siku2/warema-ewfs-mqtt/wiki) for the documentation.
