/*---------------------------------------------------------------------
hotend.h - Hotend Public class
HotEnd test Jig
Marc Cobler Cosmen - June 2015
BCN3D Technologies - Fundacio CIM

This program is for testing the assembled hotends in BCN3D Technologies.
It's Atmega328 based and it contains 6 power Mosfet Channels to heat up
6 hotends at a time. If the heating curve is within limits, then a Green
LED lights up, if not a red LED lights up.

----------------------------------------------------------------------*/
#include "Arduino.h"

#define NUMSAMPLES 5
#define SERIESRESISTOR 4700
#define TEMPERATURENOMINAL 25
#define THERMISTORNOMINAL 100000
#define BCOEFFICIENT 3950
#define UPDATEINTERVAL 500
#define MAXTEMPERATURE 260
#define MEANHEATINGTIME 90000	//Mean time that takes to heat the extruder 1,5min??


class hotend
{
//Class Member Variables initialized at startup
int heaterPin;					//Heater PWM Pin
int thermistorPin;				//Thermistor pin
unsigned long previousMillis;	//Last time Hotend updated
unsigned long startTime, stopTime;
int tempSamples[NUMSAMPLES];	//Array of temp samples


//Constructor of the class - Creates a Hotend
//Initializes the member variables and state
public:
bool state;
double averageTemp;				//Average Temp calculated from samples
double tempCelsius;				//Degrees celsius value
bool hotendState;

hotend(int therm,int heater) {
	heaterPin = heater;
	thermistorPin = therm;
	pinMode(heaterPin, OUTPUT);
	
	previousMillis = 0;
	startTime = millis();
	stopTime = 0;
	averageTemp = 0;
	tempCelsius = 0;
	state = 0; //State 0 for heating and 1 for cooling
	hotendState = false; //State true for correct hotends and false for defective
	
}
void readTemperature() {
	uint8_t i;			//iterator for the temp array
	//Get the samples
	for (i = 0; i < NUMSAMPLES; i++)
	{
		tempSamples[i] = analogRead(thermistorPin);
		delayMicroseconds(500);
	}
	//Average all the samples
	for (i = 0; i < NUMSAMPLES;  i++)
	{
		averageTemp += tempSamples[i];
	}
	averageTemp = averageTemp / NUMSAMPLES;
	//Convert to Celsius
	tempCelsius = averageTemp / THERMISTORNOMINAL;		// (R/Ro)
	tempCelsius = log(tempCelsius);						// ln(R/Ro)
	tempCelsius /= BCOEFFICIENT;						//1/B * ln(R/Ro)
	tempCelsius += 1.0 / (TEMPERATURENOMINAL + 273.15);	//+ (1/To)
	tempCelsius = 1.0 / tempCelsius;					//Invert
	tempCelsius -= 273.15;								//Convert to Celsius from Kelvin
	
}
bool manageTime() {
	stopTime = millis();
	startTime += 2100;	//Add the 2 seconds delay and a margin
	if (stopTime - startTime >= MEANHEATINGTIME*0.9 && stopTime- startTime <= MEANHEATINGTIME*1.1)
	{
		//Timing is correct so GREEN LED
		return true;
	} else {
		//Timing is incorrect so RED LED 
		return false;
	}
	
}

void update(double output, uint8_t pos) {
	unsigned long currentMillis = millis();
	if (currentMillis - previousMillis >= UPDATEINTERVAL)
	{
		previousMillis = currentMillis;
		if (state = 0)	//Heating
		{
			readTemperature();
			double gap = abs(MAXTEMPERATURE - tempCelsius);
			if (gap < 5)
			{
				//setTemperature Reached. Cooling state
				state = 1;
				//Manage time
				hotendState = manageTime();
			} else {
				analogWrite(heaterPin, output);
			}
			
		} else {		//Cooling
			analogWrite(heaterPin, 0);
		}
	}

}
//protected:
//private:
};