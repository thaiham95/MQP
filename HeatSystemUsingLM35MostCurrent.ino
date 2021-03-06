// Because arduino temperature sensor is offset with real temperature sensor 3-4C. Therefore, if want to target 37C, must input into system '33.5' to '34,5'. Any number in this range should 
// within the assay temperature tolerance. 
#include <PID_v1.h>
#include "DualMC33926MotorShield.h"
DualMC33926MotorShield md;
//****** TIMING PARAMETER********************
int i = 50;
int currentTime = 0; 
int preTime = 0;   
int timeDiff = 0; // timediff check if over 1sec yet
unsigned long time1;
//****** TRIGGER PARAMETER ********************
int trigger = 0;
int inTrigger = 0;
int val;
int tempPin = 3;
float prevTemp = 0;
int roomTemp = 23.6;
int offSet = 3;
//****** PID CONSTANT ****************************
int convSpeed;
double Setpoint, Input, Output;
double Kp = 70;
double Ki = 0;
double Kd = 0;
int pidCounter = 0; // Only for printing notice
int pidCounter2 = 0; // only for printing notice
PID myPID(&Input, &Output, &Setpoint,Kp,Ki,Kd, DIRECT);
//***** LM35 RAW DATA PROCESSING *********************
float temperature()
{
val = analogRead(tempPin);
float mv = ( val/1024.0)*5000; 
float cel = mv/10;
float farh = (cel*9)/5 + 32;
return cel;
Serial.print("TEMPRATURE = ");
Serial.print(cel);
Serial.print("*C");
Serial.println();
}

void setTemp() // can type in any number like '36.5' then press enter. After input temperature, must input character 'x' to continue the code or it will stuck there forever.
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
        double temp = atof(stringB);
        Setpoint = (double)temp;

        prevTemp = temperature();
        if (prevTemp < Setpoint)
        {
          inTrigger = 1;
        }
        else
        {
          inTrigger = 0;
        }
        Serial.print("Input Temp is: ");
        Serial.print(Setpoint,4);
        Serial.println("C");
}

void temperatureChange() // In the middle of temperature processing, for example currently at 37C. If want to reduce to room temperature, input to serial 'q', it will prompt users to ask a different temperature. 
{
    // Send the command to get temperatures
  char ch ;
  
    ch =Serial.read();
   
   if ( (ch == 'f') || (trigger == 1))
    {
      digitalWrite(30,HIGH);
    }
        else if (ch == 's' || (trigger == 0))
    {
      digitalWrite(30,LOW);
    }
   
    if (ch == 'q')
    {
      setTemp();
      pidCounter2 = 0;
      pidCounter = 0;
    }
}

void powerDirection()
{
      if (prevTemp < Setpoint && inTrigger == 1) // If want to increase temp to Setpoint, PID direction is "DIRECT"
    {
    trigger = 0;
      if (pidCounter == 0)
  {
    Serial.println("direct");
    pidCounter++;
    myPID.SetControllerDirection(DIRECT);
  }
    }
  else if ( (prevTemp > Setpoint) && ((prevTemp > roomTemp) && (inTrigger == 0))) // If want to decrease temp to room temperature, PID direction is "REVERSE"
  {
    if (pidCounter == 0)
  {
    Serial.println("reverse");
    pidCounter++;
     myPID.SetControllerDirection(REVERSE);
  }
  convSpeed = 0; // turn off Peltier module
  trigger = 1;  // turn on cooling fan
  }

  else  if ( (prevTemp > Setpoint) && ((prevTemp <= roomTemp) && (inTrigger == 0))) // If want to decrease temp below room temperature, turn on Peltier but reverse voltage for cooling
  {
    trigger = 1;
    convSpeed = 0-abs(convSpeed);
    if (pidCounter2 == 0)
    {
    Serial.println("Start cooling");
     myPID.SetControllerDirection(REVERSE);
      pidCounter2++;
    }
  }
}
void setup(void)
{
  Serial.begin(9600); //Begin serial communication
  Serial.println("Arduino Digital Temperature // Serial Monitor Version");
  Serial.println("Dual MC33926 Motor Shield");
  md.init();
  myPID.SetOutputLimits(0, 400);
  myPID.SetMode(AUTOMATIC);
  pinMode(30, OUTPUT); // pin 30 turn on or off cooling fan
  setTemp();
}

void loop(void)
{ 
  temperatureChange();
  //Update value every 0.5 sec.
  myPID.Compute();
  convSpeed = (int)Output; // output of PID is motor speed - current draw appropriate for Peltier
  prevTemp = temperature(); // Getting data from LM35 sensor.
  Input = (double)prevTemp; // Input of PID is the temperature returned from lm35 sensor.
  //************* POWER DIRECTION CHECKING *********************
    powerDirection();
  //********* OPERATING PELTIER AND DATA PRINTING **************************
   md.setM1Speed(convSpeed); // command to tell motor controller to adjust energy deliver to Peltier
      preTime  = currentTime;
     float realTemp = prevTemp + offSet;
    currentTime = millis();
    time1 = millis();
    timeDiff = timeDiff + (currentTime-preTime);
    if (timeDiff > 500) // Every 500mS, print information to Serial and reset timer.
      {
         Serial.print(prevTemp,3); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
         timeDiff = 0;
         Serial.print(" ");
         Serial.print(convSpeed); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
          Serial.print(" ");
         Serial.println(md.getM1CurrentMilliamps());
      }
  
}

