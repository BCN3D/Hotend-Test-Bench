# Hotend test bench
Electronic Board to test out 6 Hotends at a time. It basically heats up the 6 hotends to a specified temperature and then cools down.
It verifies that the time to do the cycle is correct. 

You can find all the files in eagle, the BOM and the Gerbers.
The files are from the version 1.0 and we're waiting for the boards to arrive.

The code uses the [PID library](http://playground.arduino.cc/Code/PIDLibrary) for Arduino and a home made [hotend class](https://github.com/BCN3D/Hotend-Test-Jig/blob/master/Code/hotend.h).


