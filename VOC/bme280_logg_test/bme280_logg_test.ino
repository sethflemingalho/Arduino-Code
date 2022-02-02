
// Compiled from Adafruit examples for BME280, TSL2561, SD Data Logger and RTC
// Includes LDR for comparison purposes only


#include <Adafruit_TSL2561_U.h>
#include <RTClib.h>
#include <SD.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

const int chipSelect = 10;


#define photocellPin  0
#define redLEDpin 2
#define greenLEDpin 3

/*
   Connections:
   ===========
   Connect BME_SCK to A5
   Connect BME_SDI to A4
   Connect BME_VIN to 5V

   Connect SCL to analog 5
   Connect SDA to analog 4
   Connect VDD to 3.3V DC

   I2C Address
   ===========
   The address will be different depending on whether you leave
   the ADDR pin floating (addr 0x39), or tie it to ground or vcc.
   The default addess is 0x39, which assumes the ADDR pin is floating
   (not connected to anything).  If you set the ADDR pin high
   or low, use TSL2561_ADDR_HIGH (0x49) or TSL2561_ADDR_LOW
   (0x29) respectively.
 */

Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);


// Displays basic information on TSL2591 sensor

void displaySensorDetails(void)
{
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" lux");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" lux");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" lux");

  delay(500);
}

// Configures the gain and integration time for the TSL2561

void configureSensor(void)
{
  // You can manually set the gain or enable auto-gain support - uncomment desired option
  
  // tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */

  // Changing the integration time gives you better sensor resolution (402ms = 16-bit data)

  //tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      // fast but low resolution 
  //tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  // medium resolution and speed   
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  // 16-bit data but slowest conversions 

  // Update these displayed values to reflect what was set above! 

  Serial.print  ("Gain:         "); Serial.println("Auto");
  Serial.print  ("Timing:       "); Serial.println("402 ms");
  Serial.print ("-----------------------------------");

}

/**************************************************************************/

#define LOG_INTERVAL  1000 // ---------------------------------SENSOR READ / LOG DELAY 1 of 2
// how many milliseconds before writing the logged data permanently to disk
// set it to the LOG_INTERVAL to write each time (safest)
// set it to 10*LOG_INTERVAL to write all data every 10 datareads, you could lose up to
// the last 10 reads if power is lost but it uses less power and is much faster!

#define SYNC_INTERVAL 1000 //------------------------------SENSOR READ / LOG DELAY 2 of 2
// mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()


#define WAIT_TO_START    0 // Wait for serial input in setup()


RTC_DS1307 RTC; // define the Real Time Clock object

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);

  // red LED indicates error
  digitalWrite(redLEDpin, HIGH);

  while (1);
}


Adafruit_BME280 bme; // I2C mode

File SensorData; // Data object to write sensor data to

float tempC;
float tempF;
float RH;
float pressure;
int photocellReading; // the analog reading from the analog resistor divider


void setup() //=======================================================================
{

  Serial.begin(9600);
  pinMode(10, OUTPUT);
  Serial.println(F("BME280-TSL2561 Data Logger"));

  pinMode(redLEDpin, OUTPUT); // debugging LED
  pinMode(greenLEDpin, OUTPUT); // debugging LED

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // Initialise TSL2561 sensor
  if (!tsl.begin())
  {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    while (1);
  }

  /* Display some basic information on this sensor */
  displaySensorDetails();

  /* Setup the sensor gain and integration time */
  configureSensor();

  /* We're ready to go! */
  Serial.println("");

  Serial.print("Initializing SD card...");

  pinMode(SS, OUTPUT);
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    digitalWrite(redLEDpin, HIGH);
    // don't do anything more:
    while (1) ;
  }
  Serial.println("  Card Initialized.");
  Serial.println("");


  // create a new file
  char filename[] = "LOGGER00.CSV";  // Basic Filename
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i / 10 + '0';
    filename[7] = i % 10 + '0';
    if (! SD.exists(filename)) {

      // only open a new file if it doesn't exist
      SensorData = SD.open(filename, FILE_WRITE);
      break;  // leave the loop!
    }
  }

  if (! SensorData) {
    error("couldnt create file");
  }

  Serial.print("Logging data to: ");
  Serial.println(filename);
  Serial.println("-----------------------------------");
  digitalWrite(greenLEDpin, HIGH);  // Green light on


  // connect to RTC
  Wire.begin();
  if (!RTC.begin()) {
    SensorData.println("RTC failed");
    Serial.println("RTC failed");
    digitalWrite(redLEDpin, HIGH);
  }

  Serial.println("");
}

void loop() //============================================================================
{

  DateTime now;
  photocellReading = analogRead(photocellPin);
  tempC = (bme.readTemperature()) ;
  tempF = (tempC * 9.0 / 5.0 + 32.0) ;
  RH =    (bme.readHumidity()) ;
  pressure = (bme.readPressure() / 3389.39) ;


  // delay between readings
  delay((LOG_INTERVAL - 1) - (millis() % LOG_INTERVAL));

  digitalWrite(greenLEDpin, HIGH);


  // fetch the time
  now = RTC.now(); //log time


  SensorData.print(now.year(), DEC);
  SensorData.print("-");
  SensorData.print(now.month(), DEC);
  SensorData.print("-");
  SensorData.print(now.day(), DEC);
  SensorData.print(", ");

  SensorData.print(now.hour(), DEC);
  SensorData.print(":");
  SensorData.print(now.minute(), DEC);
  SensorData.print(":");
  SensorData.print(now.second(), DEC);
  SensorData.print(',');

  SensorData.print(tempF);
  SensorData.print(",");

  SensorData.print(RH);
  SensorData.print(",");

  SensorData.print(pressure);
  SensorData.print(",");

  SensorData.print(photocellReading);
  SensorData.print(",");
  
  //-------------------------------------------------------------------
  // new TSL2561 sensor reading 
  sensors_event_t event;
  tsl.getEvent(&event);

  // Display the results (light is measured in lux) 
  if (event.light)
  {
    SensorData.print(event.light);
    SensorData.println(",");
  }
  else
  {
    /* If event.light = 0 lux the sensor is probably saturated
       and no reliable data could be generated! */
    Serial.println("Sensor overload");
  }
  //-------------------------------------------------------------------

  SensorData.flush();
  digitalWrite(greenLEDpin, LOW);

  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print("  ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
  Serial.println();

  Serial.print(tempF);
  Serial.println(" Degrees F");

  Serial.print(RH);
  Serial.println(" % Relative Humidity");

  Serial.print(pressure);
  Serial.println(" InHg");

  Serial.print(event.light);
  Serial.println(" Lux");

  Serial.print(photocellReading);
  Serial.println(" Photocell Units");
  Serial.println();


}



