#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61);

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2);


// declaration of leds************************************************************************

//solid green light to indicate what state the bridge is in its closed state location: 2 miles
int ledGRailFar = 4;


//solid red light to say bridge is open and train should be stopped. Location 1/2 mile
int solidRedTrain = 5;


//yellow led to allow boat to proceed
int yellowLed = 6;
int boatLedRed = 2;

//declaration of ultrasonc pins***********************************************************

//ultrasonics unified trig pin
int railTrigUnified = 7;
int railEchoRight = 8;
int railEchoLeft = 9;

int boatTrigUnified = 10;
int boatEchoRight = 11;
int boatEchoLeft = 12;

int cmRight;
int cmLeft;

//read the right ultrasonic
int readRightTrain();

//read the left ulrasonic
int readLeftTrain();

// read right boat
int readBoatRight();

//read left boat
int readBoatLeft();

//detect if bridge is open or closed
const int pressurePlate = 3;
int buttonState = 0;

//********************************************


// Variables will change :
int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;  // will store last time LED was updated

// constants won't change :
const long interval = 1000;

//*********************************************************
const int blinkSlowDown = 13;
int slowDownState = LOW;


//********************************************

bool state = true;


void setup() {
  AFMS.begin();
  myMotor->setSpeed(20);
  Serial.begin(9600);
  pinMode(ledGRailFar, OUTPUT);
  pinMode(pressurePlate, INPUT);
  pinMode(yellowLed, OUTPUT);
  //  pinMode(boatGreenLed, OUTPUT);
  pinMode(solidRedTrain, OUTPUT);
  pinMode(boatLedRed, OUTPUT);
  pinMode(blinkSlowDown, OUTPUT);



}

void loop() {

  //*******************************define variables*************************
  int  cmRightTrain = 0;
  int  cmLeftTrain = 0;
  int cmRightTrainTwo = 0;
  int cmLeftTrainTwo = 0;

  int velocityRight;
  int velocityLeft;

  int cmBoatRight = 0;
  int cmBoatLeft = 0;

  int ledState = LOW;

  //unsigned long currentMillis = millis();



  //*****************************set closed state*************************
  cmBoatLeft = readBoatLeftInitial();
 // delay(50);
  cmBoatRight = readBoatRightInitial();

  if (state == true) {
    digitalWrite(ledGRailFar, HIGH); // this pin will also control the red light for the boats
    digitalWrite(solidRedTrain, LOW);
    digitalWrite(yellowLed, LOW);
    digitalWrite(boatLedRed, HIGH);
    digitalWrite(blinkSlowDown, LOW);
   // state = false;
  }
  

  if (cmBoatLeft <= 90 || cmBoatRight <= 90) {

    cmRightTrain = readRightTrain();
    delay(400);
    cmRightTrainTwo = readRightTrain();

    velocityRight = (cmRightTrainTwo - cmRightTrain) / 500;
    //***********************************************************get velocity from right********************************************

    cmLeftTrain = readLeftTrain();
    delay(400);
    cmLeftTrainTwo = readLeftTrain();

    velocityLeft = (cmLeftTrainTwo - cmLeftTrain) / 500;
    //******************************************************************get velocity from left***********************************

    if (velocityRight >= 6 || velocityLeft >= 6) {

      digitalWrite(blinkSlowDown, HIGH);


    }
    if (velocityRight > 6 || velocityLeft >6){
      digitalWrite(blinkSlowDown, HIGH);
      delay(150);
      digitalWrite(blinkSlowDown, LOW);
      delay(150);
      digitalWrite(blinkSlowDown, HIGH);
      delay(150);
      digitalWrite(blinkSlowDown, LOW);
      
    }
    
    
    if (velocityRight == 0 && velocityLeft == 0) {
    
         cmLeftTrain = readLeftTrain();
         cmRightTrain = readRightTrain();
         

    if(cmLeftTrain > 20 && cmRightTrain > 20){

      buttonState = digitalRead(pressurePlate);
      Serial.println("the button state is");
      Serial.println(buttonState);

      if (buttonState == 0) { //no one is on them
        digitalWrite(solidRedTrain, HIGH);
        digitalWrite(ledGRailFar, LOW);
        digitalWrite(boatLedRed, HIGH);


        myMotor->step(1200, FORWARD, DOUBLE);
        // myMotor->step(1400, FORWARD, DOUBLE);

        delay(3000);

        digitalWrite(boatLedRed, LOW);
        digitalWrite(yellowLed, HIGH);

        do {

          delay(150);
          cmBoatLeft = readBoatLeft();
          cmBoatRight = readBoatRight();

        } while (cmBoatLeft < 30 || cmBoatRight < 30);

        if (cmBoatLeft > 10 && cmBoatRight > 10) {
          digitalWrite(solidRedTrain, HIGH);
          digitalWrite(boatLedRed, HIGH);
          digitalWrite(yellowLed, LOW);
          digitalWrite(ledGRailFar, LOW);

          // myMotor->step(1400, BACKWARD, DOUBLE);
          myMotor->step(1200, BACKWARD, DOUBLE);

        }

      
      }

    }


    }
  }
  

}




//**********************************************

int readBoatLeftInitial() {


  digitalWrite(ledGRailFar, LOW); // this pin will also control the red light for the boats
  digitalWrite(solidRedTrain, LOW);
  digitalWrite(yellowLed, LOW);
  digitalWrite(boatLedRed, LOW);
  digitalWrite(blinkSlowDown, LOW);


  long duration, cmLeftBoat;

  pinMode(boatTrigUnified, OUTPUT);
  digitalWrite(boatTrigUnified, LOW);
  delayMicroseconds(2);
  digitalWrite(boatTrigUnified, HIGH);
  delayMicroseconds(10);
  digitalWrite(boatTrigUnified, LOW);


  pinMode(boatEchoLeft, INPUT);
  duration = pulseIn(boatEchoLeft, HIGH);

  cmLeftBoat = microsecondsToCentimeters(duration);

  Serial.print(cmLeftBoat);
  Serial.println("cmleftBoat,");

  digitalWrite(ledGRailFar, HIGH); // this pin will also control the red light for the boats
  digitalWrite(solidRedTrain, LOW);
  digitalWrite(yellowLed, LOW);
  digitalWrite(boatLedRed, HIGH);
  digitalWrite(blinkSlowDown, LOW);


  delay(100);


  return cmLeftBoat;

}




//*******************************************************************************
int readBoatRightInitial() {

  digitalWrite(ledGRailFar, LOW); // this pin will also control the red light for the boats
  digitalWrite(solidRedTrain, LOW);
  digitalWrite(yellowLed, LOW);
  digitalWrite(boatLedRed, LOW);
  digitalWrite(blinkSlowDown, LOW);


  long duration, cmRightBoat;

  pinMode(boatTrigUnified, OUTPUT);
  digitalWrite(boatTrigUnified, LOW);
  delayMicroseconds(2);
  digitalWrite(boatTrigUnified, HIGH);
  delayMicroseconds(10);
  digitalWrite(boatTrigUnified, LOW);


  pinMode(boatEchoRight, INPUT);
  duration = pulseIn(boatEchoRight, HIGH);

  cmRightBoat = microsecondsToCentimeters(duration);

  Serial.print(cmRightBoat);
  Serial.println("cmRightBoat,");


  digitalWrite(ledGRailFar, HIGH); // this pin will also control the red light for the boats
  digitalWrite(solidRedTrain, LOW);
  digitalWrite(yellowLed, LOW);
  digitalWrite(boatLedRed, HIGH);
  digitalWrite(blinkSlowDown, LOW);

 delay(100);
  return cmRightBoat;



}

int readBoatRight() {

  digitalWrite(ledGRailFar, LOW); // this pin will also control the red light for the boats
  digitalWrite(solidRedTrain, LOW);
  digitalWrite(yellowLed, LOW);
  digitalWrite(boatLedRed, LOW);
  digitalWrite(blinkSlowDown, LOW);


  long duration, cmRightBoat;

  pinMode(boatTrigUnified, OUTPUT);
  digitalWrite(boatTrigUnified, LOW);
  delayMicroseconds(2);
  digitalWrite(boatTrigUnified, HIGH);
  delayMicroseconds(10);
  digitalWrite(boatTrigUnified, LOW);


  pinMode(boatEchoRight, INPUT);
  duration = pulseIn(boatEchoRight, HIGH);

  cmRightBoat = microsecondsToCentimeters(duration);

  Serial.print(cmRightBoat);
  Serial.println("cmRightBoat,");


  digitalWrite(ledGRailFar, LOW); // this pin will also control the red light for the boats
  digitalWrite(solidRedTrain, HIGH);
  digitalWrite(yellowLed, HIGH);
  digitalWrite(boatLedRed, LOW);
  digitalWrite(blinkSlowDown, LOW);

  delay(100);
  return cmRightBoat;

}

int readBoatLeft() {


  digitalWrite(ledGRailFar, LOW); // this pin will also control the red light for the boats
  digitalWrite(solidRedTrain, LOW);
  digitalWrite(yellowLed, LOW);
  digitalWrite(boatLedRed, LOW);
  digitalWrite(blinkSlowDown, LOW);


  long duration, cmLeftBoat;

  pinMode(boatTrigUnified, OUTPUT);
  digitalWrite(boatTrigUnified, LOW);
  delayMicroseconds(2);
  digitalWrite(boatTrigUnified, HIGH);
  delayMicroseconds(10);
  digitalWrite(boatTrigUnified, LOW);


  pinMode(boatEchoLeft, INPUT);
  duration = pulseIn(boatEchoLeft, HIGH);

  cmLeftBoat = microsecondsToCentimeters(duration);

  Serial.print(cmLeftBoat);
  Serial.println("cmleftBoat,");

  digitalWrite(ledGRailFar, LOW); // this pin will also control the red light for the boats
  digitalWrite(solidRedTrain, HIGH);
  digitalWrite(yellowLed, HIGH);
  digitalWrite(boatLedRed, LOW);
  digitalWrite(blinkSlowDown, LOW);


  delay(100);


  return cmLeftBoat;

}



//*************************************************************************
int readRightTrain() {


  digitalWrite(ledGRailFar, LOW); // this pin will also control the red light for the boats
  digitalWrite(solidRedTrain, LOW);
  digitalWrite(yellowLed, LOW);
  digitalWrite(boatLedRed, LOW);
  digitalWrite(blinkSlowDown, LOW);

  long duration, cmRightTrain;

  pinMode(railTrigUnified, OUTPUT);
  digitalWrite(railTrigUnified, LOW);
  delayMicroseconds(2);
  digitalWrite(railTrigUnified, HIGH);
  delayMicroseconds(10);
  digitalWrite(railTrigUnified, LOW);


  pinMode(railEchoRight, INPUT);
  duration = pulseIn(railEchoRight, HIGH);

  cmRightTrain = microsecondsToCentimeters(duration);

  Serial.print(cmRightTrain);
  Serial.println("cmRight,");

   digitalWrite(ledGRailFar, LOW); // this pin will also control the red light for the boats
  digitalWrite(solidRedTrain, HIGH);
  digitalWrite(yellowLed, LOW);
  digitalWrite(boatLedRed, HIGH);
  digitalWrite(blinkSlowDown, LOW);


  delay(100);
  return cmRightTrain;

}
//***********************************************************************



int readLeftTrain() {


  digitalWrite(ledGRailFar, LOW); // this pin will also control the red light for the boats
  digitalWrite(solidRedTrain, LOW);
  digitalWrite(yellowLed, LOW);
  digitalWrite(boatLedRed, LOW);
  digitalWrite(blinkSlowDown, LOW);

  long duration, cmLeftTrain;

  pinMode(railTrigUnified, OUTPUT);
  digitalWrite(railTrigUnified, LOW);
  delayMicroseconds(2);
  digitalWrite(railTrigUnified, HIGH);
  delayMicroseconds(10);
  digitalWrite(railTrigUnified, LOW);


  pinMode(railEchoLeft, INPUT);
  duration = pulseIn(railEchoLeft, HIGH);

  cmLeftTrain = microsecondsToCentimeters(duration);

  Serial.print(cmLeftTrain);
  Serial.println("cmLeft,");

  digitalWrite(ledGRailFar, LOW); // this pin will also control the red light for the boats
  digitalWrite(solidRedTrain, HIGH);
  digitalWrite(yellowLed, LOW);
  digitalWrite(boatLedRed, HIGH);
  digitalWrite(blinkSlowDown, LOW);




  delay(100);
  return cmLeftTrain;

}

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;

}
//********************************************************************


