#include <MCP3202.h>
#include <FlexiTimer2.h>
#define DATA_DIVIDER_BYTE1 120


int Vdd = 5; //change according to your Vdd value.

MCP3202 adc = MCP3202(10); //Parameter passed is the CS pin number. Change according to requirement.

long int index = 0; 


void flash() 
{

  float x = (float)index;
  float ch1 = (adc.readChannel(0)*Vdd)/4096.0 + 1.0f;
  float ch2 = (adc.readChannel(1)*Vdd)/4096.0;

  float vals[3] = { x, ch1, ch2 };
  Serial.write(120);
  Serial.write((byte*)&vals[0], 12);
  

  index++;
}

void setup() {
  // put your setup code here, to run once:
  adc.begin();
  Serial.begin(1000000);  
  
  FlexiTimer2::set(1, 1.0/2000.0, flash); // 2000x per second T = 0.5ms
  FlexiTimer2::start();
}

void loop() {

}
