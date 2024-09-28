// will play many audio file formats, mp3,aac,flac etc.
// See github page : https://github.com/schreibfaul1/ESP32-audioI2S

#include "Arduino.h"
#include "Audio.h"
#include "SD.h"
#include "FS.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>


RF24 radio(4, 5); 
const uint64_t address = 0xF0F0F0F0E1LL;

int counter = 0;

struct MyData 
{
  int counter;
  float temperature;
  float humidity;
  float altitude;
  float pressure;
};
MyData data;

//hspi pinouts
#define SD_CS 15
#define SPI_MOSI 13
#define SPI_MISO 22
#define SPI_SCK 14

//speakers pinouts
#define I2S_DOUT      25
#define I2S_BCLK      27    // I2S
#define I2S_LRC       26

Audio audio;

SPIClass *hspi = NULL;

void setup() 
{
  Serial.begin(115200);
  hspi = new SPIClass(HSPI);

  hspi->begin(SPI_SCK, SPI_MISO, SPI_MOSI, SD_CS);

  if(!SD.begin(SD_CS, *hspi))
  {
    Serial.println("Error talking to SD card!");
    while(true);  // end program
  }
    
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(15); // 0...21

  audio.connecttoFS(SD,"/hit.mp3");

  for(int i = 0; i <= 800; i++)
  {
    audio.loop();
    delay(10);
  }

  /* Radio Part: HEHE */
  radio.begin();                  //Starting the Wireless communication
  radio.openWritingPipe(address); //Setting the address where we will send the data
  radio.setPALevel(RF24_PA_MIN);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.stopListening();          //This sets the module as transmitter

  if( radio.isChipConnected())
  {
    Serial.println("CHip is connected! ");
  }
  else
  {
    Serial.println("CHip is NOT connected! ");
  }
  
  delay(2000);
}

void loop()
{
  data.counter = counter;
  data.temperature = 32;
  data.pressure = 100;
  data.altitude = 50;
  data.humidity = 5;

  Serial.print("Packet No. = ");
  Serial.println(data.counter);


  radio.write(&data, sizeof(MyData));
  
  Serial.println("Data Packet Sent");
  Serial.println("");

  counter++;
  delay(4000);

  if(counter % 3 == 0)
  {
    audio.connecttoFS(SD,"/hit.mp3");
    for(int i = 0; i <= 800; i++)
    {
      audio.loop();
      delay(10);
    }
  }
}

