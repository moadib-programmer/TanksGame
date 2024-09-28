// will play many audio file formats, mp3,aac,flac etc.
// See github page : https://github.com/schreibfaul1/ESP32-audioI2S

#include "Arduino.h"
#include "Audio.h"
#include "SD.h"
#include "FS.h"



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
}

void loop()
{
  while(1)
  {
    delay(10);
  }

}

