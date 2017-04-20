# Hotend test bench
This project is a test bench for BCN3D Technologies.

It consist on a electronics Board to test out 6 Hotends at a time. It basically heats up the 6 hotends to a specified temperature and then cools them down.
It verifies that the time to do the cycle is correct.

![Hotend Test Jig][system]

[system]:  https://github.com/BCN3D/Hotend-Test-Jig/blob/master/img/system.JPG

#### Folders

- _BOM: _ here you can find the electronic components for the board.
- _Code:_ the files of the firmware for the microcontroller. It has been developed in Atmel Studio 7.0
- _Gerbers:_ the gerber files of the board.
- _TemperaturesPlotter:_ Processing file that plots the temperatures sent by the board. It is exported to Windows based systems in order to run the program without installing processing IDE.
- _Eagle files:_ original design files in CadSoft Eagle.
- _img:_ just some pictures of the project.

## Electronics
The electronics consist on a board with an ATmega328, 6 Power Mosfets and 2 Leds for each power channel to display the status.
The temperature is sensed by the hotend thermistors and each signal goes to a ADC channel of the microcontroller.

The 12 LEDs are driven by 2 8-bit shift registers.

It is powered by a 24V/320W switching Power supply and a +5V LDO for the logic.
For communication, it has FTDI compatible pin headers.

Board Front

![Board Front][board_front]

Board Rear

![Board Rear][board_rear]

[board_front]: https://github.com/BCN3D/Hotend-Test-Jig/blob/master/img/board_front.JPG
[board_rear]: https://github.com/BCN3D/Hotend-Test-Jig/blob/master/img/board_rear.JPG

## Code
The code uses the [PID library](http://playground.arduino.cc/Code/PIDLibrary) for Arduino and a home made [hotend class](https://github.com/BCN3D/Hotend-Test-Jig/blob/master/Code/hotend.h).

The conversion to temperature is calculated with the Steinhart-Hart equation. You can find the guidelines of this conversion in the [adafruit learning system](https://learn.adafruit.com/thermistor?view=all). It would be better to do some table look-up but the code is harder...

First it does a startup flashing the LEDs, opens a Serial port and then it checks if the hotends are connected or not by checking the thermistor resistance.

Then the heat up process starts. Every 500ms a packet is sent through the serial port with the six temperatures. This packet is received by the computer and a [Processing](http://processing.org) program plots the data.

Main Screen of the Processing program:

![TemperaturesPlotter][plotter]

[plotter]: https://github.com/BCN3D/Hotend-Test-Jig/blob/master/img/temperaturePlotter.PNG

When the  hotends are heating, the LEDs blink as well. Finally, when the target temperature is reached, the hotends start cooling down with the help of a couple of fans.

The code computes the time it took to heat up each hotend and then decides if the time is between the established limits.
## Mechanics

The Mechanics are quite simple. Basically we designed a bench to hold the hotends in place and to prevent the operator from burning. the black pieces are 3mm aluminum painted in matt black and the white pieces are 3D printed.

The cover from behind is a 3mm PMMA sheet laser cut and handcrafted bent.

## To do in the TemperaturesPlotter
- [ ] Clear button to clear the graph area.
- [ ] Start the time when the COM port is selected.
- [ ] Fix the target temperature bar to the correct temperature.
- [ ] Calculate the time to heat for each hotend
- [ ] Export data to a `.xmml` or `.csv` file and import it in Excel. To Database purposes
- [x] Splash Screen.
- [ ] Mouse gives you temperature in the graph.
