//libraries used
//import java.lang.reflect.Array;
import processing.serial.*;

Serial SerialPort;    //The serial port

//Variables
int graphMargin = 40;
int sizeX = 800;
int sizeY = 640;
int graphSpaceX = sizeX - 2 * graphMargin;
int graphSpaceY = sizeY - 2 * graphMargin;
int axisDivisions = 29;
int axisDivisionLenght = 5;
int[] lastxPos = {graphMargin, graphMargin, graphMargin, graphMargin, graphMargin, graphMargin};
int[] lastheight = {600, 600, 600, 600, 600, 600};
int[] xPos = {graphMargin, graphMargin, graphMargin, graphMargin, graphMargin, graphMargin};
String inString;
float[] temperatureList = new float[6];
int[] mappedTemperatures = new int[6];

//Colors. Every hotendhas a color to differentiate it
color hotend1 = color(4,79,111);
color hotend2 = color(84,145,158);
color hotend3 = color(20,255,10);
color hotend4 = color(108,22,237);
color hotend5 = color(255,0,0);
color hotend6 = color(0,0,255);
color[] colors = {hotend1, hotend2, hotend3, hotend4, hotend5, hotend6};
//Config this to connect to the board. COM ports and baudrate
int baudrate = 115200;
String COMPORT = "COM51";
int MAXTEMPERATURE = 300;
int TARGETTEMPERATURE = 250;

void setup() {
  //set the window size
  size(800,640);
 
  //Draw the main screen with the graph
  drawGraph();
  
  //print the available serial ports
  printArray(Serial.list());
  // Check the listed serial ports in your machine
  // and use the correct index number in Serial.list()[].
  SerialPort = new Serial(this, COMPORT, baudrate);  //
  
  // A serialEvent() is generated when a newline character is received :
  SerialPort.bufferUntil('\n');
}

void draw() {
   //Drawing a line from Last temperature to the new one.  
   strokeWeight(1);        //stroke wider
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

void drawGraph() {
  //set the rectangle for the graph
  rect(graphMargin,graphMargin,sizeX - 2*graphMargin,sizeY - 2*graphMargin);
  String s = "The Target Temperature is " + TARGETTEMPERATURE + "ºC";
  fill(50);
  text(s, 320, 15, 200, 100);  // Text wraps within text box
  line(40,134,760,134);  //Line at 250ºC
  
  //Draw the Y axis marks for better display
  int j;
  for (j=graphMargin; j<(graphSpaceY+graphMargin); j=j+int((graphSpaceY/axisDivisions)))
  {
    line(graphMargin-axisDivisionLenght, j, graphMargin + axisDivisionLenght, j);
  }
  
  //Draw the X axis marks for better display
  for (j=graphMargin; j<(graphSpaceX+graphMargin); j=j+int((graphSpaceX/axisDivisions)))
  {
    line(j,sizeY - graphMargin - axisDivisionLenght, j, sizeY - graphMargin + axisDivisionLenght);
  }
  
}

void interpolate() {
  
  
  
}
void serialEvent (Serial SerialPort) {
  // get the ASCII string:
  inString = SerialPort.readStringUntil('\n');
  if (inString != null) {
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
        println(temperatureList[i]);
        mappedTemperatures[i] = int(map(temperatureList[i],0,MAXTEMPERATURE,600,40));
        //println(mappedTemperatures[i]);
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
      addToArray(xPos,2);
    }
  }
}