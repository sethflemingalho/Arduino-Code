#include <IRremote.h>

int receiver = 2;

IRrecv irrecv(receiver);
decode_results results;

void setup()
{
  Serial.begin(9600);
  Serial.println("IR Receive Raw Data + Button Decode Test");
  irrecv.enableIRIn();
}

void loop()
{
  if (irrecv.decode(&results));
  {
    Serial.print(results.value, HEX);
    translateIR();
    irrecv.resume();
  }
}

void translateIR()
{
  switch(results.value)
  {
    
    
    default:
    Serial.println(" = Not Programmed yet:)");
  }
  
  delay(250);
  
}
