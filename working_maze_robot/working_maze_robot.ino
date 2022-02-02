const int pingPin = 9;
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *myMotor = AFMS.getMotor(1);
Adafruit_DCMotor *myMotor2 = AFMS.getMotor(4);

Servo myservo;  // create servo object to control a servo
// a maximum of eight servo objects can be created

int pos = 0;    // variable to store the servo position
long right;
long left;
long forward;
long forward2;
long cm;
int buttonState = 0;         // variable for reading the pushbutton status
int buttonState2 = 0;
const int buttonPin = 7;     // the number of the pushbutton pin
const int buttonPin2 = 6;
void setup() {
  Serial.begin(9600);
  AFMS.begin();
  myMotor->setSpeed(90);
  myMotor->run(FORWARD);
  // turn on motor
  myMotor->run(RELEASE);
  myMotor2->setSpeed(100);
  myMotor2->run(FORWARD);
  // turn on motor
  myMotor2->run(RELEASE);
  myservo.attach(2);  // attaches the servo on pin 9 to the servo object
  pinMode(buttonPin, INPUT);
  pinMode(buttonPin2, INPUT);
}
void loop()
{


  cm = Read();


  if (cm > 15) {
    myMotor->run(FORWARD);
    myMotor->setSpeed(100);
    myMotor2->run(FORWARD);
    myMotor2->setSpeed(100);

    // read the state of the pushbutton value:
    buttonState = digitalRead(buttonPin);
    buttonState2 = digitalRead(buttonPin2);
    // check if the pushbutton is pressed.
    // if it is, the buttonState is HIGH:
    if (buttonState == HIGH) {
      myMotor->run(BACKWARD);
      myMotor->setSpeed(100);
      delay(300);
      myMotor2->run(BACKWARD);
      myMotor2->setSpeed(100);
      delay(300);
      myMotor->run(FORWARD);
      myMotor->setSpeed(100);
      delay(300);
      myMotor2->run(BACKWARD);
      myMotor2->setSpeed(100);
      delay(300);
    }
    if (buttonState2 == HIGH) {
      myMotor2->run(BACKWARD);
      myMotor2->setSpeed(100);
      delay(300);
      myMotor->run(BACKWARD);
      myMotor->setSpeed(100);
      delay(300);
      myMotor2->run(FORWARD);
      myMotor2->setSpeed(100);
      delay(300);
      myMotor->run(BACKWARD);
      myMotor->setSpeed(100);
      delay(300);
    }

    //myMotor->setSpeed(100);
   // myMotor2->setSpeed(100);

  }

  if (cm < 15) {
    myMotor->setSpeed(0);
    myMotor2->setSpeed(0);


    myservo.write(180);
    delay(350);


    cm = Read();

    if (cm > 25) {
      myMotor->run(FORWARD);
      myMotor->setSpeed(90);
      delay(100);
      myMotor2->run(BACKWARD);
      myMotor2->setSpeed(90);
      delay(100);
    }
    if (cm < 25) {
      myMotor->run(BACKWARD);
      myMotor->setSpeed(90);
      delay(100);
      myMotor2->run(FORWARD);
      myMotor2->setSpeed(90);
      delay(100);

    }




    myservo.write(90);              // tell servo to go to position in variable 'pos'
    delay(450);

  }
}













long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}

long Read() {
  pinMode(pingPin, OUTPUT);
  long dis;
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(pingPin, INPUT);
  dis = pulseIn(pingPin, HIGH);
  dis = microsecondsToCentimeters(dis);
  Serial.print(dis);
  Serial.print("cm");
  Serial.println();
  return dis;



}


