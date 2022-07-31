# ClaudoBip HDMI-CEC

An electronic board designed to drive a TV via the HDMI-CEC protocol.
The instructions are transmitted by radio via a NRF24L01 module.

## PCB

The card must be powered with 5v via the min-usb connector.

The card was designed with [Eagle].
The additional [libraries] used are:
* ATMEGA328P-AU.lbr
* CONNECTORS.lbr
* NRF24L01.lbr
* regulator.lbr

## Software

The firmware turns on the TV when it receives the instruction "workday_start" via the radio and turn it off when it receive the instruction "workday_end".
After turning on the TV it switches to the HDMI2 input automatically. This value is hardcoded.

The HDMI-CEC management is handled by a custom library. You will find the information in the dedicated readme.

The firmware is compiled and uploaded with the Arduino IDE.

The additional libraries used are:
* NRFLite: Handle the radio management. You will found it in the official Arduino repo.
* CEC: Handle the CEC protocol. You will found it in the "libraries" directory.

[Eagle]:     https://www.autodesk.fr/products/eagle
[libraries]: https://github.com/hiteule/eagle-library
