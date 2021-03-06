//Include libraries
#include <PID_v1.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DualMC33926MotorShield.h"
// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 2
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
DualMC33926MotorShield md;
int i = 50;
int currentTime = 0; 
int preTime = 0;   
int timeDiff = 0; // timediff check if over 1sec yet
//********************
float prevTemp = 0;
double Setpoint, Input, Output;
PID myPID(&Input, &Output, &Setpoint,70,0,0, DIRECT);
void setTemp()
{
  int done = 0;
  int counter = 0;
  char stringB [5]; 
  char ch;
    Serial.println("please choose temperature, must be number");   
        while(done == 0)
        {
          ch = Serial.read();
          if((isdigit(ch) == 1) || (ch == '.') )
          {
          stringB[counter] = ch;
          counter++;
          Serial.print("entered temperature is ");
          Serial.println(ch);
          }
          else if (isAlpha(ch) == 1)
          {
            if (ch == 'x')
            {
              Serial.println("done input");
              done = 1;
            }
          }
        }
        stringB[counter] = '\0';
        int temp = atoi(stringB);
        Setpoint = (double)temp;
        Serial.print("Input Temp is: ");
        Serial.print(Setpoint,4);
        Serial.println("C");
}
void setup(void)
{
  Serial.begin(9600); //Begin serial communication
  Serial.println("Arduino Digital Temperature // Serial Monitor Version"); //Print a message
  Serial.println("Dual MC33926 Motor Shield");
  md.init();
  sensors.begin();
  Setpoint = 65;
  myPID.SetOutputLimits(0, 400);
  myPID.SetMode(AUTOMATIC);
  pinMode(30, OUTPUT);
  setTemp();
  //You tell us that Output is always zero, but you don't tell us anything about Setpoint of Input.  
  //If input is uniformly greater than Setpoint, Output will always be zero.  It looks like you're reading the setpoiont from an analog input.
}

void loop(void)
{ 
  // Send the command to get temperatures
  sensors.requestTemperatures();  
  char ch ;
  
    ch =Serial.read();
    if(ch == 'a')
    {
      i = i + 20;
      Serial.print("speed is ");
      Serial.println(i);
    }
     else   if(ch == 'b')
    {
      i = i - 20;
      Serial.print("speed is ");
      Serial.println(i);
    }
    else if (ch == 'f')
    {
      digitalWrite(30,HIGH);
    }
        else if (ch == 's')
    {
      digitalWrite(30,LOW);
    }
    
  //Update value every 1 sec.
  int convSpeed = (int)Output;
  prevTemp = sensors.getTempCByIndex(0);
  Input = (double)prevTemp;
  myPID.Compute();
  if (convSpeed > 400)
  {
    Serial.println("cant above 400");
    convSpeed = 400;
  }
  else if (convSpeed < 0)
  {
    Serial.println("cant below 0");
    convSpeed = 0;
  }
  else
  {
  }
   md.setM1Speed(convSpeed);
  //************
      preTime  = currentTime;
    currentTime = millis();
    int actualTemp = prevTemp + 2;
    timeDiff = timeDiff + (currentTime-preTime);
    if (timeDiff > 1000)
      {
         Serial.print("Temperature is: ");
         Serial.println(prevTemp); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
         timeDiff = 0;
         Serial.print("Speed is: ");
         Serial.println(convSpeed); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
      }
  
}
