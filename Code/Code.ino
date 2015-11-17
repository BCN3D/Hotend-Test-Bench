/*---------------------------------------------------------------------
Hotend_TestJig.ino - main program file
HotEnd test Jig
Marc Cobler Cosmen - June 2015
BCN3D Technologies - Fundacio CIM

This program is for testing the assembled hotends in BCN3D Technologies.
It's Atmega328 based and it contains 6 power Mosfet Channels to heat up
6 hotends at a time. If the heating curve is within limits, then a Green 
LED lights up, if not a red LED lights up.


ADJUSTING PWM FREQUENCIES 
In the Arduino world timer0 is been used for the timer functions, like delay(), millis() and micros(). 
If you change timer0 registers, this may influence the Arduino timer function. 
So you should know what you are doing. 
// Note that the base frequency for pins 3, 9, 10, and 11 is 31250 Hz
// Note that the base frequency for pins 5 and 6 is 62500 Hz
-----------------------------------------------------------------------*/
#include "hotend.h"
#include "PID_v1.h"
//--------------------CONSTANTS-----------------------------------------
#define MAXTEMPERATURE 250
#define SECURITYTEMPERATURE 50
#define UPDATEINTERVAL 500 //compute everything every half second

const char* tempSensors[]= {"THERM0","THERM1","THERM2","THERM3","THERM4","THERM5"};
#define THERM0 A5
#define THERM1 A4
#define THERM2 A3
#define THERM3 A2
#define THERM4 A1
#define THERM5 A0

#define HOTEND0 3 //PD3 TIMER2
#define HOTEND1 5 //PD5 TIMER0
#define HOTEND2 6 //PD6 TIMER0
#define HOTEND3 9 //PB1 TIMER1
#define HOTEND4 10 //PB2 TIMER1
#define HOTEND5 11 //PB3 TIMER2

#define CLOCK 7
#define LATCH 8
#define DATA 4

//--------------------VARIABLES-----------------------------------------
bool connectedHotends[6]= {0,0,0,0,0,0}; //Array of boolean. Indicates hotends connected
String command;
//PID Variables
double output;
double setTemperature;
//PID Settings
double kp = 1;
double kd = 0.25;
double ki = 0.05;
//bytes for the shift Registers
byte leds1;
byte leds2;

unsigned long previousMillis = 0;	//Last time Hotend updated

//--------------------OBJECTS-------------------------------------------
//---Let's declare de hotends objects
hotend h1(THERM0, HOTEND0);
hotend h2(THERM1, HOTEND1);
hotend h3(THERM2, HOTEND2);
hotend h4(THERM3, HOTEND3);
hotend h5(THERM4, HOTEND4);
hotend h6(THERM5, HOTEND5);

PID hotendPID1(&h1.tempCelsius, &output, &setTemperature, kp, ki, kd, DIRECT);
PID hotendPID2(&h2.tempCelsius, &output, &setTemperature, kp, ki, kd, DIRECT);
PID hotendPID3(&h3.tempCelsius, &output, &setTemperature, kp, ki, kd, DIRECT);
PID hotendPID4(&h4.tempCelsius, &output, &setTemperature, kp, ki, kd, DIRECT);
PID hotendPID5(&h5.tempCelsius, &output, &setTemperature, kp, ki, kd, DIRECT);
PID hotendPID6(&h6.tempCelsius, &output, &setTemperature, kp, ki, kd, DIRECT);

hotend hotends[] = {h1,h2,h3,h4,h5,h6};
PID PIDS[] = {hotendPID1, hotendPID2, hotendPID3, hotendPID4, hotendPID5, hotendPID6};

void setup()
{
  pinMode(CLOCK, OUTPUT);
  pinMode(LATCH, OUTPUT);
  pinMode(DATA, OUTPUT);
  digitalWrite(CLOCK, LOW);
  //No need to declare analog pins as inputs
  //Set the PWM frequency to 3906.25Hz on Timer 1 and 2
  setPwmFrequency(HOTEND0, 8); //Pins 3 and 11 are paired on timer 2 so only need to change one
  setPwmFrequency(HOTEND3, 8); //Pins 9 and 10 are paired on timer 1 so only need to change one
  uint8_t j;
  for (j=0; j<6; j++)
  {
	  hotends[j].readTemperature();
  }
  
  setTemperature = MAXTEMPERATURE;
  
  hotendPID1.SetMode(AUTOMATIC);
  hotendPID2.SetMode(AUTOMATIC);
  hotendPID3.SetMode(AUTOMATIC);
  hotendPID4.SetMode(AUTOMATIC);
  hotendPID5.SetMode(AUTOMATIC);
  hotendPID6.SetMode(AUTOMATIC);
  
  Serial.begin(115200);
  delay(500);
  Serial.println("Starting the Hotend Test Jig...");
  delay(500);
  startLEDs();

  Serial.println("Checking connected hotends");
  checkConnectedHotends();
  Serial.println("start");
}
void loop()
{
	uint8_t i;
	commands();
	unsigned long currentMillis = millis();
	if (currentMillis - previousMillis >= UPDATEINTERVAL)
	{
		previousMillis = currentMillis;
		for (i=0 ; i<6; i++)
		{
			if (connectedHotends[i] == 1)	
			{
				hotends[i].readTemperature();
				//Serial.print("Hotend ");
				//Serial.print(i+1);
				//Serial.print(":	");
				//Serial.println(hotends[i].tempCelsius);		
				switch (i) {
					case 0:
						hotendPID1.Compute();
					break;
					case 1:
						hotendPID2.Compute();
					break;
					case 2:
						hotendPID3.Compute();
					break;
					case 3:
						hotendPID4.Compute();
					break;
					case 4:
						hotendPID5.Compute();
					break;
					case 5:
						hotendPID6.Compute();
					break;
					default:
						//Do nothing
					break;
				}
				hotends[i].update(output);
			} //End if connected hotends
				if (hotends[i].state == 1)	//Heating done. Temperature Reached
				{
					manageLEDs(hotends[i].hotendState, i);
					if (hotends[i].tempCelsius <= SECURITYTEMPERATURE && connectedHotends[i] == 1)
					{
						securityBlink(i);
					}
				} else {					//Temperature not reached. Blinking...
					if (connectedHotends[i] == 1)
					{	
						blinkLeds(i);
					}
			}
		} //for end
		printTemperatures();
	} //update interval end
} //loop end
//--------------------USER FUNCTIONS----------------------
void setPwmFrequency(int pin, int divisor) {
	byte mode;
	if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
		switch(divisor) {
			case 1: mode = 0x01; break;
			case 8: mode = 0x02; break;
			case 64: mode = 0x03; break;
			case 256: mode = 0x04; break;
			case 1024: mode = 0x05; break;
			default: return;
		}
		if(pin == 5 || pin == 6) {
			TCCR0B = TCCR0B & 0b11111000 | mode;
			} else {
			TCCR1B = TCCR1B & 0b11111000 | mode;
		}
		} else if(pin == 3 || pin == 11) {
		switch(divisor) {
			case 1: mode = 0x01; break;
			case 8: mode = 0x02; break;
			case 32: mode = 0x03; break;
			case 64: mode = 0x04; break;
			case 128: mode = 0x05; break;
			case 256: mode = 0x06; break;
			case 1024: mode = 0x7; break;
			default: return;
		}
		TCCR2B = TCCR2B & 0b11111000 | mode;
	}
}
void checkConnectedHotends() {
	uint8_t i;
	for (i = 0; i<= 5; i++)
	{
		hotends[i].readTemperature();
		if (hotends[i].averageTemp >= 200000)		//Remember that averageTemp is Resistance in Ohms 
		{
			//Hotend i not connected
			connectedHotends[i] = 0;
			Serial.print("Hotend ");
			Serial.print(i + 1, DEC);
			Serial.println(" not present");
		} else { 
			//Hotend i present and with a valid reading
			connectedHotends[i] = 1;
			Serial.print("Hotend ");
			Serial.print(i + 1, DEC);
			Serial.println(" connected!");
		}
	}
}
void printTemperatures()
{
	int j;
	for (j=0; j<6; j++)
	{
		Serial.print(hotends[j].tempCelsius);
		if (j < 5)
		{
			Serial.print(",");
		} 
	}
	Serial.print("\n");
}
void updateSR() {	//Update LED shift registers
	digitalWrite(LATCH, LOW);
	shiftOut(DATA, CLOCK, LSBFIRST, leds2);
	shiftOut(DATA, CLOCK, LSBFIRST, leds1);
	digitalWrite(LATCH, HIGH);
}
void startLEDs() {
	Serial.println("Turning on all the LEDS!");
	leds1 = 0x00;
	leds2 = 0x00;
	uint8_t i;
	for (i=0; i<8; i++ )
	{
		bitSet(leds1, i);
		delay(150);
		updateSR();
	}
	for (i=4; i<8; i++)
	{
		bitSet(leds2, i);
		delay(150);
		updateSR();
	}
	for (i=0; i<6; i++)
	{
		leds1 = 0xFF;
		leds2 = 0xFF;
		updateSR();
		delay(100);
		leds1 = 0x00;
		leds2 = 0x00;
		updateSR();
		delay(100); 
	}
}
void manageLEDs(bool status, uint8_t pos) {
	//leds1 = 0x00;
	//leds2 = 0x00;
	//Manage the leds throught the Shift Registers
	if (status)		//Correct timing
	{
		//Green LED must be turned on
		if (pos < 4)
		{
			bitSet(leds1, 2*pos + 1);
		} else {
			bitSet(leds2, 2*pos - 3);
		}
	} else {
		//Red LED must be turned on
		if (pos < 4)
		{
			bitSet(leds1, 2*pos );
		} else {
			bitSet(leds2, 2*pos - 4);
		}
	}
	updateSR();	
}
void blinkLeds(int i) {
	//This functions blinks the hotend LEDs to know that it is heating up
		//Serial.println("on Leds");
		
		if (i < 4){
			bitSet(leds1,i*2);
			updateSR();
			delay(50);
			bitSet(leds1,i*2 + 1);
			updateSR();
		} else {
			bitSet(leds2,i*2-4);
			updateSR();
			delay(50);
			bitSet(leds2,i*2 + 1 - 4);
			updateSR();
		}
		//Serial.println("off Leds");
		if (i < 4){
			bitClear(leds1,i*2);
			updateSR();
			delay(50);
			bitClear(leds1,i*2 + 1);
			updateSR();
		} else {
			bitClear(leds2,i*2 -4);
			updateSR();
			delay(50);
			bitClear(leds2,i*2 + 1 - 4);
			updateSR();
		}
}
void securityBlink(int i) {
	//This functions blinks the hotend LEDs to know that it is cold enough to hold it
	if (i < 4){
		bitSet(leds1,i*2);
		updateSR();
		} else {
		bitSet(leds2,i*2-4);
		updateSR();
	}
	delay(100);
	if (i < 4){
		bitClear(leds1,i*2);
		updateSR();
		} else {
		bitClear(leds2,i*2 -4);
		updateSR();
	}
}
void setPIDtunings(double Kp, double Ki, double Kd) {
	kp = Kp;
	ki = Ki;
	kd = Kd;
}
void commands()	{
	while (Serial.available() > 0)
	{
		char c = Serial.read();
		if (c == '\n')
		{
			parseCommand(command);
			command = "";
		}
		else
		{
			command += c;
		}
	}
}
void parseCommand(String com)	{
	//the commands will ignore the Case, so it could be:
	double kpTemp = 0;
	double kiTemp = 0;
	double kdTemp = 0;
	
	String part1, part2;
	bool twoArgCommand;
	int s = 0;
	while (com[s] != ' ' && s <= (com.length()-1)){
		//If found a blank space and s is less than com length-->out
		s++;
	}
	if (s < com.length())
	{
		Serial.println("Found a space in the command");
		twoArgCommand = true;
		part1 = com.substring(0, com.indexOf(" "));
		part2 = com.substring(com.indexOf(" ") + 1, com.length());
	} else if(s == com.length())
	{
		Serial.println("No space in the command");
		twoArgCommand = false;
		part1 = com.substring(0, com.length()-1);
		part2 = "";
	}
	
	Serial.print("the command is: ");
	Serial.print(part1);
	Serial.print(" ");
	Serial.println(part2);
	
	if (part1.equalsIgnoreCase("kp"))
	{
		kpTemp = part2.toFloat();
	} else if (part1.equalsIgnoreCase("kd"))
	{
		kdTemp = part2.toFloat();
	} else if (part1.equalsIgnoreCase("ki"))
	{
		kiTemp = part2.toFloat();
	} else if (part1.equalsIgnoreCase("showinfo"))
	{
		//Prints out the PID settings like de kp ki kd and Mode
		Serial.print("Kp: ");
		Serial.println(hotendPID1.GetKp());
		Serial.print("Ki: ");
		Serial.println(hotendPID1.GetKi());
		Serial.print("Kd: ");
		Serial.println(hotendPID1.GetKd());
		Serial.print("Mode: ");
		Serial.println(hotendPID1.GetMode());
		Serial.println("Direction: ");
		Serial.println(hotendPID1.GetDirection());
	}
	
	
	if (kpTemp != 0 && kdTemp != 0 && kiTemp != 0)
	{
		setPIDtunings(kpTemp, kiTemp, kdTemp);
		kpTemp = 0;
		kiTemp = 0;
		kdTemp = 0;
	}
		
	
}