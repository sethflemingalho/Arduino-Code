#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include <SPI.h>

// A simple data logger for the Arduino analog pins

// how many milliseconds between grabbing data and logging it. 1000 ms is once a second
#define LOG_INTERVAL  1000 // mills between entries (reduce to take more/faster data)

// how many milliseconds before writing the logged data permanently to disk
// set it to the LOG_INTERVAL to write each time (safest)
// set it to 10*LOG_INTERVAL to write all data every 10 datareads, you could lose up to 
// the last 10 reads if power is lost but it uses less power and is much faster!
#define SYNC_INTERVAL 10000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()

#define ECHO_TO_SERIAL   0 // Echo data to serial monitor (Setting it to 0 will turn it off)
#define WAIT_TO_START    0 // Wait for serial input in setup() (Setting it to 1 you have to send a character to zhe Arduino´s Serial port to start the logging)

// the digital pins that connect to the LEDs (Solder pads L1 (green) and L2 (red) on the datalogger shield)
#define redLEDpin 2
#define greenLEDpin 3

// The analog pin connected to the PV cell voltage divider
#define sensorPin A0


RTC_DS1307 RTC; // define the Real Time Clock object

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// the logging file
File logfile;

void error(char *str)
{
 Serial.print("error: ");
 Serial.println(str);
  
// red LED indicates error
 digitalWrite(redLEDpin, HIGH);
 while(1);
}

//88888888888888888888888888888888888888888888888888888888888888888888888
#include <SparkFunBME280.h>

#include <SparkFunCCS811.h>



#define CCS811_ADDR 0x5B //Default I2C Address

//#define CCS811_ADDR 0x5A //Alternate I2C Address



#define PIN_NOT_WAKE 5



//Global sensor objects

CCS811 myCCS811(CCS811_ADDR);

BME280 myBME280;


//8888888888888888888888888888888888888888888888888888888888888888888888


void setup(void)
{

  
  Serial.begin(9600);
  Serial.println();
  //000000000000000000000000000000000000000000000000000000000000000000
  Serial.println("Apply BME280 data to CCS811 for compensation.");



  //This begins the CCS811 sensor and prints error status of .begin()

  CCS811Core::status returnCode = myCCS811.begin();

  Serial.print("CCS811 begin exited with: ");

  //Pass the error code to a function to print the results

  //printDriverError( returnCode );

  Serial.println();



  //For I2C, enable the following and disable the SPI section

  myBME280.settings.commInterface = I2C_MODE;

  myBME280.settings.I2CAddress = 0x77;



  //Initialize BME280

  //For I2C, enable the following and disable the SPI section

  myBME280.settings.commInterface = I2C_MODE;

  myBME280.settings.I2CAddress = 0x77;

  myBME280.settings.runMode = 3; //Normal mode

  myBME280.settings.tStandby = 0;

  myBME280.settings.filter = 4;

  myBME280.settings.tempOverSample = 5;

  myBME280.settings.pressOverSample = 5;

  myBME280.settings.humidOverSample = 5;



  //Calling .begin() causes the settings to be loaded

  delay(10);  //Make sure sensor had enough time to turn on. BME280 requires 2ms to start up.

  myBME280.begin();

  //000000000000000000000000000000000000000000000000000000000000000000
  // use debugging LEDs
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);
  
#if WAIT_TO_START
  Serial.println("Type any character to start");
  while (!Serial.available());
#endif //WAIT_TO_START

  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    error("Card failed, or not present");
  }
  Serial.println("card initialized.");
  
  // create a new file
  char filename[] = "LOGGER00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
  if (! logfile) {
    error("couldnt create file");
  }
  
  Serial.print("Logging to: ");
  Serial.println(filename);

  // connect to RTC
  Wire.begin();  
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL
  }
  

  logfile.println("millis; stamp; datetime; voltage");    
#if ECHO_TO_SERIAL
  Serial.print("millis");
  Serial.print("\t");
  Serial.print("stamp");
  Serial.print("\t");
  Serial.print("\t");
  Serial.print("datetime");
  Serial.print("\t");
  Serial.print("\t");
  Serial.println("voltage");
#endif //ECHO_TO_SERIAL
  }

void loop(void)
{
{



 //0000000000000000000000000000000000000000000000000000000000000000000000000

  //Check to see if data is available

  if (myCCS811.dataAvailable())

  {

    //Calling this function updates the global tVOC and eCO2 variables

    myCCS811.readAlgorithmResults();

    //printInfoSerial fetches the values of tVOC and eCO2

//    printInfoSerial();



    float BMEtempC = myBME280.readTempC();

    float BMEhumid = myBME280.readFloatHumidity();

  //This sends the temperature data to the CCS811

    myCCS811.setEnvironmentalData(BMEhumid, BMEtempC);

  }

  else if (myCCS811.checkForStatusError())

  {

    //If the CCS811 found an internal error, print it.

   // printSensorError();

  }



  delay(2000); //Wait for next reading

}


 //000000000000000000000000000000000000000000000000000000000000000000000000
  DateTime now;

  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
  
  digitalWrite(greenLEDpin, HIGH);
  
  // log milliseconds since starting
  uint32_t m = millis();
  logfile.print(m);           // milliseconds since start
  logfile.print("; ");    
#if ECHO_TO_SERIAL
  Serial.print(m);         // milliseconds since start
  Serial.print("\t");  
#endif

  // fetch the time
  now = RTC.now();
  // log time
  /*
  logfile.print(now.unixtime()); // seconds since 1/1/1970
  logfile.print("; ");
  // logfile.print();
  logfile.print(now.day(), DEC);  
  logfile.print(".");
  logfile.print(now.month(), DEC);
  logfile.print(".");
  logfile.print(now.year(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
  // logfile.print('"');
#if ECHO_TO_SERIAL
  Serial.print(now.unixtime()); // seconds since 1/1/1970
  Serial.print("\t");
  Serial.print(now.day(), DEC);
  Serial.print(".");
  Serial.print(now.month(), DEC);
  Serial.print(".");
  Serial.print(now.year(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.print("\t");
#endif //ECHO_TO_SERIAL
      
    */
  
  logfile.print("; ");    
  logfile.println(myCCS811.getCO2());

  #if ECHO_TO_SERIAL
  Serial.println(myCCS811.getCO2());
#endif //ECHO_TO_SERIAL










  logfile.print("; ");
  logfile.print(myCCS811.getTVOC());

  #if ECHO_TO_SERIAL
  Serial.println(myCCS811.getTVOC());
#endif //ECHO_TO_SERIAL







   logfile.print("; ");
   logfile.print(myBME280.readTempC(), 2);
   #if ECHO_TO_SERIAL
  Serial.println(myBME280.readTempC(), 2);
#endif //ECHO_TO_SERIAL



   logfile.print("; ");
   logfile.print(myBME280.readFloatPressure(), 2);
   #if ECHO_TO_SERIAL
  Serial.println(myBME280.readFloatPressure(), 2);
#endif //ECHO_TO_SERIAL





  



   logfile.print("; ");
   logfile.print(myBME280.readFloatAltitudeMeters(), 2);
   #if ECHO_TO_SERIAL
  Serial.println(myBME280.readFloatAltitudeMeters(), 2);
#endif //ECHO_TO_SERIAL




// At the end of data logging the green LED is turned off
digitalWrite(greenLEDpin, LOW);

  // Now we write data to disk! Don't sync too often - requires 2048 bytes of I/O to SD card
  // which uses a bunch of power and takes time
  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();
  
  // blink LED to show we are syncing data to the card & updating FAT!
 digitalWrite(redLEDpin, HIGH);
  logfile.flush();
 digitalWrite(redLEDpin, LOW);
 }


 void printInfoSerial()

{

  //getCO2() gets the previously read data from the library

  Serial.println("CCS811 data:");

  Serial.print(" CO2 concentration : ");

  Serial.print(myCCS811.getCO2());

  Serial.println(" ppm");



  //getTVOC() gets the previously read data from the library

  Serial.print(" TVOC concentration : ");

  Serial.print(myCCS811.getTVOC());

  Serial.println(" ppb");



  Serial.println("BME280 data:");

  Serial.print(" Temperature: ");

  Serial.print(myBME280.readTempC(), 2);

  Serial.println(" degrees C");



  Serial.print(" Temperature: ");

  Serial.print(myBME280.readTempF(), 2);

  Serial.println(" degrees F");



  Serial.print(" Pressure: ");

  Serial.print(myBME280.readFloatPressure(), 2);

  Serial.println(" Pa");



  Serial.print(" Pressure: ");

  Serial.print((myBME280.readFloatPressure() * 0.0002953), 2);

  Serial.println(" InHg");



  Serial.print(" Altitude: ");

  Serial.print(myBME280.readFloatAltitudeMeters(), 2);

  Serial.println("m");



  Serial.print(" Altitude: ");

  Serial.print(myBME280.readFloatAltitudeFeet(), 2);

  Serial.println("ft");



  Serial.print(" %RH: ");

  Serial.print(myBME280.readFloatHumidity(), 2);

  Serial.println(" %");



  Serial.println();





}


