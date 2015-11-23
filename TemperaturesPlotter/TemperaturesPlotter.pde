//BCN3D Technologies - Fundacio CIM 
//Marc Cobler Cosmen - November 2015
//Temperatures Plotter. Part of the Hotend Test Jig
//Processing program that reads from a serial port the temperatures of 6 hotends at a time
//and plots them in order to verify the manufacturing and the resistance and thermistor value

//Any suggestions contact me directly at mcobler@fundaciocim.org
//This program is exported to windows and can run by itself without processing being installed.

//libraries used
import processing.serial.*;


//****************************************************************************************
//Variables
//Config this to connect to the board. COM ports and baudrate
int baudrate = 115200;
String selectedCOMPORT;
String[] serialPorts = new String[20];
//this values are fixed by firmware. There are needed to show the graph correctly
int MAXTEMPERATURE = 300;
int TARGETTEMPERATURE = 250;
//****************************************************************************************
Serial SerialPort;    //The serial port

int graphMargin = 40;
int sizeX = 800;
int sizeY = 640;
int graphSpaceX = sizeX - 2 * graphMargin;
int graphSpaceY = sizeY - 2 * graphMargin;
int incrementXPosition = 2;
int axisDivisions = 29;
int axisDivisionLenght = 3;
int temperatureIncrementAxis = MAXTEMPERATURE / (axisDivisions + 1); 
int comPortBoxSizeX = 250;
int comPortBoxSizeY = 50;
int comPortTextSize = 18;
int comPortOffsetY = 50;

String author = "Marc Cobler";
String title = "Temperature Plotter";
String subtitle = "Hotend Test Jig";

int[] lastxPos = {graphMargin, graphMargin, graphMargin, graphMargin, graphMargin, graphMargin};
int[] lastheight = {600, 600, 600, 600, 600, 600};
int[] xPos = {graphMargin, graphMargin, graphMargin, graphMargin, graphMargin, graphMargin};
String inString;
float[] temperatureList = new float[6];
int[] mappedTemperatures = new int[6];
int startTime, finishTime, heatingTime;

//Colors. Every hotendhas a color to differentiate it
color hotend1 = color(4,79,111);
color hotend2 = color(84,145,158);
color hotend3 = color(20,255,10);
color hotend4 = color(108,22,237);
color hotend5 = color(255,0,0);
color hotend6 = color(0,0,255);
color[] colors = {hotend1, hotend2, hotend3, hotend4, hotend5, hotend6};

//Variables used for the diferents screens
int screen = 0;
int splashScreenTime = 4000;
//Image Values
int wheelOriginX = 212;
int wheelOriginY = 114;
int wheelSize = 291;
int wheelOffset;
PImage logo;
PImage wheel;

void setup() {
  //set the window size
  size(800,700);
  background(255);
  //Load and process the logo to make it turn
  logo = loadImage("logo.jpg");
  wheel = logo.get(wheelOriginX, wheelOriginY, wheelSize, wheelSize);
  //It should be 75px but don't know yet why
  wheelOffset = logo.height/2 - (wheelOriginY + wheelSize/2);
  
  //print the available serial ports
  printArray(Serial.list());  
  //Save the Serial Ports available
  serialPorts = Serial.list();
  
}

void draw() {
  
  switch (screen) {
    
    case 0:
    splashScreen();
    break;
    
    case 1:    //Serial Port menu
    selectCOMPORT();    
    break;
    
    case 2:    //Main graph screen
       //Drawing a line from Last temperature to the new one.  
       strokeWeight(1.10);        //stroke wider
       int i;
       for (i=0; i<6; i++) 
       {
         if (temperatureList[i] <= 0)  //Hotend not connected. Negative Temperature
          {
          } else {
           stroke(colors[i]);
           line(lastxPos[i], lastheight[i], xPos[i], mappedTemperatures[i]); 
          }
       }
       printTime();
       printTemperaturesLegend();
  }
}


//User Functions
void setArray(int[] a , int v) {
    int i, n = a.length;
    for (i = 0; i < n; ++i) {
        a[i] = v;
    }
}

void addToArray(int[] a, int v) {
    int i, n = a.length;
    for (i = 0; i < n; i++) {
       a[i] += v; 
    }
}

void splashScreen() {
  background(255);
  
  rectMode(CENTER);
  textAlign(CENTER,CENTER);
  textSize(26);
  fill(0);
  text(title, sizeX/2, sizeY/6, 400, 100);
  text(subtitle, sizeX/2, sizeY/1.20, 400, 100);
  textSize(15);
  text("by " + author, sizeX/2, sizeY/1.1, 400, 100);
  
  imageMode(CENTER);
  if (millis() < splashScreenTime) {
    image(logo, sizeX/2, sizeY/2, logo.width/2, logo.height/2);
    translate(width/2, height/2- 75);
    rotate(100*TWO_PI/millis());
    image(wheel, 0, 0,wheel.width/2, wheel.height/2);
  } else {
    screen = 1;  
  }
}

//This function prints a simple menu and lets you select the COM Port
void selectCOMPORT() {
  // Check the listed serial ports in your machine
  // and use the correct index number in Serial.list()[].
  background(255);
  rectMode(CENTER);
  textAlign(CENTER,CENTER);
  textSize(26);
  fill(0);
  text("Select the communication Port:", sizeX/2, sizeY/5, 400, 100);
  
  textSize(comPortTextSize);
  int i;
  for (i = 0; i< serialPorts.length; i++) {
    text(serialPorts[i], sizeX/2, sizeY/5 + comPortOffsetY + i * comPortOffsetY, comPortBoxSizeX, comPortBoxSizeY);
  }
  
}

void mousePressed() {
  int option;
  if (screen == 1) {  //Select COM Port Screen
    if ( mouseX >= (sizeX/2 - comPortBoxSizeX/2) && mouseX <= (sizeX/2 + comPortBoxSizeX/2) ) {
      option = floor((mouseY - sizeY/5 + comPortOffsetY/2) / comPortBoxSizeY);
      println(option-1);
      if (option <= serialPorts.length && option >= 0) {
        selectedCOMPORT = serialPorts[option-1];
        SerialPort = new Serial(this, selectedCOMPORT, baudrate);
        // A serialEvent() is generated when a newline character is received :
        SerialPort.bufferUntil('\n');
  
        //Next Screen - Graph
        screen = 2;
        //Draw the graph only one time! if not, the lines will become points
        drawGraph();
      }
    }
  }
}

void drawGraph() {
  int i;
  
  String s = "The Target Temperature is " + TARGETTEMPERATURE + "ºC";
  fill(0);
  text(s, 320, 15, 200, 20);  // Text wraps within text box
  
  //set the rectangle for the graph
  rectMode(CORNER);
  textAlign(LEFT,BOTTOM);
  background(255);
  fill(255);
  stroke(0);
  rect(graphMargin,graphMargin,graphSpaceX, graphSpaceY);

  //!use the funtion interpolate to set the height
  line(40,134,760,134);  //Line at TARGET TEMPERATURE
  
  //Draw the legend of colors at the bottom of the screen
  noStroke();
  textSize(12);
  for (i=0; i<6; i++) {
    fill(colors[i]);
    if(i<2) {
      rect(40,620+(i*30),20,20,20); 
      fill(0);
      text("hotend" + (i+1),70,635+(i*30));
    } else if (i<4) {
      rect(200,620+(i-2)*30,20,20,20);
      fill(0);
      text("hotend" + (i+1),230,635+((i-2)*30));
    } else if (i<6) {
      rect(360,620+(i-4)*30,20,20,20);
      fill(0);
      text("hotend" + (i+1),390,635+((i-4)*30));
    }
  }
  
  //back to stroke in black
  stroke(0);
  //Draw the Y axis marks for better display
  int j;
  int displayTemperature = 0;
  for (j=graphMargin; j<(graphSpaceY+graphMargin); j=j+int((graphSpaceY/axisDivisions)))
  {
    line(graphMargin-axisDivisionLenght, j, graphMargin + axisDivisionLenght, j);
    text(MAXTEMPERATURE-displayTemperature,graphMargin-axisDivisionLenght-25,j);
    displayTemperature += temperatureIncrementAxis;
  }

  //Draw the X axis marks for better display
  for (j=graphMargin; j<(graphSpaceX+graphMargin); j=j+int((graphSpaceX/axisDivisions)))
  {
    line(j,sizeY - graphMargin - axisDivisionLenght, j, sizeY - graphMargin + axisDivisionLenght);
  }
  
}

//Prints the current time with a resolution of milliseconds
void printTime() {
   noStroke();
   //Draw a white rectangle to delete the text
   fill(255);
   rect(525,625,100,20);
   fill(0);
   textSize(12);
   text("Time: " + millis()/1000.0 + " s",525,620,100,20); 
}

//Prints the Legend of the 6 temperatures with the current temperature
void printTemperaturesLegend() {
  int i;
  int offset = 55;
  String displayString;
  noStroke();
  textSize(12);
    for (i=0; i<6; i++) {
      if (temperatureList[i] < 0.0) {
        //Hotend not connected, or really really cold day
        displayString = "NC";
      } else {
        displayString = temperatureList[i] + " ºC"; 
      }
      fill(255);
      if(i<2) {
        rect(70+offset,620+(i*30),70,20); 
        fill(0);
        text(displayString,70+offset,635+(i*30));
      } else if (i<4) {
        rect(230+offset,620+(i-2)*30,70,20);
        fill(0);
        text(displayString,230+offset,635+((i-2)*30));
      } else if (i<6) {
        rect(390+offset,620+(i-4)*30,70,20);
        fill(0);
        text(displayString,390+offset,635+((i-4)*30));
      }
    }
}

void interpolate() {
}

//This is where the magic happens. Gets the values from the COM Port and map them to the Graph
void serialEvent (Serial SerialPort) {
  // get the ASCII string:
  inString = SerialPort.readStringUntil('\n');
  if (inString != null) {
    if (inString != "start") {
      //Separate strings by comas
      String[] temperatureStrings = split(inString, ',');
      
      //Convert the strings to floats
      int i;
      for (i=0; i<temperatureStrings.length; i++)
      {
        temperatureList[i] = float(temperatureStrings[i]);
      }
      
      //map the values readed to the height of the screen
      for (i=0; i<temperatureList.length; i++)
      {
        if (temperatureList[i] <= 0)  //Hotend not connected. Negative Temperature
        {
          //Do nothing
        } else {                      //Map the temperature to the graph
          //Print the temperatures to the console. Used for debugging
          //print("Hotend " + (i+1) + ": ");
          //println(temperatureList[i]);
          mappedTemperatures[i] = int(map(temperatureList[i],0,MAXTEMPERATURE,graphSpaceY+graphMargin+10,graphMargin));
          //println(mappedTemperatures[i]);
          
          //Check if target temperature and calculate time
          if (temperatureList[i] >= TARGETTEMPERATURE) {
             //Temperature Reached 
             finishTime = millis();
             heatingTime = finishTime - startTime;
             println(heatingTime);
          }
        }
      }
  
      //Drawing a line from Last temperature to the new one.
      for (i=0; i<temperatureList.length; i++)
      {
      lastxPos[i] = xPos[i];
      lastheight[i] = int(mappedTemperatures[i]);
      }
  
      // at the edge of the window, go back to the beginning:
      if (xPos[0] >= sizeX - graphMargin) {
        setArray(xPos, graphMargin);
        setArray(lastxPos, graphMargin);
      } 
      else {
        // increment the horizontal position:
        addToArray(xPos,incrementXPosition);
      }
    } else {
      startTime = millis(); 
    }
  }
}