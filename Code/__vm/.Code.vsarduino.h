/* 
	Editor: http://www.visualmicro.com
	        visual micro and the arduino ide ignore this code during compilation. this code is automatically maintained by visualmicro, manual changes to this file will be overwritten
	        the contents of the Visual Micro sketch sub folder can be deleted prior to publishing a project
	        all non-arduino files created by visual micro and all visual studio project or solution files can be freely deleted and are not required to compile a sketch (do not delete your own code!).
	        note: debugger breakpoints are stored in '.sln' or '.asln' files, knowledge of last uploaded breakpoints is stored in the upload.vmps.xml file. Both files are required to continue a previous debug session without needing to compile and upload again
	
	Hardware: Arduino/Genuino Uno, Platform=avr, Package=arduino
*/

#define __AVR_ATmega328p__
#define __AVR_ATmega328P__
#define _VMDEBUG 1
#define ARDUINO 106011
#define ARDUINO_MAIN
#define F_CPU 16000000L
#define __AVR__
#define F_CPU 16000000L
#define ARDUINO 106011
#define ARDUINO_AVR_UNO
#define ARDUINO_ARCH_AVR
 #include <VM_DBG.h>

//
//
void setPwmFrequency(int pin, int divisor);
void checkConnectedHotends();
void printTemperatures();
void updateSR();
void startLEDs();
void manageLEDs(bool status, uint8_t pos);
void blinkLeds(int i);
void securityBlink(int i);
void setPIDtunings(double Kp, double Ki, double Kd);
void commands();
void parseCommand(String com);
void printSettings();

#include "pins_arduino.h" 
#include "arduino.h"
#include "Code.ino"
