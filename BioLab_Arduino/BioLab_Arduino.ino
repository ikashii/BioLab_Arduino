#include <FlexiTimer2.h>
#include <SPI.h>
#include <Wire.h>


//     MCP3204 -- Arduino
//Chipselect 8 -- D10
//       DIN 9 -- D11
//     DOUT 10 -- D12
//      CLK 11 -- D13

// Set Constants
const int CS = 10;      // set pin 10 as the chip select for the ADC:
char adresse1 = 40;    //0101A2A1A0 = 0101000 = 40
char adresse2 = 44;    //0101A2A1A0 = 0101100 = 44


typedef struct
{
    uint32_t sampleIndex;            // Number of sample
    uint16_t ch0, ch1, ch2;            // Sample values
    
} DataPackage;

int8_t dataDividerByte = 120;    // Separator
uint32_t index = 0;

int8_t start_sending_Byte = 001;
int8_t stop_sending_Byte = 000;
int8_t mag_Byte = 121;
bool senddata = false;


byte values[3] = {100, 100, 100};


void setWiper(char adresse, int wiper, int value){
  
  Wire.beginTransmission(adresse);              //Gerät wird ausgewählt mit der Adresse
  if(wiper == 0 && value <= 255 && value >= 0){ //Es wird zwischen Wiper 0 und 1 gewählt
      Wire.write(0xA9);                         //Wiper0 = 0xA9 (Datenblatt)
      Wire.write(value);                        //Der wert wird in den Wiper geschrieben
  }if(wiper == 1 && value <= 255 && value >= 0){
      Wire.write(0xAA);                         //Wiper1 = 0xAA (Datenblatt)
      Wire.write(value);
  }
  int fehler = Wire.endTransmission();                       //Das übertragen wird Beendet
  Serial.println(fehler);
}

void flash() 
{
  index++;
  if(!senddata){
    return;
  }
  DataPackage package;
  package.sampleIndex = index;
  package.ch0 = readAdc(0);
  package.ch1 = readAdc(1);
  package.ch2 = readAdc(2);

  Serial.write(dataDividerByte);
  Serial.write((byte*)&package, 10);    //10 Bit, 80 Bytes
}

// Start setup function:
void setup() {  
  
  Serial.begin(1000000);
  pinMode (CS, OUTPUT); 
  
  // set the ChipSelectPins high initially: 
  digitalWrite(CS, HIGH); 
   
  // initialise SPI:
  SPI.beginTransaction(SPISettings(1400000, MSBFIRST, SPI_MODE3));
  SPI.begin();

  FlexiTimer2::set(1, 1.0/2000.0, flash); // 2000x per second T = 0.5ms
  FlexiTimer2::start();

  Wire.begin();
  //Wire.setClock(10000);
  //setWiper(adresse1, 1, 100);
  setWiper(adresse1, 1, values[0]);
  setWiper(adresse1, 0, values[1]);                   
  setWiper(adresse2, 1, values[2]);
  setWiper(adresse2, 0, values[2]);
} // End setup function.

// Start loop function:
void loop() {  

  if(Serial.available() >= 1){
    char inchar = Serial.read();
    if(inchar == start_sending_Byte){
      senddata = true;
     
    }
    if(inchar == stop_sending_Byte){
      senddata = false;
          
    }
    if(inchar == mag_Byte){
        Serial.readBytes(values, 3);      
        setWiper(adresse1, 1, values[0]);
        setWiper(adresse1, 0, values[1]);                   
        setWiper(adresse2, 1, values[2]);
        setWiper(adresse2, 0, values[2]);
        
       }
    
  }
//  delay(500);
//  setWiper(adresse1, 1, 255);
//  setWiper(adresse1, 0, 255);                   
//  setWiper(adresse2, 0, 255);
//  setWiper(adresse2, 1, 255);


  
}// End of loop function.

//Function to read the ADC, accepts the channel to be read.
int16_t readAdc(int channel) {
  byte adcPrimaryRegister = 0b00000110;      // Sets default Primary ADC Address register B00000110, This is a default address setting, the third LSB is set to one to start the ADC, the second LSB is to set the ADC to single ended mode, the LSB is for D2 address bit, for this ADC its a "Don't Care" bit.
  byte adcPrimaryRegisterMask = 0b00000111;  // b00000111 Isolates the three LSB.  
  byte adcPrimaryByteMask = 0b00001111;      // b00001111 isolates the 4 LSB for the value returned. 
  byte adcPrimaryConfig = adcPrimaryRegister & adcPrimaryRegisterMask; // ensures the adc register is limited to the mask and assembles the configuration byte to send to ADC.
  byte adcSecondaryConfig = channel << 6;
  noInterrupts(); // disable interupts to prepare to send address data to the ADC.  
  digitalWrite(CS,LOW); // take the Chip Select pin low to select the ADC.
  SPI.transfer(adcPrimaryConfig); //  send in the primary configuration address byte to the ADC.  
  byte adcPrimaryByte = SPI.transfer(adcSecondaryConfig); // read the primary byte, also sending in the secondary address byte.  
  byte adcSecondaryByte = SPI.transfer(0x00); // read the secondary byte, also sending 0 as this doesn't matter. 
  digitalWrite(CS,HIGH); // take the Chip Select pin high to de-select the ADC.
  interrupts(); // Enable interupts.
  adcPrimaryByte &= adcPrimaryByteMask; // Limits the value of the primary byte to the 4 LSB:
  int16_t digitalValue = (adcPrimaryByte << 8) | adcSecondaryByte; // Shifts the 4 LSB of the primary byte to become the 4 MSB of the 12 bit digital value, this is then ORed to the secondary byte value that holds the 8 LSB of the digital value.
  //float value = (float(digitalValue) * 5.000) / 4096.000; // The digital value is converted to an analogue voltage using a VREF of 2.048V.
  return digitalValue; // Returns the value from the function
}
